
#pragma once

#include "cull/BoundingBox.h"
#include "renderer/Model.h"

using glm::vec3;

#define NMF_MAGIC ((0<<24)+('F'<<16)+('M'<<8)+'N')

//major ver - not compat
//minor ver - compat
struct nmfHeaderBase_t{
	int magic;
	uint16_t version[2];
	int length;
	int hash;
	int flags;
};

//v1.0
struct nmfHeader1_t{
	int vertexFormat; //bit mask pos,nrm,uv,uv2,tangents,weights, etc. (packet types?)
	int vertexStride;
	int vertexCount;
	int vertexOffset;

	int indexSize;
	int indexCount;
	int indexOffset;

	int submeshCount;
	int submeshOffset;

	int materialsCount;
	int materialsOffset;
};
//v 1.1
struct nmfHeader11_t{
	int collidersCount;
	int collidersOffset;

	BoundingBox bbox;
};

//v2.0
struct nmfHeader2_t{
	BoundingBox bbox;

	int vertAttribCount;
	int vertexCount;
	int vertexDataLength;
	int vertexOffset;

	int indexSize;
	int indexCount;
	int indexOffset;

	int submeshCount;
	int submeshOffset;

	int materialsCount;
	int materialsOffset;

	int collidersCount;
	int collidersOffset;
};

//TODO:
/*Bones
Sequences (+external)
Attachments (to bones)
*/

struct nmfVertAttrib_t{
	uint32_t offset;
	uint16_t type;
	uint8_t id;
	uint8_t stride;

	operator vertAttrib_t() const{
		int size = (type&3)+1;
		int otype = 0;
		if((type&32)==0){
			if((type&16)==0)
				otype = GL_BYTE;
			else
				otype = GL_SHORT;
		}else{
			if((type&16)==0)
				otype = GL_FLOAT;
			else
				otype = GL_HALF_FLOAT;
		}
		return vertAttrib_t(id,size,otype,type&8,stride,offset);
	}
};

//nmf_loader.cpp

#include "resource/ResourceManager.h"

class NenuzhnoModelLoader: public IModelLoader{
public:
	NenuzhnoModelLoader(ResourceManager *rm):IModelLoader(rm){}
	virtual Model *Load(const char *name);
	virtual bool CheckExt(const char *name);
	virtual const char *GetExt(){return "nmf";}
};

