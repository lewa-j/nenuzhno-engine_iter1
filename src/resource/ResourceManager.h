
#pragma once

#include <set>
#include <map>
#include <string>

class Model;
class IMaterial;
class Texture;
class ResourceManager;

class IModelLoader{
public:
	IModelLoader(ResourceManager *rm):resMan(rm){}
	virtual Model* Load(const char *name)=0;
	virtual bool CheckExt(const char *name)=0;
	virtual const char *GetExt()=0;
	ResourceManager *resMan;
};

class IMaterialLoader{
public:
	IMaterialLoader(ResourceManager *rm):resMan(rm){}
	virtual IMaterial* Load(const char *name)=0;
	virtual bool CheckExt(const char *name)=0;
	virtual const char *GetExt()=0;
	ResourceManager *resMan;
};
class ITextureLoader{
public:
	virtual Texture* Load(const char *name)=0;
	virtual bool CheckExt(const char *name)=0;
	virtual const char *GetExt()=0;
};

class ResourceManager{
public:
	ResourceManager();

	bool Init();

	Model *GetModel(const char *name);
	IMaterial *GetMaterial(const char *name);
	Texture *GetTexture(const char *name);

	void AddMaterial(const char *name,IMaterial *mat);

	void AddModelLoader(IModelLoader *loader);
	void AddMaterialLoader(IMaterialLoader *loader);
	void AddTextureLoader(ITextureLoader *loader);

private:
	std::map<std::string,Model*> models;
	std::map<std::string,IMaterial*> materials;
	std::map<std::string,Texture*> textures;

	std::set<IModelLoader*> modelLoaders;
	std::set<IMaterialLoader*> materialLoaders;
	std::set<ITextureLoader*> textureLoaders;
};
