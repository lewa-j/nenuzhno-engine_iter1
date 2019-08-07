
#include <cstring>
#include <string>
#include <fstream>
#include <stdint.h>
using namespace std;

#include "log.h"
#include "system/FileSystem.h"

#include "graphics/platform_gl.h"
#include "graphics/texture.h"
#include "renderer/font.h"
#include "resource/ResourceManager.h"

struct bmfontInfoBlock_t
{
	uint16_t fontSize;
	uint8_t bitField;//bit 0: smooth, bit 1: unicode, bit 2: italic, bit 3: bold, bit 4: fixedHeigth, bits 5-7: reserved
	uint8_t charSet;
	uint16_t stretchH;
	uint8_t aa;
	uint8_t paddingUp;
	uint8_t paddingRight;
	uint8_t paddingDown;
	uint8_t paddingLeft;
	uint8_t spacingHoriz;
	uint8_t spacingVert;
	uint8_t outline;//added with version 2
	//fontName 	n+1	string 	14	null terminated string with length n
};

struct bmfontCommonBlock_t
{
	uint16_t lineHeight;
	uint16_t base;
	uint16_t scaleW;
	uint16_t scaleH;
	uint16_t pages;
	uint8_t bitField;//bits 0-6: reserved, bit 7: packed
	uint8_t alphaChnl;
	uint8_t redChnl;
	uint8_t greenChnl;
	uint8_t blueChn;
	//padding
};

struct bmfontChar_t
{
	uint32_t id;//These fields are repeated until all characters have been described
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	int16_t xoffset;
	int16_t yoffset;
	int16_t xadvance;
	uint8_t page;
	uint8_t chnl;
};

bool Font::LoadBMFont(const char *fileName,ResourceManager *resMan)
{
	string filePath = "fonts/"+string(fileName)+".fnt";
	IFile *file = g_fs.Open(filePath.c_str());
	if(!file){
		return false;
	}

	char ident[4];
	file->Read(ident, 4);

	if(ident[0]!='B'||ident[1]!='M'||ident[2]!='F'){
		Log("LoadBMFont: unknown format\n");
		g_fs.Close(file);
		return false;
	}
	//Log("BMFont \"%s\" version %d\n", fileName, ident[3]);

	char blockType = 0;
	int blockSize = 0;
	//info block
	file->Read(&blockType,1);
	if(blockType==1){
		file->Read(&blockSize, 4);
		char *block = new char[blockSize];
		file->Read(block, blockSize);
		//bmfontInfoBlock_t *infoBlock = (bmfontInfoBlock_t *)block;
		//LOG("font size %d, charset %d\n",infoBlock->fontSize,infoBlock->charSet);
		//LOG("name %s\n",block+14);

		delete[] block;
	}else{
		Log("first block type is not 1\n");
		g_fs.Close(file);
		return false;
	}

	//common block
	//int numPages=0;
	int texW=1;
	int texH=1;
	file->Read(&blockType,1);
	if(blockType==2){
		file->Read(&blockSize, 4);
		char *block = new char[blockSize];
		file->Read(block, blockSize);
		bmfontCommonBlock_t *commonBlock = (bmfontCommonBlock_t *)block;
		//LOG("line height %d, base %d, scale %d %d, pages %d\n",commonBlock->lineHeight,commonBlock->base,commonBlock->scaleW,commonBlock->scaleH,commonBlock->pages);
		//numPages = commonBlock->pages;
		texW = commonBlock->scaleW;
		texH = commonBlock->scaleH;
		base = (float)commonBlock->base/texW;
		delete[] block;
	}

	//pages block
	//TODO: multiple pages
	string pageName;
	file->Read(&blockType,1);
	if(blockType==3){
		file->Read(&blockSize, 4);
		char *block = new char[blockSize];
		file->Read(block, blockSize);
		//LOG("page 0: %s\n",block);
		pageName = block;
		delete[] block;
	}

	tex = resMan->GetTexture(("fonts/"+pageName).c_str());

	//chars block
	int numChars = 0;
	file->Read(&blockType,1);
	if(blockType==4){
		file->Read(&blockSize, 4);
		char *block = new char[blockSize];
		file->Read(block, blockSize);
		numChars = blockSize/20;//sizeof char
		//LOG("numChars %d\n",numChars);
		bmfontChar_t *curChar = (bmfontChar_t *)block;
		for(int i=0; i<numChars; i++){
			if(curChar->id>255){
				Log("LoadBMFont: char %d id %d too big (%s)\n",i,curChar->id,fileName);
				continue;
			}
			chars[curChar->id].x = (float)curChar->x/texW;
			chars[curChar->id].y = (float)curChar->y/texH;
			chars[curChar->id].width = (float)curChar->width/texW;
			chars[curChar->id].height = (float)curChar->height/texH;
			chars[curChar->id].xoffset = (float)curChar->xoffset/texW;
			chars[curChar->id].yoffset = (float)curChar->yoffset/texH;
			chars[curChar->id].xAdv = (float)curChar->xadvance/texW;
			curChar++;
		}

		delete[] block;
	}


	bmfont = true;
	g_fs.Close(file);
	return true;
}

