
#pragma once

#include "resource/ResourceManager.h"
#include "system/neArray.h"
#include "renderer/Model.h"

struct loadMDLOut_t{
	loadMDLOut_t(){
		meshes = 0;
	}
	~loadMDLOut_t(){
		if(meshes)
			delete[] meshes;
	}
	int numMeshes;
	struct mdlOutMesh_t{
		uint64_t vertOffs;
		int vertCount;
		uint64_t indOffs;
		int indCount;
	} *meshes;
	//int numBones;
	neArray<bone_t> bones;
	neArray<Animation_t> anims;
};

class MDLLoader: public IModelLoader{
public:
	MDLLoader(ResourceManager *resMan):IModelLoader(resMan){}

	Model *Load(const char *name);
	bool CheckExt(const char *name);
	const char *GetExt(){return "mdl";}
private:
	bool LoadMDL(const char *name, loadMDLOut_t &mdlData);
	bool LoadVVD(const char *name, Model *mdl, int lod);
	bool LoadVTX(const char *name, Model *mdl, loadMDLOut_t &mdlData, int lod);
};

