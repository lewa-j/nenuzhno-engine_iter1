
#include "stdint.h"

#include <gtc/type_ptr.hpp>

#include "log.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/gl_ext.h"
#include "graphics/glsl_prog.h"
#include "graphics/ArrayBuffer.h"
#include "graphics/vao.h"
#include "graphics/texture.h"

#ifdef ANDROID
#include "GLES2/gl2ext.h"
#ifndef GL_COMPRESSED_RGB8_ETC2
#define GL_COMPRESSED_RGB8_ETC2 0x9274
#endif
#else
#define GL_ETC1_RGB8_OES 0x8D64
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03

#endif

#include "game/IGame.h"

class ComprTexGame : public IGame{
public:
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "base";
	}
};

IGame *CreateGame(){
	return new ComprTexGame();
}

float quadVerts[] =
{
	 1,-1,
	 1, 1,
	-1, 1,
	-1,-1
};

int scrWidth = 0;
int scrHeight = 0;
float aspect = 1.0f;

glslProg progQuadTexture;
GLint u_quad_transform;
Texture etc1Texture;
Texture pvrTex;
VertexBufferObject vboQuad;
VertexArrayObject vaoQuad;

void ComprTexGame::Created()
{
	CheckGLError("pre Created", __FILE__, __LINE__);
	GLExtensions::Init();
	
	progQuadTexture.CreateFromFile("quad", "col_tex");
	u_quad_transform = progQuadTexture.GetUniformLoc("u_transform");
	progQuadTexture.u_color = progQuadTexture.GetUniformLoc("u_color");
	progQuadTexture.Use();
	glUniform4f(progQuadTexture.u_color,1,1,1,1);
	
	glUseProgram(0);
	CheckGLError("Created shaders", __FILE__, __LINE__);

	vboQuad.Create();
	vboQuad.Upload(2*4*4, quadVerts);

	vaoQuad.Create();
	vaoQuad.Bind();
	
	vboQuad.Bind();
	vaoQuad.SetAttribute(0,2,GL_FLOAT,GL_FALSE,8,0);

	vaoQuad.Unbind();

	vboQuad.Unbind();
	CheckGLError("Created meshes", __FILE__, __LINE__);

#if 0
	GLubyte testTexData[] =
	{
		0,0,0, 255,255,255, 0,0,0, 255,255,255,
		255,255,255, 0,0,0, 255,255,255, 0,0,0,
		0,0,0, 255,255,255, 0,0,0, 255,255,255,
		255,255,255, 0,0,0, 255,255,255, 0,0,0
	};
	etc1Texture.Create(4,4);
	etc1Texture.Upload(0, GL_RGB, testTexData);
#else
	GLubyte etc1Data[]={
		0b01110111,
		0b01110111,
		0b01110111,
		0b11111100,
		
		0b00000110,
		0b10100010,
		0b00001110,
		0b11101110
	};
	etc1Texture.Create(4,4);
	etc1Texture.UploadCompressed(GL_COMPRESSED_RGB8_ETC2, 8, etc1Data);//GL_ETC1_RGB8_OES
	CheckGLError("Upload etc1", __FILE__, __LINE__);
#endif
#if 0
	/*For PVRTC 4BPP formats the imageSize is calculated as:
		( max(width, 8) * max(height, 8) * 4 + 7) / 8
	For PVRTC 2BPP formats the imageSize is calculated as:
		( max(width, 16) * max(height, 8) * 2 + 7) / 8*/
	
	//pvrtc block word (reversed)
	//1 col a mode, 555 col a, 1 col b mode, 554 col b, 1 mode, 32 modul table (2b per sample)
	GLubyte pvrData[32]={
		255,255,0,0,0b11111110,0b11111111,0b00000000,0b11111100,
		255,255,0,0,0b11100000,0b11111111,0b11100000,0b10000011,
		255,255,0,0,0b00011110,0b11111100,0b00011111,0b10000000,
		255,255,0,0,0b11111110,0b10000011,0b00000000,0b10000000
		/*
		255,170,85,0,0b11111110,0b11111111,0b00000000,0b11111100,
		0,85,170,255,0b11100000,0b11111111,0b11100000,0b10000011,
		255,170,85,0,0b00011110,0b11111100,0b00011111,0b10000000,
		0,85,170,255,0b11111110,0b10000011,0b00000000,0b10000000
		*/
	};
	
	pvrTex.Create(8,8);
	pvrTex.UploadCompressed(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 32, pvrData);
	CheckGLError("Upload pvr", __FILE__, __LINE__);
#endif

	glBindTexture(GL_TEXTURE_2D, 0);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
//	glEnable(GL_CULL_FACE);
	CheckGLError("Created", __FILE__, __LINE__);
}

void ComprTexGame::Changed(int w, int h)
{
	scrWidth = w;
	scrHeight = h;
	glViewport(0, 0, w, h);
	aspect = w/(float)h;
}

void ComprTexGame::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	CheckGLError("Clear", __FILE__, __LINE__);

	vaoQuad.Bind();
	CheckGLError("Bind plane", __FILE__, __LINE__);

	progQuadTexture.Use();
	glUniform4f(u_quad_transform, 0, 0, 0.8, 0.8*aspect);
	CheckGLError("Use shader", __FILE__, __LINE__);

	etc1Texture.Bind();
	//pvrTex.Bind();
	CheckGLError("Bind texture", __FILE__, __LINE__);
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	CheckGLError("Draw plane", __FILE__, __LINE__);

	vaoQuad.Unbind();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(0);
	CheckGLError("Draw", __FILE__, __LINE__);
}
