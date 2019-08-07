

#include <cstring>
#include <string>
#include <fstream>
#include "log.h"
#include "system/FileSystem.h"
#include "graphics/texture.h"
#include "resource/ResourceManager.h"
#include "tpsMaterials.h"

using namespace std;

IMaterial *TXTMaterialLoader::Load(const char *name){
	if(strstr(name,".vtf")){
		Texture *tex = resMan->GetTexture(name);
		return new TexMaterial(tex,false);
	}else{
		string filePath = "materials/"+string(name)+".txt";
		char path[256];
		g_fs.GetFilePath(filePath.c_str(), path);
		ifstream mat_file(path);
		if(!mat_file){
			Log("Material file %s not found\n", path);
			return 0;
		}
		//shader
		mat_file >> path;
		bool lit = false;
		if(!strcmp(path,"light")){
			lit = true;
		}

		//texture
		Texture *matTex;
		mat_file >> path;

		matTex = resMan->GetTexture(path);
		if(!matTex){
			Log("Unknown material texture: %s\n", path);
			mat_file.close();
			return 0;
		}
		mat_file.close();
		Log("Load material %s: t %s, l %d\n",name,path,lit);

		return new TexMaterial(matTex,lit);
	}
	return 0;
}

