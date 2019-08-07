
#include <string>
#include <fstream>
#include <cstring>
using namespace std;
#include <stdint.h>

#include "log.h"
#include "system/FileSystem.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/texture.h"
#include "resource/dds.h"

bool DDSLoader::CheckExt(const char *name){
	return strstr(name,".dds")!=0;
}

//bool Texture::LoadDDS(const char *fileName, int mip){
Texture *DDSLoader::Load(const char *fileName){
	string filePath = string(fileName);
	if(filePath.find(".dds") == string::npos)
		filePath += ".dds";

	//Log("Load DDS Texture %s\n",filePath.c_str());

	IFile *input = g_fs.Open(filePath.c_str());
	if(!input){
		//LOG( "File: %s not found\n", path);
		return 0;
	}

	int ident=0;
	input->Read(&ident, 4);
	if(ident!=DDS_IDENT){
		Log("LoadDDS Invalid dds file (%s)\n", fileName);
		g_fs.Close(input);
		return 0;
	}

	DDS_HEADER header;
	input->Read(&header, sizeof(DDS_HEADER));

	//LOG("LoadDDS %s: size %dx%d, mips %d, format %d\n",fileName,header.dwWidth,header.dwHeight,header.dwMipMapCount,header.ddspf.dwFourCC);

	Texture *tex = new Texture();

	tex->width = header.dwWidth;
	tex->height = header.dwHeight;
	/*if(tex->width < 4)
		tex->width = 4;
	if(tex->height < 4)
		tex->height = 4;*/

	size_t texSize = 0;

	int ifmt;
	//int fmt;
	//int type = GL_UNSIGNED_BYTE;
	bool compressed = false;
	if(header.ddspf.dwFourCC == DDS_DXT1){//4bpp
		texSize = ((tex->width + 3) / 4) * ((tex->height + 3) / 4) * 8;
		ifmt = GL_COMPRESSED_RGB_S3TC_DXT1;
		//fmt = GL_RGB;
		compressed = true;
	}else if(header.ddspf.dwFourCC == DDS_DXT5){//8bpp
		texSize = ((tex->width + 3)/4)*((tex->height+3)/4)*16;
		ifmt = GL_COMPRESSED_RGBA_S3TC_DXT5;
		compressed = true;
	}else{
		if(header.ddspf.dwFourCC==0){
			if(header.ddspf.dwRGBBitCount==24){
				texSize = tex->width*tex->height*3;
				ifmt = FMT_BGR8;
			}else if(header.ddspf.dwRGBBitCount==8){
				texSize = tex->width*tex->height;
				ifmt = GL_LUMINANCE;
			}else{
  				Log("DDS: fourCC is 0, size %d, flags %X, bitCount %d, mask %X%X%X%X\n",header.ddspf.dwSize,
					header.ddspf.dwFlags, header.ddspf.dwRGBBitCount, header.ddspf.dwRBitMask, header.ddspf.dwGBitMask, header.ddspf.dwBBitMask, header.ddspf.dwABitMask);
			}
  		}
	}
	if(texSize==0){
		g_fs.Close(input);
		delete tex;
		char fourCC[5]={0};
		memcpy(fourCC,&header.ddspf.dwFourCC,4);
		Log("LoadDDS (%s) Unsuported format %s(%d)\n", fileName, fourCC,header.ddspf.dwFourCC);
		return 0;
	}

	GLubyte *texData = new GLubyte[texSize];
	//input.seekg(offset);
	input->Read(texData, texSize);
	g_fs.Close(input);

	if(ifmt==FMT_BGR8){
		ResampleBGR(texData,texSize);
		ifmt = GL_RGB;
	}

	tex->Create(tex->width, tex->height);
	tex->SetWrap(GL_REPEAT);
	tex->SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

#ifndef ANDROID
	glTexParameteri(tex->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
#endif
	if(compressed){
		tex->UploadCompressed(ifmt, texSize, texData);
	}else{
		tex->Upload(0,ifmt,texData);
	}
	if(CheckGLError("LoadDDS Upload", __FILE__, __LINE__))
		Log("GL error while loading %s\n", fileName);

	glGenerateMipmap(tex->target);

	delete[] texData;
	
	return tex;
}
