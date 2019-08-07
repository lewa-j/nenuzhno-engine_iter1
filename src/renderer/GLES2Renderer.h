#pragma once

#include "graphics/ArrayBuffer.h"
#include "graphics/glsl_prog.h"
#include "graphics/texture.h"
class Font;
#include "graphics/fbo.h"
class Mesh;

enum eDrawState{
	eDSDefault,
	eDSDepth
};

class GLES2Renderer: public IRenderer
{
public:
	GLES2Renderer();
	virtual void Init(int initFlags,ResourceManager *resMan);
	virtual void Resize(int w, int h);
	virtual void Draw();
	
	virtual void Clear();
	virtual void DrawText(const char *t,float x,float y,float s);
	virtual void DrawRect(float x, float y, float w, float h);
	virtual void DrawModel(Model *mdl,const glm::mat4 &mtx);
	
	virtual void SetScene(Scene *sc);
	virtual void SetCamera(Camera *cam);
	virtual void SetModelMtx(const glm::mat4 &mtx);
	void UpdateMtx();
	virtual void UseProg(glslProg *p);
	virtual void Set2DMode(bool m = true);
	virtual void ResetViewport();
	virtual void SetColor(float r, float g, float b, float a);
	virtual void AddButton(Button *b);
	virtual void SetBackBufferScale(float s);
	virtual FrameBufferObject *GetFBO();
	virtual void RenderDepth(Camera *cam);
	virtual void DrawBBox(BoundingBox *bbox);
	virtual void DrawCube();
	
	VertexBufferObject vboQuad;
	glslProg progTex;
	Texture texWhite;
	Font *font;
	
	glslProg progDepth;

	FrameBufferObject backFBO;
	float backBufferScale;

	glslProg *progCur;

	void DrawSkyBox(Texture *s);
	glslProg progSkybox;
	Mesh *meshCube;

	ILighting *lighting;
	
	eDrawState drawState;

	//debug
	void DrawDebugScene();
	struct debugBBox_t{
		BoundingBox *bbox;
		mat4 mtx;

		debugBBox_t(BoundingBox *b,mat4 m):bbox(b),mtx(m){}
	};
	std::vector<debugBBox_t> debugBBox;
};

