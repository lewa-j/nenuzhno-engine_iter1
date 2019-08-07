
#include <math.h>
#include "BokehBlur.h"
#include "graphics/glsl_prog.h"
#include "graphics/ArrayBuffer.h"
#include "graphics/fbo.h"

#define min(x,y) (x<y?x:y)

#define USHORT_VERTS 0

BokehBlur::BokehBlur(){
	progBokeh=0;
	u_size=-1;
	vboGrid=0;
	gridW=0;
	gridH=0;
	texCore=0;
	texSrc=0;
}

BokehBlur::~BokehBlur(){
	if(progBokeh)
		delete progBokeh;
	if(vboGrid)
		delete vboGrid;
	if(texCore)
		delete texCore;
}

bool BokehBlur::Init(int w, int h, Texture *src){
	
	progBokeh = new glslProg();
	progBokeh->CreateFromFile("bokeh","bokeh");
	u_size = progBokeh->GetUniformLoc("u_size");
	int u_coreTex = progBokeh->GetUniformLoc("u_coreTex");
	progBokeh->Use();
	glUniform1i(u_coreTex,1);
	glUseProgram(0);
	
	gridW=w;
	gridH=h;
#if USHORT_VERTS
	GLushort *gridVerts = new GLushort[w*h*2];
	float stepx = 65536/(float)w;
	float stepy = 65536/(float)h;
	for(int i=0;i<w*h;i++){
		int x = i%w;
		int y = i/w;
		gridVerts[i*2]=(x+0.5f)*stepx;
		gridVerts[i*2+1]=(y+0.5f)*stepy;
	}
	vboGrid = new VertexBufferObject();
	vboGrid->Create();
	vboGrid->Upload(w*h*2*2,gridVerts);
	delete[] gridVerts;
#else
	float *gridVerts = new float[w*h*2];
	float stepx = 1.0f/(float)w;
	float stepy = 1.0f/(float)h;
	for(int i=0;i<w*h;i++){
		int x = i%w;
		int y = i/w;
		gridVerts[i*2]=(x+0.5f)*stepx;
		gridVerts[i*2+1]=(y+0.5f)*stepy;
	}
	vboGrid = new VertexBufferObject();
	vboGrid->Create();
	vboGrid->Upload(w*h*2*4,gridVerts);
	delete[] gridVerts;
#endif
	
	GLubyte *coreTexData = new GLubyte[32*32];
	for(int i=0;i<32*32;i++){
		float x = i%32-15.5f;
		float y = i/32-15.5f;
		coreTexData[i]=0;
		float l = sqrt(x*x+y*y);
		if(l<15.0f){
			coreTexData[i]=127*min((15.0f-l)*0.5,1.0f);
		}
	}
	texCore=new Texture();
	texCore->Create(32,32);
	texCore->SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	texCore->Upload(0,GL_LUMINANCE,GL_LUMINANCE,GL_UNSIGNED_BYTE,coreTexData);
	glGenerateMipmap(texCore->target);
	delete[] coreTexData;
	glBindTexture(GL_TEXTURE_2D,0);
	
	texSrc=src;

	return true;
}

void BokehBlur::Render(float size, Texture *src){
	progBokeh->Use();
	glUniform1f(u_size,size);
	
	glActiveTexture(GL_TEXTURE1);
	texCore->Bind();
	glActiveTexture(GL_TEXTURE0);
	if(src)
		src->Bind();
	else
		texSrc->Bind();
	
	glEnable(GL_BLEND);
	glBlendFunc(1,1);
	vboGrid->Bind();
#if USHORT_VERTS
	glVertexAttribPointer(0,2,GL_UNSIGNED_SHORT,GL_TRUE,4,0);
#else
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,8,0);
#endif
	glDrawArrays(GL_POINTS,0,gridW*gridH);
	vboGrid->Unbind();
	glDisable(GL_BLEND);
}

