
#include <cstdlib>
#include <cstdio>
#include <vector>
using namespace std;
#include <mat4x4.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/random.hpp>

#include "log.h"
#include "engine.h"
#include "game/IGame.h"
#include "resource/ResourceManager.h"
#include "renderer/renderer.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/glsl_prog.h"
#include "renderer/mesh.h"
#include "graphics/texture.h"
#include "renderer/camera.h"
#include "renderer/font.h"
#include "renderer/Material.h"
#include "button.h"

#include "entity.h"

#include "star-fleet.h"

StarFleetGame *gGame=0;

IGame *CreateGame(){
	gGame = new StarFleetGame();
	return gGame;
}

eGameState gameState = eGameState_Menu;

glslProg starboxProg;

Texture starboxTex;

Model *modelShip;
Model *modelCube;
//Mesh *shipMesh;
Texture shipTex;
TexMaterial matLight(&shipTex,false);
#define SHIP_COUNT 10
Ship *ships[SHIP_COUNT];
vector<Entity *> entities;

//menu buttons
std::vector<Button*> buttons;
//start
Button bNewGame;
Button bQuit;
//game
Button bPause;
//pause
Button bResume;

void NewGame();
void Quit();
void Pause();
void Resume();

void UpdateButtons()
{
	bNewGame = Button(0.1,0.3,0.5,0.1,"New game");
	bQuit = Button(0.1,0.6,0.5,0.1,"Quit");
	bPause = Button(0.75,0.02,0.2,0.1,"[=] Pause");
	bResume = Button(0.1,0.3,0.5,0.1,"Resume");
	bNewGame.func=NewGame;
	bPause.func=Pause;
	bQuit.func=Quit;
	bResume.func=Resume;
}

void ChangeState(eGameState s){
	gameState = s;

	for(uint32_t i=0; i<buttons.size(); i++){
		buttons[i]->active = false;
	}
	
	switch(s){
	case eGameState_Menu:
		bNewGame.active = true;
		bQuit.active = true;
		break;
	case eGameState_Play:
		bPause.active = true;
		break;
	case eGameState_Pause:
		bQuit.active = true;
		bResume.active = true;
		break;
	}
}

void StartMenu()
{
	buttons.clear();
	buttons.push_back(&bNewGame);
	buttons.push_back(&bPause);
	buttons.push_back(&bQuit);
	buttons.push_back(&bResume);

	//TODO remove!
	gGame->renderer->AddButton(&bNewGame);
	gGame->renderer->AddButton(&bPause);
	gGame->renderer->AddButton(&bQuit);
	gGame->renderer->AddButton(&bResume);

	//UpdateButtons(); //in Changed()

	ChangeState(eGameState_Menu);
}

void NewGame()
{
	ChangeState(eGameState_Play);
	
	if(!entities.empty())
		EngineError("Game already started");
	srand(time(0)); 
	for(int i=0; i<SHIP_COUNT; i++)
	{
		Ship *ship = new Ship();
		ship->position = glm::vec3(glm::sin(i*0.85f)*5.0,0,-i*1.5f);
		ship->velocity = glm::vec3(0,0,2.0f);//glm::sphericalRand(0.2f);
		ship->angles = glm::sphericalRand(180.0f);
		ship->state = (eShipState)((i%2)+1);
		if(i%2)
			ship->target = ships[i-1];
		entities.push_back(ship);
		ships[i] = ship;
		gGame->scene->AddObject(ship);
	}
	ships[1]->gun = shipGun_t(16,20.0f,4,0.04f,0.04f);
	//ships[1]->target = entities[2];
	
	gGame->camera.pos = glm::vec3(0,1.6f,2);
	gGame->camera.rot = glm::vec3(30,0,0);
	gGame->camera.UpdateView();

	for(int i=0; i<1000; i++){
		Asteroid *obj = new Asteroid(glm::ballRand(250.0f),glm::linearRand(0.05f,5.0f));
		if(i<600){
			obj->velocity = glm::ballRand(20.0f);
			obj->ang = glm::linearRand(-40,40);
		}
		gGame->scene->AddObject(obj);
	}
}

void Quit(){
	exit(0);
}

