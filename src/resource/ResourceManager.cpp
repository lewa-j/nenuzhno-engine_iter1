
#include "log.h"
#include "resource/ResourceManager.h"
#include "renderer/Model.h"
#include "renderer/Material.h"
#include "graphics/texture.h"
#include "resource/nmf.h"
#include "resource/vtf.h"
#include "resource/dds.h"

using namespace std;

ResourceManager::ResourceManager(): models(),materials(),textures(), modelLoaders(),materialLoaders(),textureLoaders()
{}

bool ResourceManager::Init()
{
	AddModelLoader(new NenuzhnoModelLoader(this));
	AddTextureLoader(new VTFLoader());
	AddTextureLoader(new DDSLoader());

	return true;
}

Model *ResourceManager::GetModel(const char *name)
{
	map<string,Model*>::iterator fit = models.find(string(name));
	if(fit != models.end())
		return fit->second;

	for(set<IModelLoader*>::iterator mlit = modelLoaders.begin(); mlit!=modelLoaders.end(); mlit++)
	{
		IModelLoader *ml = *mlit;
		if(ml->CheckExt(name)){
			Model *mdl = ml->Load(name);
			if(mdl){
				models[string(name)] = mdl;
			}else{
				Log("ResourceManager::GetModel(%s) Not Found\n",name);
				//TODO: defaultModel?
			}
			return mdl;
		}
	}

	Log("ResourceManager::GetModel(%s) Couldn't load Model\n",name);
	return 0;
}

IMaterial *ResourceManager::GetMaterial(const char *name)
{
	map<string,IMaterial*>::iterator fit = materials.find(string(name));
	if(fit != materials.end())
		return fit->second;

	for(set<IMaterialLoader*>::iterator mlit = materialLoaders.begin(); mlit!=materialLoaders.end(); mlit++)
	{
		IMaterialLoader *ml = *mlit;
		if(ml->CheckExt(name)){
			IMaterial *mat = ml->Load(name);
			if(mat){
				materials[string(name)] = mat;
			}else{
				Log("ResourceManager::GetMaterial(%s) Not Found\n",name);
				//TODO: defaultMaterial?
			}
			return mat;
		}
	}

	Log("ResourceManager::GetMaterial(%s) Couldn't load Material\n",name);
	return 0;
}

Texture *ResourceManager::GetTexture(const char *name)
{
	map<string,Texture*>::iterator fit = textures.find(string(name));
	if(fit != textures.end())
		return fit->second;

	for(set<ITextureLoader*>::iterator tlit = textureLoaders.begin(); tlit!=textureLoaders.end(); tlit++)
	{
		ITextureLoader *tl = *tlit;
		if(tl->CheckExt(name)){
			Texture *tex = tl->Load(name);
			if(tex){
				textures[string(name)] = tex;
			}else{
				Log("ResourceManager::GetTexture(%s) Not Found\n",name);
				//TODO: defaultTexture?
			}
			return tex;
		}
	}

	Log("ResourceManager::GetTexture(%s) Couldn't load Texture\n",name);
	return 0;
}

void ResourceManager::AddMaterial(const char *name,IMaterial *mat){
	materials[string(name)] = mat;
}

void ResourceManager::AddModelLoader(IModelLoader *loader)
{
	Log("ResourceManager::AddModelLoader(*.%s)\n",loader->GetExt());
	modelLoaders.insert(loader);
}

void ResourceManager::AddMaterialLoader(IMaterialLoader *loader)
{
	Log("ResourceManager::AddMaterialLoader(*.%s)\n",loader->GetExt());
	materialLoaders.insert(loader);
}

void ResourceManager::AddTextureLoader(ITextureLoader *loader)
{
	Log("ResourceManager::AddTextureLoader(*.%s)\n",loader->GetExt());
	textureLoaders.insert(loader);
}
