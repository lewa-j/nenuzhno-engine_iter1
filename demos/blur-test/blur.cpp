
#include <math.h>
#include <gtc/type_ptr.hpp>
#include "log.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/glsl_prog.h"
#include "graphics/ArrayBuffer.h"
#include "graphics/fbo.h"
#include "graphics/gl_ext.h"

#include "renderer/BokehBlur.h"
#include "game/IGame.h"

class BlurGame : public IGame{
public:
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "blur";
	}

	void DrawBlur();
};

IGame *CreateGame(){
	return new BlurGame();
}

#define min(x,y) (x<y?x:y)

//TODO: GL_OES_vertex_array_object

int scrWidth=0;
int scrHeight=0;

glslProg *progTex;
glslProg *progMip;
glslProg *progBin;
VertexBufferObject *vboQuad;
VertexBufferObject *vboTri;
Texture *tex1;
Texture *tex2;
Texture *texWhite;
FrameBufferObject *fbo1;
Texture *texFb1;
glm::mat4 mvpMtx(1.0f);

BokehBlur bokeh;
float bokehSize=20.0f;

void BlurGame::Created()
{

	GLExtensions::Init();
	
	progTex = new glslProg();
	progTex->CreateFromFile("generic", "col_tex");
	progTex->u_mvpMtx = progTex->GetUniformLoc("u_mvpMtx");
	progTex->u_color = progTex->GetUniformLoc("u_color");
	progTex->Use();
	glUniform4f(progTex->u_color,1,1,1,1);
	
	progMip = new glslProg();
	progMip->CreateFromFile("generic", "tex_mip");
	progMip->u_mvpMtx = progMip->GetUniformLoc("u_mvpMtx");
	
	//
	glslProg tempProg("#version 100\nattribute vec4 a_p;void main(){gl_Position = a_p;}",
		"#version 100\nprecision highp float; void main(){gl_FragColor = vec4(0.5);}");
	//"precision mediump float; attribute vec4 a_position;uniform mat4 u_mvpMtx; void main(){gl_Position = u_mvpMtx * a_position;gl_PointSize = 8.0;}", 
	//"precision highp float;void main(){gl_FragColor = vec4(0.5);}");
	
	tempProg.Save("test");
	
	char *tempData = 0;
	int tempLen = 0;
	int tempFmt = 0;
	if(!tempProg.GetBinaryData(&tempFmt,&tempData,&tempLen)){
		Log("Can't get shader prog data\n");
	}else{
		progBin = new glslProg();
		progBin->CreateFromBinary(tempFmt,tempData,tempLen);
	}
	if(tempData)
		delete[] tempData;
	
	glUseProgram(0);

	//*((int*)0)=42;

	float quadVerts[] ={
	 1,-1,1,0,
	 1, 1,1,1,
	-1, 1,0,1,
	-1,-1,0,0
	};
	vboQuad = new VertexBufferObject();
	vboQuad->Create();
	vboQuad->Upload(4*4*4, quadVerts);
	
	float triVerts[] =
	{
	 0.8,-0.6,1,0,
	 0.2, 0.9,1,1,
	-0.9,-0.5,0.1,0.3
	};
	vboTri = new VertexBufferObject();
	vboTri->Create();
	vboTri->Upload(3*4*4, triVerts);
	
	GLubyte whiteTexData[]={
		255
	};
	texWhite = new Texture();
	texWhite->Create(1,1);
	texWhite->Upload(0, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, whiteTexData);
	
	GLushort testTexData[] =
	{
		0x0000,0x444F,0x888F,0xCCCF, 0x400F,0x800F,0xC00F,0xF00F, 0x040F,0x080F,0x0C0F,0x0F0F, 0x004F,0x008F,0x00CF,0x00FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x444F,0x844F,0xC44F,0xF44F, 0x444F,0x484F,0x4C4F,0x4F4F, 0x444F,0x448F,0x44CF,0x44FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x488F,0x888F,0xC88F,0xF88F, 0x848F,0x888F,0x8C8F,0x8F8F, 0x884F,0x888F,0x88CF,0x88FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x4ccF,0x8ccF,0xCccF,0xFccF, 0xc4cF,0xc8cF,0xcCcF,0xcFcF, 0xcc4F,0xcc8F,0xccCF,0xccFF,
		0x0000,0x444F,0x888F,0xCCCF, 0x440F,0x840F,0xC40F,0xF40F, 0x440F,0x480F,0x4C0F,0x4F0F, 0x404F,0x408F,0x40CF,0x40FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x480F,0x880F,0xC80F,0xF80F, 0x840F,0x880F,0x8C0F,0x8F0F, 0x804F,0x808F,0x80CF,0x80FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x4c0F,0x8c0F,0xCc0F,0xFc0F, 0xc40F,0xc80F,0xcC0F,0xcF0F, 0xc04F,0xc08F,0xc0CF,0xc0FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x4f0F,0x8f0F,0xCf0F,0xFf0F, 0xf40F,0xf80F,0xfC0F,0xfF0F, 0xf04F,0xf08F,0xf0CF,0xf0FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x404F,0x804F,0xC04F,0xF04F, 0x044F,0x084F,0x0C4F,0x0F4F, 0x044F,0x048F,0x04CF,0x04FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x408F,0x808F,0xC08F,0xF08F, 0x048F,0x088F,0x0C8F,0x0F8F, 0x084F,0x088F,0x08CF,0x08FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x40cF,0x80cF,0xC0cF,0xF0cF, 0x04cF,0x08cF,0x0CcF,0x0FcF, 0x0c4F,0x0c8F,0x0cCF,0x0cFF,
		0x0000,0x444F,0x888F,0xCCCF, 0x40fF,0x80fF,0xC0fF,0xF0fF, 0x04fF,0x08fF,0x0CfF,0x0FfF, 0x0f4F,0x0f8F,0x0fCF,0x0fFF,
		0x0000,0x444F,0x888F,0xCCCF, 0x400F,0x800F,0xC00F,0xF00F, 0x040F,0x080F,0x0C0F,0x0F0F, 0x004F,0x008F,0x00CF,0x00FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x400F,0x800F,0xC00F,0xF00F, 0x040F,0x080F,0x0C0F,0x0F0F, 0x004F,0x008F,0x00CF,0x00FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x400F,0x800F,0xC00F,0xF00F, 0x040F,0x080F,0x0C0F,0x0F0F, 0x004F,0x008F,0x00CF,0x00FF,
		0x0000,0x444F,0x888F,0xCCCF, 0x400F,0x800F,0xC00F,0xF00F, 0x040F,0x080F,0x0C0F,0x0F0F, 0x004F,0x008F,0x00CF,0x00FF
	};
	
	tex1 = new Texture();
	tex1->Create(16,16);
	tex1->SetWrap(GL_CLAMP_TO_EDGE);
	//tex1->SetFilter(GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
	tex1->SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	tex1->Upload(0, GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, testTexData);
	tex1->Upload(1, GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, testTexData);
	tex1->Upload(2, GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, testTexData);
	tex1->Upload(3, GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, testTexData+9);
	tex1->Upload(4, GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, testTexData+38);
	
	tex2 = new Texture();
	tex2->Create(64,64);
	tex2->Upload(0,GL_RGBA4,GL_RGBA,GL_UNSIGNED_SHORT_4_4_4_4,0);
/*	tex2->SetFilter(GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
	for(int i=1;i<7;i++){
		tex2->Upload(i,GL_RGBA4,GL_RGBA,GL_UNSIGNED_SHORT_4_4_4_4,0);
	}
	tex2->Upload(2,GL_RGBA4,GL_RGBA,GL_UNSIGNED_SHORT_4_4_4_4,testTexData);
*/
	texFb1 = new Texture();
	texFb1->Create(64,64);
	texFb1->SetFilter(GL_LINEAR, GL_LINEAR);
	texFb1->Upload(0,GL_RGBA,GL_RGBA,GL_UNSIGNED_BYTE,0);

	fbo1 = new FrameBufferObject();
	fbo1->Create();
	fbo1->AttachTexture(tex2, 0);
	
	bokeh.Init(64,64,tex2);
	
	glBindTexture(GL_TEXTURE_2D,0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifndef ANDROID
//TODO: add defines to header
	if(!(GLExtensions::extFlags&eGLES)){
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glEnable(GL_POINT_SPRITE);
	}
#endif
	CheckGLError("Created", __FILE__, __LINE__);
	Log("Init done\n");
}

void BlurGame::Changed(int w, int h)
{
	scrWidth=w;
	scrHeight=h;
	CheckGLError("Changed", __FILE__, __LINE__);
}
float a=0;
void BlurGame::Draw()
{
	a+=0.02f;

	progTex->Use();
	glUniformMatrix4fv(progTex->u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);

	DrawBlur();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(2);
	glBindTexture(GL_TEXTURE_2D,0);
	
	CheckGLError("Draw", __FILE__, __LINE__);
}

void BlurGame::DrawBlur()
{
	bokehSize=(sin(a)+1.0f)*10.0f+0.1f;
	//Log("boken size %f\n",bokehSize);

#if 1
	fbo1->Bind();
	fbo1->AttachTexture(tex2, 0);
	fbo1->SetupViewport();
	glClear(GL_COLOR_BUFFER_BIT);
	
	tex1->Bind();
	vboTri->Bind();
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,0);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,(void*)8);
	glDrawArrays(GL_TRIANGLES,0,3);
	vboTri->Unbind();