void Pause(){
	ChangeState(eGameState_Pause);
}

void Resume(){
	ChangeState(eGameState_Play);
}

void StarFleetGame::Created()
{
	Log("Init nenuzhno engine. StarFleet\n");

	resMan = new ResourceManager();
	resMan->Init();
	renderer = CreateRenderer();
	renderer->Init(RENDERER_GUI|RENDERER_LIGHT,resMan);

	scene = new Scene();
	renderer->SetScene(scene);

	starboxProg.CreateFromFile("skybox","skybox");
	starboxProg.u_mvpMtx = starboxProg.GetUniformLoc("u_mvpMtx");
	glUseProgram(0);
	CheckGLError("Created shaders", __FILE__, __LINE__);

	GLubyte testTexData[] =
	{
		63,63,   255,255, 31,31,   255,255,
		63,63,   255,255, 31,31,   255,255,
		255,255, 63,63,   255,255, 31,31,
		255,255, 63,63,   255,255, 31,31,
		31,31,   255,255, 63,63,   255,255,
		31,31,   255,255, 63,63,   255,255,
		255,255, 31,31,   255,255, 63,63,
		255,255, 31,31,   255,255, 63,63
	};
	shipTex.Create(8,8);
	shipTex.SetFilter(GL_LINEAR, GL_LINEAR);
	shipTex.Upload(0, GL_LUMINANCE, testTexData);

	resMan->AddMaterial("light",&matLight);

	GLubyte *starboxData = new GLubyte[64*64];
	memset(starboxData,0,64*64);
	starboxData[75]=255;
	starboxData[130]=255;
	starboxData[406]=255;
	starboxData[625]=255;
	starboxData[1307]=255;
	starboxData[3506]=255;
	starboxData[4002]=255;
	starboxData[2383]=255;
	starboxTex.target = GL_TEXTURE_CUBE_MAP;
	starboxTex.Create(64,64);
	starboxTex.SetFilter(GL_LINEAR, GL_LINEAR);
	for(int i=0; i<6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_LUMINANCE, 64, 64, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, starboxData);
	}
	delete[] starboxData;
	//TODO: Move to StartGame?
	scene->skyBox = &starboxTex;

	CheckGLError("Created texture", __FILE__, __LINE__);
	
	//shipMesh = new MeshFBO_N3_T2(vertices, 4, GL_TRIANGLE_FAN);
	//shipMesh = LoadMeshFile("sship1", true);

	modelCube = resMan->GetModel("cube.nmf");
	modelShip = resMan->GetModel("sship1.nmf");
	CheckGLError("Created meshes", __FILE__, __LINE__);

	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	CheckGLError("Created", __FILE__, __LINE__);
	
	StartMenu();

	oldTime=GetTime();
}

float aspect;//for input
void StarFleetGame::Changed(int w, int h)
{
	renderer->Resize(w,h);
	aspect = renderer->aspect;
	camera.UpdateProj(85.0f, renderer->aspect, 0.1f, 1500.0f);
	renderer->SetCamera(&camera);
	UpdateButtons();
	ChangeState(gameState);
}

void StarFleetGame::Update()
{
	double startTime = GetTime();
	deltaTime = float(startTime-oldTime);
	oldTime = startTime;

	for(uint32_t i=0; i<buttons.size();i++){
		buttons[i]->Update();
	}

	if(gameState==eGameState_Play){
		scene->Update(deltaTime);

	//for(vector<Entity *>::iterator it=entities.begin();it<entities.end();it++)
	for(uint32_t i=0;i<entities.size();i++){
		Entity *ent = entities[i];
		//ent->Update(deltaTime);
		if(ent->remove){
			entities.erase(entities.begin()+i);
			gGame->scene->RemoveObject(ent);
			delete ent;
			i--;
		}
	}
	camera.viewMtx = glm::lookAt(
		//glm::vec3(3.2f,0,0),
		entities[1]->position+glm::mat3(entities[1]->modelMtx)*glm::vec3(0.5,1.3,-1.6)+glm::vec3(0,0.2,0),
		entities[1]->position+glm::mat3(entities[1]->modelMtx)*glm::vec3(0.4,1.0,0.8),
		glm::vec3(0,1,0));

//	camera.UpdateView();
	}
}

