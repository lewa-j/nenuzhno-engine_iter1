
#include "log.h"
#include "engine.h"
#include "game/IGame.h"
#include "graphics/gl_utils.h"
#include "graphics/texture.h"
#include "renderer/renderer.h"
#include "renderer/Model.h"
#include "renderer/camera.h"
#include "resource/ResourceManager.h"
#include "system/FileSystem.h"
#include "button.h"
#include "system/config.h"

#include "tpsMaterials.h"
#include "tpsPlayer.h"
#include "tpsPhysics.h"
#include "tpsObjects.h"
#include "tpsMenu.h"

using namespace std;

using glm::vec2;
using glm::vec3;
using glm::mat4;
using glm::translate;
using glm::scale;
using glm::dot;
using glm::normalize;
using glm::mix;
using glm::radians;

Scene *LoadTXTScene(const char *fileName,ResourceManager *resMan,PhysicsSystem *physics);
Texture *GenerateStarBox(int size, int count);

std::string strGameLog;

glm::vec3 atovec3(const char *str)
{
	glm::vec3 out(0);
	sscanf(str, "%f %f %f", &out.x, &out.y, &out.z);
	return out;
}

void GameLog(const char *s){
	Log("Game: %s",s);
	strGameLog+=s;
}

//TODO interacting objects
std::vector<ButtonObject*> g_buttons;
std::vector<Projectile*> g_projs;

enum eGameState{
	eGameMenu=1,
	eGamePlay=2,
	eGamePause=3
};

class tpsGame:public IGame
{
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "tpsGame";
	}
	void OnKey(int key, int scancode, int action, int mods);
	void OnTouch(float tx, float ty, int ta, int tf);
	void OnMouseMove(float x, float y);

	void Update(float dt);
	void InitTouch();
	void UpdateMouse(float deltaTime);
	void StartGame();
	void ResumeGame();
	void PauseGame();
	
	eGameState gameState;
	
	ResourceManager *resMan;
	IRenderer *renderer;
	PhysicsSystem *physics;
	Scene *scene;
	Texture *starBox;
	Camera cam;

	double oldTime;
	Player *player;
	Joystick joyM;
	Joystick joyL;
	KeyJoystick keyM;
	vec2 mousePos;
	vec2 lastMousePos;
	Button bJump;
	Button bUse;
	Button bAttack;
	Button bPause;
	
	StaticModel *box1;
	//ButtonObject *button1;
	//DoorObject *door1;
	ButtonObject *curButt;
	LightObject *lamp1;
	
	Weapon pistol;
	vec3 pistOffs;
	
	tpsMenu menu;
	tpsSettings settings;
	float lastScreenScale;
	std::string startScene;

	friend void tpsStartGame();
	friend void tpsResumeGame();
	friend void ShootBullet(vec3 orig, vec3 dir);
};

tpsGame *g_game;

IGame *CreateGame()
{
	return (g_game=new tpsGame());
}

Model *g_mdlBox=0;

void ShootBullet(vec3 orig, vec3 dir)
{
	Projectile *p = new Projectile(orig,dir);
	
	g_projs.push_back(p);
	g_game->scene->AddObject(p);
}

void tpsGame::InitTouch()
{
	joyM = Joystick(0,0.3,0.5,0.7);
	joyL = Joystick(0.5,0,0.5,1);
	bJump = Button(0.7,0.5,0.2,0.2);
	bUse = Button(0.15,0.25,0.2,0.2);
	bAttack = Button(0.75,0.25,0.2,0.2,true);
	bPause = Button(0.9,0.1,0.1,0.1);
}

void tpsStartGame()
{
	g_game->StartGame();
}

void tpsResumeGame()
{
	g_game->ResumeGame();
}

void tpsGame::ResumeGame()
{
	gameState = eGamePlay;
	
	//InitTouch();
	renderer->SetScene(scene);
	EnableCursor(false);
}

void tpsGame::PauseGame()
{
	gameState = eGamePause;
	renderer->SetScene(0);
	EnableCursor(true);
}