#if RENDER_TO_MIP
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	fbo1->Bind();
	fbo1->AttachTexture(tex2, 3);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
#if WHITE_LINE
	texWhite->Bind();
	float lineVerts[]={
		-1,0.5,0,0,
		1,-0.5,0,0
	};
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,lineVerts);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,lineVerts+2);
	glDrawArrays(GL_LINES,0,2);
#endif
	Texture *outTex=tex2;
#define POSTPROCESS 1
#if POSTPROCESS
	fbo1->AttachTexture(texFb1, 0);
	fbo1->SetupViewport();
	glClear(GL_COLOR_BUFFER_BIT);

	glDisableVertexAttribArray(2);
	bokeh.Render(bokehSize);
	glEnableVertexAttribArray(2);

	outTex=texFb1;
#endif
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glViewport(0,0,scrWidth,scrHeight);
	int d = min(scrWidth,scrHeight);
	glViewport(0,0,d,d);
	glClear(GL_COLOR_BUFFER_BIT);
	
	progTex->Use();
	
	vboQuad->Bind();
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,0);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,(void*)8);
	//tex2->Bind();
	//glDrawArrays(GL_TRIANGLE_FAN,0,4);
	outTex->Bind();
	//glEnable(GL_BLEND);
	glBlendFunc(1,1);
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	glDisable(GL_BLEND);
	vboQuad->Unbind();

#else
	int d = min(scrWidth,scrHeight);
	glViewport(0,0,d,d);
	progTex->Use();

	bokeh.texCore->Bind();
	vboQuad->Bind();
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,0);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,(void*)8);
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	vboQuad->Unbind();
#endif

#if SHOW_MIPS
	progMip->Use();
	glUniformMatrix4fv(progMip->u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));

	tex2->Bind();
	vboQuad->Bind();
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,0);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,(void*)8);
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	vboQuad->Unbind();
#endif
}

