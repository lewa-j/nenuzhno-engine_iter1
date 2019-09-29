
#include <vector>

#include "graphics/platform_gl.h"
#include "log.h"
#include "engine.h"
#include "button.h"
#include "game/IGame.h"
#include "system/config.h"
#include "resource/ResourceManager.h"
#include "renderer/renderer.h"
#include "renderer/mesh.h"
#include "renderer/material.h"
#include "renderer/model.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/glsl_prog.h"

#include "Volumetrics.h"

#include <vec2.hpp>
#include <gtc/random.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <common.hpp>
#include <geometric.hpp>
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::linearRand;
using glm::clamp;
using glm::abs;
using glm::normalize;
using glm::translate;
using glm::scale;

class VolLightGame: public IGame
{
public:
	VolLightGame(){}
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "vol_light";
	}
	void OnTouch(float tx, float ty, int ta, int tf);
	void OnKey(int key, int scancode, int action, int mods);

	IRenderer *rend;
	ResourceManager *resMan;
	ConfigFile cfg;
	Camera cam;
	Scene scene;

	Texture texWhite;
	TexMaterial *matWhite;

	Volumetrics lightVol;
	FrameBufferObject volFBO;
	float volScale;
	glslProg progVolLight;
	int u_lightMtx;
	int u_invVPMtx;
	int u_lightPosSize;
	glslProg progVolLightIn;
	int u_lightMtxIn;
	int u_invVPMtxIn;
	int u_lightPosSizeIn;

	Joystick joyL;
#ifdef ANDROID
	Joystick joyM;
#else
	KeyJoystick joyM;
#endif
	int frame;
	
	double oldTime;
};

IGame *CreateGame(){
	return new VolLightGame();
}

void VolLightGame::Created()
{
	Log("VolLight test Created()\n");
	resMan = new ResourceManager();
	resMan->Init();
	rend = CreateRenderer();
	rend->Init(RENDERER_GUI|RENDERER_LIGHT|RENDERER_BACKBUFFER, resMan);
	
	uint8_t whiteData[]={255,255,255};
	texWhite.Create(1,1);
	texWhite.Upload(0,GL_RGB,whiteData);
	matWhite = new TexMaterial(&texWhite,true);
	
	progVolLight.CreateFromFile("volLight","volLight");
	progVolLight.u_mvpMtx = progVolLight.GetUniformLoc("u_mvpMtx");
	progVolLight.u_modelMtx = progVolLight.GetUniformLoc("u_modelMtx");
	progVolLight.u_cameraPos = progVolLight.GetUniformLoc("u_cameraPos");
	u_lightMtx = progVolLight.GetUniformLoc("u_lightMtx");
	u_invVPMtx = progVolLight.GetUniformLoc("u_invVPMtx");
	u_lightPosSize = progVolLight.GetUniformLoc("u_lightPosSize");
	progVolLight.UniformTex("u_lightDepth",1);
	progVolLight.UniformTex("u_sceneDepth",2);
	
	progVolLightIn.CreateFromFile("volLight","volLightIn");
	progVolLightIn.u_mvpMtx = progVolLightIn.GetUniformLoc("u_mvpMtx");
	progVolLightIn.u_modelMtx = progVolLightIn.GetUniformLoc("u_modelMtx");
	progVolLightIn.u_cameraPos = progVolLightIn.GetUniformLoc("u_cameraPos");
	u_lightMtxIn = progVolLightIn.GetUniformLoc("u_lightMtx");
	u_invVPMtxIn = progVolLightIn.GetUniformLoc("u_invVPMtx");
	u_lightPosSizeIn = progVolLightIn.GetUniformLoc("u_lightPosSize");
	progVolLightIn.UniformTex("u_lightDepth",1);
	progVolLightIn.UniformTex("u_sceneDepth",2);

	cfg.Load("config.txt");

	Model *mdl = resMan->GetModel(cfg["modelName"].c_str());
	mdl->materials[0].mat = matWhite;
	
	scene = Scene();

	StaticModel *obj1 = new StaticModel();
	obj1->mdl = mdl;
	vec3 obj1Pos = cfg.GetVec3("modelPos");
	obj1->modelMtx = translate(mat4(1.0f),obj1Pos);
	scene.AddObject(obj1);
	
	lightVol.Init(&scene,cfg.GetVec3("lightPos"),cfg.GetVec3("lightTarget"),atof(cfg["lightFOV"].c_str()),
			atof(cfg["lightAspect"].c_str()),atof(cfg["lightNear"].c_str()),atof(cfg["lightFar"].c_str()),cfg.GetInt("lightRes"));

	volScale = atof(cfg["volScale"].c_str());
	volFBO.Create();
	volFBO.CreateTexture(64,64,GL_LINEAR);
	FrameBufferObject::Unbind();

	rend->SetScene(&scene);
	
	cam = Camera();
	cam.pos = cfg.GetVec3("cameraPos");
	cam.rot = cfg.GetVec3("cameraRot");
	cam.UpdateView();
	
	rend->SetCamera(&cam);

	rend->debug = cfg["debug"]!="0";

	joyL = Joystick(0,0.5,0.5,0.5);
#ifdef ANDROID
	joyM = Joystick(0.5,0.5,0.5,0.5);
#else
	joyM = KeyJoystick(IN_KEY_W,IN_KEY_S,IN_KEY_D,IN_KEY_A);
#endif
	frame = 0;
	oldTime = GetTime();
}

