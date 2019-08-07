
#include <string>
#include <fstream>
#include <cstring>
using namespace std;

#include "log.h"
#include "system/FileSystem.h"
#include "renderer/Model.h"
#include "graphics/texture.h"
#include "resource/nmf.h"

bool NenuzhnoModelLoader::CheckExt(const char *name){
	return strstr(name,".nmf")!=0;
}

//Model *LoadNenuzhnoModel(const char *fileName,int flags){
Model *NenuzhnoModelLoader::Load(const char *fileName){//,int flags
	string filePath = "models/"+string(fileName);
	IFile *file = g_fs.Open(filePath.c_str());
	if(!file){
		return 0;
	}

	nmfHeaderBase_t headerBase;
	nmfHeader2_t head2;
	file->Read(&headerBase,sizeof(nmfHeaderBase_t));
	if(headerBase.magic!=NMF_MAGIC){
		Log("Load NMF: Error invalid magic %d (%s)\n", headerBase.magic, fileName);
		g_fs.Close(file);
		return nullptr;
	}
	//TODO: check length
	//TODO: check hash

	Log("Loading model (%s) ver %d.%d\n",fileName,headerBase.version[0],headerBase.version[0]);
	if(headerBase.version[0]==1){
		nmfHeader1_t head1;
		file->Read(&head1,sizeof(nmfHeader1_t));

		//Convert old header to new
		//head2.bbox //calc later
		head2.vertexCount = head1.vertexCount;
		head2.vertexDataLength = head1.vertexCount * head1.vertexStride;
		head2.vertexOffset = head1.vertexOffset;
		head2.indexSize = head1.indexSize;
		head2.indexCount = head1.indexCount;
		head2.indexOffset = head1.indexOffset;
		head2.submeshCount = head1.submeshCount;
		head2.submeshOffset = head1.submeshOffset;
		head2.materialsCount = head1.materialsCount;
		head2.materialsOffset = head1.materialsOffset;
		//head1.vertexStride //now unused

		head2.collidersCount = 0;
		if(headerBase.version[1]>=1){
			nmfHeader11_t head11;
			file->Read(&head11,sizeof(nmfHeader11_t));
			head2.bbox = head11.bbox;
			head2.collidersCount = head11.collidersCount;
			head2.collidersOffset = head11.collidersOffset;
		}
	}else if(headerBase.version[0]==2){
		file->Read(&head2,sizeof(nmfHeader2_t));
	}else{
		Log("Load NMF: Error invalid version %d.%d (%s)\n", headerBase.version[0], headerBase.version[1], fileName);
		g_fs.Close(file);
		return nullptr;
	}

	//Log("Load NMF: %s\n", fileName);

	Model *model = new Model();

	file->Seek(head2.vertexOffset);
	if(headerBase.version[0]==2){
		nmfVertAttrib_t *attribs = new nmfVertAttrib_t[head2.vertAttribCount];
		file->Read(attribs, head2.vertAttribCount*sizeof(nmfVertAttrib_t));
		model->vertexFormat.Resize(head2.vertAttribCount);
		for(int i=0; i<head2.vertAttribCount; i++){
			model->vertexFormat[i] = attribs[i];
		}
		delete[] attribs;
	}else{
		int stride = head2.vertexDataLength/head2.vertexCount;
		model->vertexFormat.Resize(3);
		model->vertexFormat[0]={0,3,GL_FLOAT,false,stride,0};//pos
		model->vertexFormat[1]={1,3,GL_FLOAT,false,stride,12};//norm
		model->vertexFormat[2]={2,2,GL_FLOAT,false,stride,24};//uv
	}
	char *verts = new char[head2.vertexDataLength];
	file->Read(verts,head2.vertexDataLength);

	model->vbo.Create();
	model->vbo.Upload(head2.vertexDataLength,verts);
	//delete[] verts;//need for bbox
	model->vertexCount = head2.vertexCount;
	//model->vertexStride = header.vertexStride;

	if(head2.indexCount){
		model->indexSize = head2.indexSize;
		if(head2.indexSize==1){
			model->indexType = GL_UNSIGNED_BYTE;
		}else if(head2.indexSize==2){
			model->indexType = GL_UNSIGNED_SHORT;
		}else if(head2.indexSize==4){
			//TODO: check oes uint extension
			model->indexType = GL_UNSIGNED_INT;
		}else{
			Log("Load NMF: invelid index size %d (%s)\n",head2.indexSize,fileName);
		}
		int len = head2.indexCount*head2.indexSize;
		char *inds = new char[len];
		file->Seek(head2.indexOffset);
		file->Read(inds,len);

		model->ibo.Create();
		model->ibo.Upload(len,inds);
		delete[] inds;
	}

	if(head2.submeshCount){
		model->submeshes.Resize(head2.submeshCount);
		file->Seek(head2.submeshOffset);
		file->Read(model->submeshes.data,sizeof(submesh_t)*head2.submeshCount);
	}else{
		Log("Model %s haven't submeshes\n",fileName);
	}

	if(head2.materialsCount){
		int *matOffsets = new int[head2.materialsCount];
		file->Seek(head2.materialsOffset);
		file->Read(matOffsets,sizeof(int)*head2.materialsCount);

		model->materials.Resize(head2.materialsCount);
		//Log("Load NMF: materialsCount %d\n",header.materialsCount);
		for(int i=0;i<head2.materialsCount;i++){
			file->Seek(matOffsets[i]);
			int len;
			file->Read(&len,sizeof(int));
			char temp[len]={0};
			file->Read(temp,len);
			model->materials[i].name = temp;
			//Log("Load NMF: material %d offs %d len %d name %s\n",i,matOffsets[i],len,temp);
			model->materials[i].mat = resMan->GetMaterial(temp);
		}
		delete[] matOffsets;
	}

	if(headerBase.version[0]==1&&headerBase.version[1]==0){
		//vertexFormat[0] must be position
		model->bbox.FromVerts(verts,head2.vertexCount,model->vertexFormat[0]);
	}else{
		model->bbox = head2.bbox;
		if(head2.collidersCount){
			model->colliders.Resize(head2.collidersCount);
			file->Seek(head2.collidersOffset);
			file->Read(model->colliders.data,sizeof(nmfCollider_t)*head2.collidersCount);
		}
		//Log("NMF (%s) Generate bbox (min: %.2f %.2f %.2f, max: %.2f %.2f %.2f)\n",fileName,model->bbox.min.x,model->bbox.min.y,model->bbox.min.z,model->bbox.max.x,model->bbox.max.y,model->bbox.max.z);
	}
	delete[] verts;

	g_fs.Close(file);
	return model;
}
