
#pragma once

#include "graphics/texture.h"

class FrameBufferObject
{
public:
	FrameBufferObject()
	{
		id = 0;
		tex = 0;
		lvl = 0;
		depthTex = 0;
	}
	bool Create();
	void Bind();
	static void Unbind();
	void SetupViewport();
	void Resize(int w, int h);
	Texture* CreateTexture(int w, int h, int filter = GL_NEAREST);
	void AttachTexture(Texture *aTex, int level = 0, bool alloc=false);
	void AttachCubemap(Texture *aTex, int face, int level = 0);
	void BindTexture();
	Texture* CreateDepthTexture(int w=256, int h=256);
	Texture* CreateDepthStencilTexture(int w=256, int h=256);
	void BindDepthTexture();
	void AddDepthBuffer(bool depth24=false);
	void AddDepthStencilBuffer();
	void Clear(int mask=0);

	Texture *tex;
private:
	GLuint id;
	GLuint depthid;
	int lvl;
	Texture *depthTex;
};