void Ship::Draw(IRenderer *r){
	r->DrawModel(modelShip, displayMtx);
}

void Projectile::Draw(IRenderer *r){
	r->SetColor(1,0,0,1);
	r->DrawModel(modelCube, displayMtx);
}

void Explosion::Draw(IRenderer *r){
	r->SetColor(1,0.5,0,1);
	r->DrawModel(modelCube, displayMtx);
}

void Asteroid::Draw(IRenderer *r){
	r->SetColor(0.7,0.7,0.7,1);
	r->DrawModel(modelCube, displayMtx);
}

/*
void DrawText(const char *t,float x,float y,float s)
{
	//glEnable(GL_BLEND);
	//glBlendFunc(1,1);
	testFont.Print(t,x,y/renderer->aspect,s);
	//glDisable(GL_BLEND);
}*/
/*
void DrawRect(float x, float y, float w, float h)
{
	float lx=x;
	float rx=(x+w);
	float dy=(y+h);
	float verts[] = {
		lx,	y, 0,0,
		lx,	dy,	 0,1,
		rx,	dy,	 1,1,
		rx,	y, 1,0
	};
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, verts);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 16, verts+2);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
*/
/*
void StarFleetGame::DrawButton(Button *b)
{
	if(b->text){
		//testFont.Print(b->text,b->x*aspect+0.01,1-b->y-0.05,0.5);
		//float x = b->x*2.0f-1.0f;
		//float y = 1-(b->y*2.0f-1.0f);
		renderer->DrawText(b->text,b->x+0.01,b->y-0.05f,0.5);
	}//else
	{
		//DrawRect(b->x,b->y,b->w,b->h);
	}
}*/

void StarFleetGame::Draw()
{
	Update();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	CheckGLError("Clear", __FILE__, __LINE__);

	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	renderer->Draw();
#if 0
	glEnableVertexAttribArray(2);
	texProg.Use();
	glUniform4f(texProg.u_color,1,1,1,1);
	if(gameState==eGameState_Play||gameState==eGameState_Pause){
	shipMesh->Bind();
	shipTex.Bind();
	for(int i=0;i<SHIP_COUNT;i++)
	{
		if(((Ship*)entities[i])->isDead)
			continue;
		mvpMtx = camera.projMtx * camera.viewMtx * entities[i]->displayMtx;
		glUniformMatrix4fv(texProg.u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));
		shipMesh->Draw();
	}
	cubeMesh->Bind();
	//proj
	texWhite.Bind();
	glUniform4f(texProg.u_color,1,0,0,1);
	for(uint32_t i=SHIP_COUNT;i<entities.size();i++){
		if(entities[i]->type!=eProj)
			continue;
		mvpMtx = camera.projMtx * camera.viewMtx * entities[i]->displayMtx;
		glUniformMatrix4fv(texProg.u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));
		cubeMesh->Draw();
	}
	//expl
	glUniform4f(texProg.u_color,1,0.5f,0,1);
	for(uint32_t i=SHIP_COUNT;i<entities.size();i++){
		if(entities[i]->type!=eExpl)
			continue;
		mvpMtx = camera.projMtx * camera.viewMtx * entities[i]->displayMtx;
		glUniformMatrix4fv(texProg.u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));
		cubeMesh->Draw();
	}
	cubeMesh->Unbind();
	glUniform4f(texProg.u_color,1,1,1,1);
	}