void tpsGame::StartGame()
{
	gameState = eGamePlay;
	
	Model *mdlBox = resMan->GetModel("cube.nmf");
	g_mdlBox = mdlBox;

	scene = LoadTXTScene(startScene.c_str(),resMan,physics);
	starBox = GenerateStarBox(256,128);
	scene->skyBox = starBox;

	InitTouch();

	Model *mdlChar = resMan->GetModel("char1.nmf");
	player = new Player(&cam,scene->startPos,scene->startRot,mdlChar);
#ifdef ANDROID
	player->move = &joyM;
#else
	keyM = KeyJoystick(IN_KEY_W,IN_KEY_S,IN_KEY_D,IN_KEY_A);
	player->move = &keyM;
#endif
	player->look = &joyL;
	player->jump = &bJump;
	player->bAttack = &bAttack;
	player->phys = physics->CreatePlayer(1,0.4,player->modelMtx);
	scene->AddObject(player);
	
	box1 = new StaticModel();
	box1->mdl = mdlBox;
	box1->modelMtx = translate(mat4(1.0f),vec3(1.0f,6.0f,-3.0f));
	physics->AddBox(vec3(1),box1,100);
	scene->AddObject(box1);

/*
	door1 = new DoorObject(vec3(-3.5,1,3.5),mdlBox);
	scene->AddObject(door1);

	button1 = new ButtonObject(vec3(-2,1.5f,3.0f));
	button1->mdl = mdlBox;
	button1->target = door1;
	button1->name = "Button1";
	button1->text = "Open door";
	scene->AddObject(button1);
*/
	//lamp1 = new LightObject(ePoint,vec3(-20.4,1.6,3.5),vec3(0.8,0.8,1),5);
	//scene->AddLight(lamp1);

	Model *pistolMdl = resMan->GetModel("pistol1.nmf");

	pistol = Weapon(eWeapPistol,pistolMdl,5,2);
	pistol.offsetMtx = rotate(translate(mat4(1),pistOffs),radians(90.0f),vec3(0,1,0));
	player->weapon = &pistol;

	curButt = 0;

	renderer->SetScene(scene);
	EnableCursor(false);
}

void tpsGame::Created()
{
	gameState = eGameMenu;
	resMan = new ResourceManager();
	resMan->Init();
	resMan->AddMaterialLoader(new TXTMaterialLoader(resMan));

	renderer = CreateRenderer();
	renderer->Init(RENDERER_GUI|RENDERER_LIGHT|RENDERER_BACKBUFFER,resMan);

	ConfigFile config;
	config.Load("tpsConfig.txt");
	settings.Load(config);
	/*for(uint32_t i=0;i<config.values.size();i++){
		map<string,string>::iterator it = config.values.begin()+i;
		Log("cfg %d: %s=%s\n",i,it.first->c_str(),it.second->c_str());
	}*/
	renderer->debug = settings.drawBbox;
	lastScreenScale=settings.screenScale;
	renderer->SetBackBufferScale(settings.screenScale);
	startScene = config.values["startScene"];
	pistOffs = atovec3(config.values["pistOffs"].c_str());

	physics = new PhysicsSystem();
	physics->Init(settings.debugPhysics?renderer:0);
	
	menu.Init(&settings,renderer);
	//StartGame();

	mousePos = vec2(0);
	lastMousePos = vec2(0);

	Log("tpsGame Init %f\n",GetTime());
	oldTime = GetTime();
}

void tpsGame::Changed(int w, int h)
{
	renderer->Resize(w,h);
	cam.UpdateProj(80.0f,renderer->aspect,0.05f,100.0f);
	renderer->SetCamera(&cam);
	menu.Resize(renderer->aspect);
	InitTouch();
}

void tpsGame::Update(float deltaTime){
	menu.Update(deltaTime);
	if(gameState==eGamePlay){
		//TODO fixed update
#ifndef ANDROID
		float mouseSens=0.2f;
		joyL.vel = (mousePos-lastMousePos)/vec2(renderer->width,renderer->height)*mouseSens;
		lastMousePos = mousePos;
#endif
		if(lastScreenScale!=settings.screenScale){
			lastScreenScale=settings.screenScale;
			renderer->SetBackBufferScale(settings.screenScale);
		}

		player->tps = settings.tps;

		physics->Update(deltaTime);
		scene->Update(deltaTime);

		if(bPause.pressed){
			bPause.pressed = false;
			PauseGame();
		}

		curButt = 0;
		float curButtDist=99;
		for(uint32_t i=0; i<g_buttons.size();i++){
			g_buttons[i]->rot=player->rot.y;
			if(glm::distance(g_buttons[i]->pos,player->cam->pos)>2.0f)
				continue;
			float buttDist = dot(normalize(g_buttons[i]->pos-player->cam->pos),player->view);
			if(buttDist>0.9){
				if(buttDist<curButtDist){
					curButt = g_buttons[i];
					curButtDist = buttDist;
				}
			}
		}

		if(bUse.pressed){
			bUse.pressed = false;
			if(curButt)
				curButt->Press(player);
		}
	}
}

