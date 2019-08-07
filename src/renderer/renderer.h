
#pragma once

#include "scene/Scene.h"
#include "renderer/camera.h"

using glm::mat4;

#define RENDERER_GUI 1
#define RENDERER_LIGHT 2
#define RENDERER_BACKBUFFER 4

class ResourceManager;
class Button;
struct submesh_t;
class glslProg;
class FrameBufferObject;

class IRenderer;
IRenderer *CreateRenderer();

class IRenderer
{
public:
	IRenderer():width(256),height(256),aspect(1),scene(0),camera(0){}
	virtual void Init(int initFlags,ResourceManager *resMan)=0;
	virtual void Resize(int w, int h)=0;
	virtual void Draw()=0;

	virtual void SetScene(Scene *sc){scene=sc;}
	virtual void SetCamera(Camera *cam){camera=cam;}
	virtual void SetColor(float r,float g,float b,float a)=0;
	virtual void SetModelMtx(const glm::mat4 &mtx)=0;
	virtual void Clear()=0;
	virtual void DrawModel(Model *mdl,const glm::mat4 &mtx)=0;
	virtual void Set2DMode(bool m = true)=0;
	virtual void ResetViewport()=0;
	virtual void DrawText(const char *t,float x,float y,float s)=0;
	virtual void DrawRect(float x, float y, float w, float h)=0;
	virtual void AddButton(Button *b)=0;
	virtual void SetBackBufferScale(float s)=0;
	virtual FrameBufferObject *GetFBO()=0;
	virtual void RenderDepth(Camera *cam)=0;
	virtual void DrawBBox(BoundingBox *bbox)=0;
	virtual void DrawCube()=0;
	
	virtual void UseProg(glslProg *p){}
	
	int flags;
	int width;
	int height;
	float aspect;
	Scene *scene;
	Camera *camera;

	glm::mat4 modelMtx;
	glm::mat4 vpMtx;
	glm::mat4 mvpMtx;

	std::vector<Button*> buttons;

	bool debug;
};

class ILighting{
public:
	virtual void Init(IRenderer *rend)=0;
	virtual void Draw()=0;
	virtual void SetDirectional()=0;
	virtual void SetCamera(Camera *cam)=0;
	virtual void SetScene(Scene *s)=0;
	virtual void AddLights(const mat4 &mtx, const submesh_t &submesh)=0;
};

ILighting *GetLighting();