#endif
	renderer->SetColor(1,1,1,1);
	//2D
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//glm::mat4 mvpMtx = glm::scale(glm::mat4(1.0),glm::vec3(1.0f,renderer->aspect,1.0f));
	//glUniformMatrix4fv(texProg.u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));
	if(gameState==eGameState_Play){
		//glUniform4f(texProg.u_color,1,1,1,1);
		renderer->Set2DMode();
		char str[64];
		snprintf(str,64,"hp: %.0f",ships[1]->hp);
		renderer->DrawText(str,0,0.1,0.48);

		snprintf(str,64,"ents: %d",entities.size());
		renderer->DrawText(str,0.4,0.1,0.48);
		
		renderer->SetColor(0,1,0,1);
		for(int i=0;i<SHIP_COUNT;i++){
			if(((Ship*)entities[i])->isDead)
				continue;
			glm::mat4 tmtx = camera.projMtx * camera.viewMtx * entities[i]->modelMtx;
			glm::vec4 p(0,0,0,1);
			p=tmtx*p;
			p/=p.w;
			p.y = (-p.y/renderer->aspect*0.5+0.5);
			p.x = p.x*0.5+0.5;
			if(p.z>0&&p.z<1){
				//DrawRect(p.x-0.15,p.y+0.2,0.3*ships[i]->hp*0.01,0.02);
				renderer->DrawRect(p.x-0.15,p.y-0.2,0.3*ships[i]->hp*0.01,0.02);
			}
		}
	}
	renderer->SetColor(1,1,1,1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(2);

	CheckGLError("Draw", __FILE__, __LINE__);
}

void StarFleetGame::OnTouch(float x, float y, int a, int tf)
{
	float tx = x/renderer->width;
	float ty = y/renderer->height;
	if(a==0){
		for(uint32_t i=0; i<buttons.size();i++){
			if(buttons[i]->active)
				buttons[i]->Hit(tx,ty);
		}
	}
}

bool Button::SetUniform(int loc)
{
	glUniform4f(loc, x*2.0f-(1-w), -(y*2.0f-(1-h)), w, h);
	return true;
}

class Projectile;
void TraceLine(Projectile *proj, float deltaTime);

void CreateExplosion(Entity *owner,float size){
	Explosion *expl = new Explosion(owner->position,size);
	expl->angles = owner->angles;
	entities.push_back(expl);
	gGame->scene->AddObject(expl);
}

void CreateProjectile(Entity *shooter, glm::vec3 dir,float damage)
{
	Projectile *proj = new Projectile(shooter,damage);
	proj->position = shooter->position;
	proj->angles = shooter->angles;
	//proj->velocity = glm::normalize(shooter->velocity)*2.0f;
	proj->velocity = dir;
	
	entities.push_back(proj);
	gGame->scene->AddObject(proj);
	//Log("CreateProjectile entities.size() %d\n",entities.size());
}

void TraceLine(Projectile *proj, float deltaTime)
{
	for(int i=0;i<SHIP_COUNT;i++){
		Ship *ent = (Ship *)entities[i];
		if(ent->isDead||ent==proj->shooter)
			continue;
		
		glm::mat4 invmodelMtx = glm::inverse(ent->modelMtx);
		glm::vec3 s = glm::vec3(invmodelMtx*(glm::vec4(proj->position,1.0f)));
		glm::vec3 d = glm::vec3(invmodelMtx*(glm::vec4(proj->velocity*deltaTime,1.0f)));//proj->modelMtx[2];
#if 0
		glm::vec3 p = glm::vec3(0.0f);
		glm::vec3 e = glm::vec3(0.5f);
		float hl  = glm::length(d)*0.5f;
		glm::vec3 m = s+d*0.5f;
		glm::vec3 t = p-m;
		
		if( (glm::abs(t.x)>e.x+hl+glm::abs(d.x)) ||
			(glm::abs(t.y)>e.y+hl+glm::abs(d.y)) ||
			(glm::abs(t.z)>e.z+hl+glm::abs(d.x)) )
			continue;
		
		float r = e.y*glm::abs(d.z)+e.z*glm::abs(d.y);
		if(glm::abs(t.y*d.z-t.z*d.y)>r)
			continue;
		
		r = e.x*glm::abs(d.z)+e.z*glm::abs(d.x);
		if(glm::abs(t.z*d.x-t.x*d.z)>r)
			continue;
		
		r = e.x*glm::abs(d.y)+e.y*glm::abs(d.x);
		if(glm::abs(t.x*d.y-t.y*d.x)>r)
			continue;
#endif
		if(!ent->bbox.Intersect(s,d))
			continue;
		
		//Log("Ship %p hit %p\n",proj->shooter,ent);
		CreateExplosion(proj,0.2f);
		proj->remove = true;
		ent->Hit(proj->damage);
		return;
	}
}