void VolLightGame::Changed(int w, int h)
{
	if(!w||!h)
		return;

	rend->Resize(w,h);
	
	cam.UpdateProj(80.0f,rend->aspect,0.02f,20.0f);
	//cam.SetOrtho(aspect,bScroll.pos);

	volFBO.Resize(w*volScale,h*volScale);
	frame = 0;
}

void VolLightGame::Draw()
{

	double startTime = GetTime();
	float deltaTime = (startTime-oldTime);
	oldTime = startTime;

	cam.pos += glm::inverse(glm::mat3(cam.viewMtx))*vec3(joyM.vel.x,0,-joyM.vel.y)*2.0f*deltaTime;
	cam.rot += vec3(-joyL.vel.y*rend->aspect,-joyL.vel.x,0)*60.0f*deltaTime;
	cam.UpdateView();

	rend->Draw();

	bool drawVolumetricLight = true;
	bool isInsideVolume = lightVol.CheckPoint(cam.pos);
	//drawVolumetricLight = !isInsideVolume;

	//Add light box to depth
	if(drawVolumetricLight){
		rend->GetFBO()->Bind();
		rend->Set2DMode(false);
		glColorMask(0,0,0,0);
		//UseProg depth
		lightVol.AddToDepth(rend);
		glColorMask(1,1,1,1);
		rend->GetFBO()->Unbind();
	}
#if 0
	//display scene depth
	rend->Set2DMode(true);
	rend->SetModelMtx(mat4(1));
	rend->GetFBO()->BindDepthTexture();
	rend->DrawRect(0.02,0.02,0.5,0.5);
#else
	//prepare depth texture
	if(drawVolumetricLight){
		if(frame%128==0)
			lightVol.Prepare(rend);

		//Display light depth
		/*lightFBO.BindDepthTexture();
		rend->SetModelMtx(mat4(1));
		rend->Set2DMode();
		rend->DrawRect(0.02,0.02,0.5,0.5);*/

		volFBO.Bind();
		volFBO.Clear(1);
		rend->Set2DMode(false);
		if(!isInsideVolume){
			glCullFace(GL_FRONT);
			rend->UseProg(&progVolLight);
			lightVol.Draw(rend,u_lightMtx,u_invVPMtx,u_lightPosSize);
			glCullFace(GL_BACK);
		}else{
			rend->UseProg(&progVolLightIn);
			lightVol.Draw(rend,u_lightMtxIn,u_invVPMtxIn,u_lightPosSizeIn);
		}

		//clean up
		glActiveTexture(GL_TEXTURE1);
		texWhite.Bind();
		glActiveTexture(GL_TEXTURE2);
		texWhite.Bind();
		glActiveTexture(GL_TEXTURE0);

		volFBO.Unbind();
		rend->ResetViewport();
		//draw effect additive
		rend->Set2DMode(true);
		glEnable(GL_BLEND);
		glBlendFunc(1,1);
		volFBO.BindTexture();
		rend->DrawRect(0.0,0.0,1.0,1.0);
		glDisable(GL_BLEND);
	}
	texWhite.Bind();
#endif
	char temp[256];
	snprintf(temp,256,"volumetric light: %s",drawVolumetricLight?"On":"Off");
	rend->DrawText(temp,0.02,0.9,0.2*rend->aspect);

	frame++;
}

Button::Button(float nx, float ny, float nw, float nh, bool adjust):
			x(nx),y(ny),w(nw),h(nh),text(0),active(true),pressed(false)
{
}

Button::Button(float nx, float ny, float nw, float nh, const char *t, bool adjust):
			x(nx),y(ny),w(nw),h(nh),text(t),active(true),pressed(false)
{
}
void Button::Update(){}
bool Button::SetUniform(int loc){return false;}

void VolLightGame::OnTouch(float tx, float ty, int ta, int tf)
{
	float x = tx/rend->width;
#ifdef ANDROID
	float y = (ty-64)/rend->height;
#else
	float y = ty/rend->height;
#endif

	if(ta==0){
		joyL.Hit(x,y,tf);
		joyM.Hit(x,y*rend->aspect,tf);
	}else if(ta==1){
		joyL.Release(tf);
		joyM.Release(tf);
	}else if(ta==2){
		joyL.Move(x,y,tf);
		joyM.Move(x,y*rend->aspect,tf);
	}
}

void VolLightGame::OnKey(int key, int scancode, int action, int mods)
{
#ifndef ANDROID
	joyM.OnKey(key,action);
#endif
}

