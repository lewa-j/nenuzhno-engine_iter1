
#include <cstdlib>//exit()

#include "engine.h"
#include "log.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"

#ifdef ANDROID
#include <GLES2/gl2ext.h>
#define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif

bool FrameBufferObject::Create()
{
#ifndef ANDROID
	if(!glGenFramebuffers)
		EngineError("FBO::Create gl func is null");
#endif
	glGenFramebuffers(1, &id);
	Log("Created FBO %d\n", id);
	if(!id)
		return false;
	glBindFramebuffer(GL_FRAMEBUFFER, id);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void FrameBufferObject::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	SetupViewport();//remove?
}

void FrameBufferObject::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferObject::SetupViewport()
{
	if(tex)
		glViewport(0, 0, tex->width>>lvl, tex->height>>lvl);
	else if(depthTex)
		glViewport(0, 0, depthTex->width, depthTex->height);
}

Texture* FrameBufferObject::CreateTexture(int w, int h, int filter)
{
	/*if(tex){
		EngineError("FrameBufferObject::CreateTexture called for fbo with texture\n");
	}*/

	Log("FBO %d CreateTexture(%d,%d)\n",id,w,h);
	tex = new Texture();
	tex->Create(w, h);
	tex->SetWrap(GL_CLAMP_TO_EDGE);
	tex->SetFilter(filter, filter);
	tex->Upload(0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(tex->target, 0, GL_RGBA4, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 0);
	AttachTexture(tex, 0);

	CheckGLError("FBO::CreateTexture", __FILE__, __LINE__);
	return tex;
}

void FrameBufferObject::Resize(int w, int h)
{
	if(tex)
	{
		tex->Bind();
		tex->Upload(0, w, h, 0);
		//glTexImage2D(tex->target, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}
	if(depthTex)
	{
		depthTex->Bind();
		depthTex->Upload(0,w,h,0);
		//glTexImage2D(depthTex->target, 0, GL_DEPTH_COMPONENT, w, h, 0,GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
	}
	glBindTexture(GL_TEXTURE_2D,0);
	if(depthid){
		glBindRenderbuffer(GL_RENDERBUFFER, depthid);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, tex->width, tex->height);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	Log("FBO %d Resize(%d,%d)\n",id,w,h);
}

void FrameBufferObject::AttachTexture(Texture *aTex, int level, bool alloc)
{
	//TODO check !gles or extension
/*#ifdef ANDROID
	if(level!=0){
		Log("FBO %d: AttachTexture %d: level!=0(%d)\n",id,aTex->id,level);
	}
#endif*/
	lvl = level;
	tex = aTex;
	//glBindFramebuffer(GL_FRAMEBUFFER, id);
	if(alloc)
	{
		tex->Bind();
		glTexImage2D(tex->target, level, GL_RGBA, tex->width>>level, tex->height>>level, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		//glTexImage2D(tex->target, level, GL_RGBA, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex->target, tex->id, level);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CheckGLError("FBO::AttachTexture", __FILE__, __LINE__);
}

void FrameBufferObject::AttachCubemap(Texture *aTex, int face, int level)
{
	tex = aTex;
	lvl = level;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, tex->id, level);

	CheckGLError("FBO::AttachCubemap", __FILE__, __LINE__);
}

void FrameBufferObject::BindTexture()
{
	tex->Bind();
}

Texture *FrameBufferObject::CreateDepthTexture(int w, int h)
{
	if(depthTex){
		EngineError("FrameBufferObject::CreateDepthTexture called for fbo with texture\n");
	}

	if(tex){
		w = tex->width;
		h = tex->height;
	}

	Log("FBO %d CreateDepthTexture(%d,%d)\n",id,w,h);

	depthTex = new Texture();
	depthTex->Create(w, h);
	depthTex->SetWrap(GL_CLAMP_TO_EDGE);
	depthTex->SetFilter(GL_NEAREST, GL_NEAREST);
#ifndef ANDROID
	//TODO check extension
	depthTex->SetFilter(GL_LINEAR, GL_LINEAR);
#endif
	//depthTex->Upload(0, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
	depthTex->Upload(0, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
	//glTexImage2D(depthTex->target, 0, GL_DEPTH_COMPONENT, depthTex->width, depthTex->height, 0,GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex->target, depthTex->id, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CheckGLError("FBO::CreateDepthTexture", __FILE__, __LINE__);
	return depthTex;
}

Texture *FrameBufferObject::CreateDepthStencilTexture(int w, int h)
{
	if(depthTex)
	{
		EngineError("FrameBufferObject::CreateDepthStencilTexture called for fbo with texture\n");
	}
	
	if(tex)
	{
		w = tex->width;
		h = tex->height;
	}
	
	depthTex = new Texture();
	depthTex->Create(w, h);
	depthTex->SetWrap(GL_CLAMP_TO_EDGE);
	depthTex->SetFilter(GL_NEAREST, GL_NEAREST);

	depthTex->Upload(0, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
	//glTexImage2D(depthTex->target, 0, GL_DEPTH24_STENCIL8, depthTex->width, depthTex->height, 0,GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex->target, depthTex->id, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CheckGLError("FBO::CreateDepthStencilTexture", __FILE__, __LINE__);
	return depthTex;
}

void FrameBufferObject::BindDepthTexture()
{
	depthTex->Bind();
}

void FrameBufferObject::AddDepthBuffer(bool depth24)
{
	glGenRenderbuffers(1, &depthid);

	glBindRenderbuffer(GL_RENDERBUFFER, depthid);
	//TODO Check extension
	if(depth24)
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, tex->width, tex->height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, tex->width, tex->height);

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,depthid);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CheckGLError("FBO::AddDepthBuffer", __FILE__, __LINE__);
}

void FrameBufferObject::AddDepthStencilBuffer()
{
	glGenRenderbuffers(1, &depthid);

	glBindRenderbuffer(GL_RENDERBUFFER, depthid);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex->width, tex->height);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthid);
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthid);

	CheckGLError("FBO::AddDepthStencilBuffer", __FILE__, __LINE__);
}

void FrameBufferObject::Clear(int aMask)
{
	if(!aMask)
		return;
	int mask=0;
	if(aMask&1)
		mask|=GL_COLOR_BUFFER_BIT;
	if(aMask&2)
		mask|=GL_DEPTH_BUFFER_BIT;
	if(aMask&4)
		mask|=GL_STENCIL_BUFFER_BIT;
	glClear(mask);
}