void Font::Print(const char *text, float x, float y, float size)
{
	if(bmfont)
		PrintBMFont(text,x,y,size);
	else
		PrintOLD(text,x,y,size);
}

void Font::PrintBMFont(const char *text, float x, float y, float size)
{
	if(!tex)
		return;
	int l = strlen(text);
	if(!l)
		return;
	GLfloat *verts = new GLfloat[l*6*4];
	int cv = 0;
	float cx = x;
	float cy = y;

	for(int i=0; i<l; i++)
	{
		int c = text[i];

		if(c=='\n')
		{
			cx = x;
			cy -= base*size*1.1f;
			continue;
		}

		float tx = chars[c].x;
		float tw = chars[c].width;
		float ty = chars[c].y;
		float th = chars[c].height;
		float xo = chars[c].xoffset*size;
		float yo = (base-th-chars[c].yoffset)*size;

		verts[cv*4]=cx+xo;
		verts[cv*4+1]=cy+yo;
		verts[cv*4+2]=tx;
		verts[cv*4+3]=ty+th;
		cv++;
		verts[cv*4]=cx+xo;
		verts[cv*4+1]=cy+yo+size*th;
		verts[cv*4+2]=tx;
		verts[cv*4+3]=ty;
		cv++;
		verts[cv*4]=cx+xo+size*tw;
		verts[cv*4+1]=cy+yo;
		verts[cv*4+2]=tx+tw;
		verts[cv*4+3]=ty+th;
		cv++;

		verts[cv*4]=cx+xo+size*tw;
		verts[cv*4+1]=cy+yo;
		verts[cv*4+2]=tx+tw;
		verts[cv*4+3]=ty+th;
		cv++;
		verts[cv*4]=cx+xo;
		verts[cv*4+1]=cy+yo+size*th;
		verts[cv*4+2]=tx;
		verts[cv*4+3]=ty;
		cv++;
		verts[cv*4]=cx+xo+size*tw;
		verts[cv*4+1]=cy+yo+size*th;
		verts[cv*4+2]=tx+tw;
		verts[cv*4+3]=ty;
		cv++;

		cx += size * chars[c].xAdv;
	}

	tex->Bind();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, verts);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 16, verts+2);
	glDrawArrays(GL_TRIANGLES, 0, cv);

	delete[] verts;
}

void Font::PrintOLD(const char *text, float x, float y, float size)
{
//	static bool first = true;

	if(!tex)
		return;
	int l = strlen(text);
	GLfloat *verts=new GLfloat[l*6*4];
	int cv=0;
	float cx=x;
	float cy=y;
	size *= 0.125;

	for(int i=0; i<l; i++)
	{
		int c = tolower(text[i]);
		//if(first)
		//	LOG("c %d %c %d\n", i,text[i],c);
		if(c=='\n')
		{
			cx = x;
			cy -= size*1.1f;
			continue;
		}
		if(c>=' '&&c<='?')
		{
			c -= ' ';
			
		}
		else if(c>='a'&&c<='x')
		{
			c -= 'a';
			c += 32;
		}
		else
			c = -1;
		if(c>=0)
		{
			float tx = (c%8)/8.0f;
			float tw = 1.0f/8.0f;
			float ty = (c/8)/7.0f;
			float th = 1.0f/7.0f;
			verts[cv*4]=cx;
			verts[cv*4+1]=cy;
			verts[cv*4+2]=tx;
			verts[cv*4+3]=ty+th;
			cv++;
			verts[cv*4]=cx;
			verts[cv*4+1]=cy+size;
			verts[cv*4+2]=tx;
			verts[cv*4+3]=ty;
			cv++;
			verts[cv*4]=cx+size;
			verts[cv*4+1]=cy;
			verts[cv*4+2]=tx+tw;
			verts[cv*4+3]=ty+th;
			cv++;
			
			verts[cv*4]=cx+size;
			verts[cv*4+1]=cy;
			verts[cv*4+2]=tx+tw;
			verts[cv*4+3]=ty+th;
			cv++;
			verts[cv*4]=cx;
			verts[cv*4+1]=cy+size;
			verts[cv*4+2]=tx;
			verts[cv*4+3]=ty;
			cv++;
			verts[cv*4]=cx+size;
			verts[cv*4+1]=cy+size;
			verts[cv*4+2]=tx+tw;
			verts[cv*4+3]=ty;
			cv++;
		}
		cx+=size*13.0f/12.0f;
	}
	tex->Bind();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, verts);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 16, verts+2);
	glDrawArrays(GL_TRIANGLES, 0, cv);

	delete[] verts;
	//first=false;
}
