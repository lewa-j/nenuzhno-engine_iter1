
#include "Volumetrics.h"

#include "scene/Scene.h"
#include "renderer/renderer.h"

#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc/type_ptr.hpp>
using glm::vec4;
using glm::mat4;

Volumetrics::Volumetrics(){
	lightObj = 0;
}

void Volumetrics::Init(Scene *sc,vec3 pos,vec3 target,float fov, float aspect,float aNear,float aFar,int lightRes)
{
	lightObj = new LightObject(ePoint,pos,vec3(1),aFar);
	sc->AddLight(lightObj);

	camera = Camera();
	camera.LookAt(lightObj->pos,target,vec3(0,1,0));
	camera.UpdateProj(fov,aspect,aNear,aFar);
	camera.UpdateFrustum();
	depthFBO.Create();
	depthFBO.CreateDepthTexture(lightRes,lightRes);
	FrameBufferObject::Unbind();
}

void Volumetrics::AddToDepth(IRenderer *rend){
	mat4 mtx = scale(glm::inverse(camera.projMtx*camera.viewMtx),vec3(2));
	rend->SetModelMtx(mtx);
	rend->DrawCube();
}

void Volumetrics::Prepare(IRenderer *rend){
	depthFBO.Bind();
	depthFBO.SetupViewport();
	depthFBO.Clear(2);
	rend->RenderDepth(&camera);
	depthFBO.Unbind();
	rend->ResetViewport();
}

void Volumetrics::Draw(IRenderer *rend,int u_lightMtx,int u_invVPMtx, int u_lightPosSize){
	mat4 lightMtx = camera.projMtx*camera.viewMtx;
	//progVolLight.UniformMat4(u_lightMtx,lightMtx);
	glUniformMatrix4fv(u_lightMtx,1,false,glm::value_ptr(lightMtx));
	lightMtx = scale(glm::inverse(lightMtx),vec3(2));
	rend->SetModelMtx(lightMtx);
	//progVolLight.UniformMat4(u_invVPMtx,glm::inverse(rend->vpMtx));
	glUniformMatrix4fv(u_invVPMtx,1,false,glm::value_ptr(glm::inverse(rend->vpMtx)));
	glUniform4f(u_lightPosSize,lightObj->pos.x,lightObj->pos.y,lightObj->pos.z,lightObj->radius);
	glActiveTexture(GL_TEXTURE1);
	depthFBO.BindDepthTexture();
	glActiveTexture(GL_TEXTURE2);
	rend->GetFBO()->BindDepthTexture();
	glActiveTexture(GL_TEXTURE0);

	rend->DrawCube();
}

bool Volumetrics::CheckPoint(glm::vec3 pos){
	return camera.frustum.Contains(vec4(pos,0.02));
}