void tpsGame::Draw(){
	double startTime = GetTime();
	float deltaTime = (startTime-oldTime);
	oldTime = startTime;
	
	Update(deltaTime);

	renderer->Draw();

	if(gameState==eGamePlay){
		//if(player->phys->onGround())
		//	renderer->DrawText("on ground",0.02,0.1,0.4);

		if(curButt){
			renderer->DrawText(curButt->text.c_str(),0.4*renderer->aspect,0.9,0.5);
		}

		//touch
		if(settings.drawTouch){
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			renderer->SetColor(1,1,1,0.2);
			Button *b = &bJump;
			renderer->DrawRect(b->x,b->y,b->w,b->h);
			if(curButt){
				b = &bUse;
				renderer->DrawRect(b->x,b->y,b->w,b->h);
			}
			if(player->weapon){
				b = &bAttack;
				renderer->DrawRect(b->x,b->y,b->w,b->h);
			}
			b = &bPause;
			renderer->DrawRect(b->x,b->y,b->w,b->h);
			glDisable(GL_BLEND);
			renderer->SetColor(1,1,1,1);
		}

		if(settings.debugPhysics){
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			physics->world->debugDrawWorld();
			renderer->SetColor(1,1,1,1);
			//glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
		}
	}else{
		menu.Draw(renderer);
	}
	renderer->Set2DMode();
	//renderer->SetColor(1,1,1,1);
	renderer->DrawText(strGameLog.c_str(),0,0.95,0.2);

	CheckGLError("tpsGame::Draw", __FILE__, __LINE__);
}

Texture *GenerateStarBox(int size, int count){
	GLubyte *starboxData = new GLubyte[size*size];
	memset(starboxData,0,size*size);
	/*starboxData[75]=255;
	starboxData[130]=255;
	starboxData[406]=255;
	starboxData[625]=255;
	starboxData[1307]=255;
	starboxData[3506]=255;
	starboxData[4002]=255;
	starboxData[2383]=255;*/
	for(int i=0;i<count;i++){
		starboxData[glm::clamp((rand()*2)%(size*size),0,(size*size)-1)] = 255;
	}
	
	Texture *tex = new Texture();
	tex->target = GL_TEXTURE_CUBE_MAP;
	tex->Create(size,size);
	tex->SetFilter(GL_LINEAR, GL_LINEAR);
	for(int i=0; i<6; i++){
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_LUMINANCE, size,size, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, starboxData);
	}
	delete[] starboxData;
	
	return tex;
}

Button::Button(float nx, float ny, float nw, float nh,bool adj):
	x(nx),y(ny),w(nw),h(nh),type(0),text(0),active(true),pressed(false){
}
Button::Button(float nx, float ny, float nw, float nh, const char *t,bool adj):
	x(nx),y(ny),w(nw),h(nh),type(0),text(t),active(true),pressed(false){}
bool Button::SetUniform(int loc){return false;}
void Button::Update(){}

void tpsGame::OnTouch(float x, float y, int ta, int tf){
	float tx = x/renderer->width;
#ifdef ANDROID
	float ty = (y-64)/renderer->height;
#else
	float ty = y/renderer->height;
#endif
	if(gameState==eGameMenu||gameState==eGamePause){
		menu.OnTouch(tx,ty,ta,tf);
	}
#ifdef ANDROID
	else if(gameState==eGamePlay){
		if(ta==IN_PRESS){
			joyM.Hit(tx,ty,tf);
			joyL.Hit(tx,ty,tf);
			bJump.Hit(tx,ty);
			bUse.Hit(tx,ty);
			bAttack.Hit(tx,ty);
			bPause.Hit(tx,ty);
			for(uint32_t i=0; i<renderer->buttons.size();i++){
				if(renderer->buttons[i]->active)
					renderer->buttons[i]->Hit(tx,ty);
			}
		}else if(ta==IN_RELEASE){
			joyM.Release(tf);
			joyL.Release(tf);
			bAttack.pressed = false;
		}else if(ta==IN_MOVE){
			joyM.Move(tx,ty,tf);
			joyL.Move(tx,ty,tf);
			joyL.vel.y*=-1;
		}else{
			Log("touch %d %d\n",ta, tf);
		}
	}
#endif
}

void tpsGame::OnMouseMove(float x, float y)
{
	mousePos = vec2(x,y);
}

#ifndef ANDROID
#include <GLFW/glfw3.h>
void tpsGame::OnKey(int key, int scancode, int action, int mods)
{
	if(gameState==eGamePlay){
		keyM.OnKey(key,action);

		if(action==GLFW_PRESS){
			if(key==GLFW_KEY_ESCAPE){
				PauseGame();
			}
			if(key==GLFW_KEY_SPACE){
				bJump.pressed = true;
			}
			if(key==GLFW_KEY_E){
				bUse.pressed = true;
			}
		}else if(action==GLFW_RELEASE){
		}
	}
}
#else
void tpsGame::OnKey(int key, int scancode, int action, int mods)
{
}
#endif

