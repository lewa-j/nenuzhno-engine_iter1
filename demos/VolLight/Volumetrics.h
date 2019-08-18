
#pragma once

class Scene;
class IRenderer;
class LightObject;
#include "renderer/camera.h"
#include "graphics/fbo.h"

#include <vec3.hpp>
using glm::vec3;

class Volumetrics
{
public:
	Volumetrics();
	void Init(Scene *sc,vec3 pos,vec3 target,float fov, float aspect,float aNear,float aFar,int lightRes);
	void AddToDepth(IRenderer *rend);
	void Prepare(IRenderer *rend);
	void Draw(IRenderer *rend,int u_lightMtx,int u_invVPMtx, int u_lightPosSize);
	bool CheckPoint(vec3 pos);
	LightObject *lightObj;
	Camera camera;
	FrameBufferObject depthFBO;
};
