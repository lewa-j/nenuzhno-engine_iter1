
#include <string>
#include <fstream>
#include <cstring>
using namespace std;

#include "log.h"
#include "system/FileSystem.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/texture.h"
#include "resource/vtf.h"

size_t computeMipsSize(int width, int height, int mipcount, float bpp);

bool VTFLoader::CheckExt(const char *name)
{
	return strstr(name,".vtf")!=0;
}

Texture *VTFLoader::Load(const char *fileName)
{
	return Load(fileName,0);
}

Texture *VTFLoader::Load(const char *fileName, int mip)
{
	string filePath = "materials/"+string(fileName);
	if(filePath.find(".vtf") == string::npos)
		filePath += ".vtf";

	IFile *input = g_fs.Open(filePath.c_str());

	if(!input){
		//LOG( "File: %s not found\n", path);
		return 0;
	}

	VTFFileHeader_t header;
	input->Read(&header, sizeof(VTFFileHeader_t));

	if(header.width!=header.height){
		//LOG("%s(%dx%d) is not quad\n",fileName,header.width,header.height);
	}

	if(header.flags&0x4000)
		mip = 0;
	if(mip >= header.numMipLevels)
		mip = header.numMipLevels-1;

	Texture *tex = new Texture();

	tex->width = header.width>>mip;
	tex->height = header.height>>mip;
	if(tex->width < 4)
		tex->width = 4;
	if(tex->height < 4)
		tex->height = 4;

	size_t texSize = 0;
	size_t offset = header.headerSize;
	int faces = 1;
	if(header.flags&0x4000){
		if(header.version[1] < 5)
			faces = 7;
		else
			faces = 6;
	}else{
		offset += ((header.lowResImageWidth+3)/4)*((header.lowResImageHeight+3)/4)*8;
	}
	int ifmt;
	int fmt;
	int type = GL_UNSIGNED_BYTE;
	bool compressed = false;
	if(header.imageFormat == IMAGE_FORMAT_DXT1){//4bpp
		texSize = ((tex->width + 3) / 4) * ((tex->height + 3) / 4) * 8;
		offset += computeMipsSize(tex->width,tex->height,header.numMipLevels-mip,0.5f)*faces;
		ifmt = GL_COMPRESSED_RGB_S3TC_DXT1;
		fmt = GL_RGB;
		compressed=true;
	}else if(header.imageFormat == IMAGE_FORMAT_DXT5){//8bpp
		texSize = ((tex->width + 3) / 4) * ((tex->height + 3) / 4) * 16;
		offset += computeMipsSize(tex->width,tex->height,header.numMipLevels-mip,1)*faces;
		ifmt = GL_COMPRESSED_RGBA_S3TC_DXT5;
		fmt = GL_RGBA;
		compressed=true;
	}else if(header.imageFormat == IMAGE_FORMAT_RGBA16161616F){//64bpp
		Log("image format RGBA16F (%s)\n",fileName);
		texSize = tex->width * tex->height * 8;
		offset += computeMipsSize(tex->width,tex->height,header.numMipLevels-mip,8)*faces;
		ifmt = GL_RGBA16F;
		fmt = GL_RGBA;
		type = GL_HALF_FLOAT;
	}else if(header.imageFormat == IMAGE_FORMAT_BGRA8888){//32bpp
		Log("image format BGRA8 (%s)\n",fileName);
		texSize = tex->width * tex->height * 4;
		offset += computeMipsSize(tex->width,tex->height,header.numMipLevels-mip,4)*faces;
		ifmt = GL_RGBA;
		fmt = GL_BGRA;
	}else if(header.imageFormat == IMAGE_FORMAT_BGR888){//24bpp
		Log("image format BGR8 (%s)\n",fileName);
		texSize = tex->width * tex->height * 3;
		offset += computeMipsSize(tex->width,tex->height,header.numMipLevels-mip,3)*faces;
		ifmt = GL_RGB;
		fmt = FMT_BGR8;
		//need resample
	}else{
		Log("Unknown image format %d (%s)\n", header.imageFormat,fileName);
		g_fs.Close(input);
		delete tex;
		return 0;
	}

	//LOG("texSize %d offset %d\n", texSize, offset);

	GLubyte *texData = new GLubyte[texSize*faces];
	input->Seek(offset);
	input->Read(texData, texSize*faces);
	g_fs.Close(input);
	
	//add resample flag or something like that
	if(fmt==FMT_BGR8){
		ResampleBGR(texData,texSize*faces);
		fmt=GL_RGB;
	}

	//TODO extension
	if(fmt==GL_BGRA){
		ResampleBGRA(texData,texSize*faces);
		fmt=GL_RGBA;
	}

	if(header.flags&0x4000){
		//LOG("%s is a cubemap! size %dx%d mips %d fmt %d ifmt %x type %x\n",fileName,width,height,header.numMipLevels, header.imageFormat,ifmt,type);
		tex->target = GL_TEXTURE_CUBE_MAP;
		tex->Create(tex->width, tex->height);
		tex->SetFilter(GL_LINEAR, GL_LINEAR);
		if(CheckGLError("LoadVTF cubemap create", __FILE__, __LINE__))
			Log("GL error while loading %s\n",fileName);

		glTexParameteri(tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		CheckGLError("LoadVTF cubemap Parameters", __FILE__, __LINE__);

		if(!compressed){
			for(int i=0;i<6;i++){
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, ifmt, tex->width, tex->height, 0, fmt, type, texData+texSize*i);
			}
		}else{
//#ifdef ANDROID
			//GLubyte *newData = new GLubyte[width*height*4];
			for(int i=0;i<6;i++){
				tex->target=GL_TEXTURE_CUBE_MAP_POSITIVE_X+i;
				tex->UploadCompressed(ifmt,texSize,texData+texSize*i);
				//DecompressDXT(this, texData+texSize*i, newData, texSize, header.imageFormat);
				//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, fmt, width, height, 0, fmt, type, newData);
			}
			tex->target = GL_TEXTURE_CUBE_MAP;
			//delete[] newData;
/*#else
			for(int i=0;i<6;i++)
			{
				glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,ifmt, width, height, 0, texSize, texData+texSize*i);
				//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, ifmt, width, height, 0, fmt, type, texData+texSize*i);
			}
#endif*/

		}
		tex->SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		glGenerateMipmap(tex->target);
		CheckGLError("LoadVTF env upload", __FILE__, __LINE__);
	}else{
		tex->Create(tex->width, tex->height);
		tex->SetWrap(GL_REPEAT);
		tex->SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

#ifndef ANDROID
		glTexParameteri(tex->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
#endif
		//if(header.imageFormat == IMAGE_FORMAT_DXT1||header.imageFormat == IMAGE_FORMAT_DXT5)
		if(compressed){
			//Upload(0, GL_RGBA_DXT5_S3TC, texData);
			tex->UploadCompressed(ifmt,texSize,texData);
		}else if (header.imageFormat==IMAGE_FORMAT_BGRA8888||header.imageFormat==IMAGE_FORMAT_BGR888){
			tex->Upload(0, ifmt,fmt,type, texData);
		}
		else
			Log("wut?\n");
		if(CheckGLError("LoadVTF Upload", __FILE__, __LINE__))
			Log("GL error while loading %s\n",fileName);
		glGenerateMipmap(tex->target);
	}

	delete[] texData;

	return tex;
}

size_t computeMipsSize(int width, int height, int mipcount, float bpp)
{
	size_t imgSize = 0;

	width>>=1;
	height>>=1;
	if(bpp<3)
	{
		if(width < 4)
			width = 4;

		if(height < 4)
			height = 4;
	}
	for(int i=0; i<mipcount-1; i++)
	{
		imgSize+=(width*height*bpp);

		width>>=1;
		height>>=1;
		if(bpp<3)
		{
			if(width < 4)
				width = 4;

			if(height < 4)
				height = 4;
		}
	}

	return imgSize;
}

/*
size_t computeMipsSize(int width, int height, int mipcount, float bpp)
{
	size_t imgSize = 0;

	width>>=1;
	height>>=1;
	if(width < 4)
		width = 4;

	if(height < 4)
		height = 4;

	for(int i=1; i<mipcount; i++)
	{
		imgSize+=(width*height*bpp);

		width>>=1;
		height>>=1;

		if(width < 4)
			width = 4;

		if(height < 4)
			height = 4;
	}

	return imgSize;
}
*/
