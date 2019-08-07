
#pragma once

#include "renderer/Material.h"
class Texture;
class ResourceManager;

class TXTMaterialLoader: public IMaterialLoader{
public:
	TXTMaterialLoader(ResourceManager *rm):IMaterialLoader(rm){}
	virtual IMaterial *Load(const char *name);
	virtual bool CheckExt(const char *name){return 1;/*strstr(name,".txt")!=0;*/}
	virtual const char *GetExt(){return "txt";}
};

