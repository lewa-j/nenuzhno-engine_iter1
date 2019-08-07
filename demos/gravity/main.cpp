
#include "log.h"
#include "graphics/platform_gl.h"
#include "button.h"
#include "renderer/renderer.h"
#include "renderer/mesh.h"
#include "resource/ResourceManager.h"

#include "game/IGame.h"

#include <vector>
#include <vec2.hpp>
#include <gtc/random.hpp>
#include <gtc/matrix_transform.hpp>
#include <common.hpp>
#include <geometric.hpp>
using glm::vec2;
using glm::vec3;
using glm::linearRand;
using glm::clamp;
using glm::abs;
using glm::normalize;
using glm::translate;
using glm::scale;

class vec3_queue{
public:
	vec3_queue():_data(){}
	void push(vec3 v){
		_data.push_back(v);
		for(int i=_data.size()-2;i>=0;i--){
			_data[i+1]=_data[i];
		}
		_data[0] = v;
	}
	vec3 pop(){
		if(!_data.size())
			return vec3(0);
		vec3 v = _data[_data.size()-1];
		_data.erase(_data.end()-1);
		return v;
	}
	vec3& operator[] (int n){
		return _data[n];
	}
	void *data(){
		return _data.data();
	}
	int size(){
		return _data.size();
	}
	void clear(){
		_data.clear();
	}
private:
	std::vector<vec3> _data;
};

class Entity
{
public:
	virtual ~Entity(){}
	virtual void Update(float dt){}
	virtual void UpdateTail(){}
	virtual void Respawn(){}

	vec3 pos;
	vec3 vel;
	vec2 size;
	float mass;
	int hp;

	vec3_queue trail;
};

class Unit: public Entity
{
public:
	Unit();
	virtual ~Unit(){}
	virtual void Update(float dt);
	virtual void UpdateTail();
	virtual void Respawn();
	vec3 target;
};

Unit::Unit():Entity(),target(0){
	vel = vec3(0);
	size = vec2(0.02f);
	mass = 0.5f;
	hp = 100;
};

Entity* Collide(Unit *u,vec3 orig, vec3 vel);
vec3 GetGravity(Unit *u);

vec3 planetPos(0.0f,0.0f,0.0f);
const int asteroidsCount = 2048;
float planetRadius = 1;
float planetMass = 2000;
float G = 0.000066;
const float timeScale = 10;

float aspect=1;

float RadiusFromMass(float m)
{
	const float p = 5515.3;
	float V = m/p;
	return glm::pow((3*V)/(4* M_PI ),1.0/3.0);
	//return glm::pow(m,0.5f)*0.02f;
}

void Unit::Update(float dt)
{
	if(hp<=0)
		return;

	float d=glm::distance(pos,planetPos);
	if(d<planetRadius+size.x||d>59)
	{
		planetMass += mass;
		planetRadius = RadiusFromMass(planetMass);
		Respawn();
	}
	/*
	if(pos.x<0||pos.x>aspect-size.x){
		vel.x*=-1;
		//vel+=linearRand(vec3(-0.02,-0.06,0),vec3(0.02,0.06,0));
		pos.x=clamp(pos.x,0.0f,aspect-size.x);
	}
	if(pos.y<0||pos.y>1-size.y){
		vel.y*=-1;
		//vel+=linearRand(vec3(-0.06,-0.02,0),vec3(0.06,0.02,0));
		pos.y=clamp(pos.y,0.0f,1-size.y);
	}
	*/
	//vel+=vec3(0,0.5f*dt,0);
	vec3 gravity = GetGravity(this);
	vel += gravity/mass*dt;
	
	if(Entity *e = Collide(this,pos,vel*dt))
	{
		/*if(abs(pos.x-e->pos.x)>abs(pos.y-e->pos.y)){
			vel.x*=-1;
			e->vel.x*=-1;
		}else{
			vel.y*=-1;
			e->vel.y*=-1;
		}*/
		
		vel = (mass*vel + e->mass*e->vel)/(mass+e->mass);
		pos = (mass*pos + e->mass*e->pos)/(mass+e->mass);
		mass+=e->mass;
		//e->Respawn();
		e->hp = 0;
	}
	
	pos+=vel*dt;
	if(trail.size())
		trail[0] = pos;
	
	size = vec2(RadiusFromMass(mass));
}
void Unit::UpdateTail()
{
	if(hp<=0)
		return;
	
	if(trail.size()>64){
		trail.pop();
	}
	trail.push(pos);
}

void Unit::Respawn()
{
	hp = 100;
	
	mass = linearRand(1.0f, 4.0f);

	size = vec2(RadiusFromMass(mass));
	const float rr = 12;
	//pos = linearRand(vec3(-rr,-rr,0.0),vec3(rr,rr,0));
	pos = vec3(glm::diskRand(rr),0.0f);
	pos += normalize(pos-planetPos)*planetRadius*1.2f;
	//vel = linearRand(vec3(-0.6,-0.6,0),vec3(0.6,0.6,0));
	//vel = vec3(0);
	
	//vel = glm::cross(normalize(planetPos-pos),vec3(0,0,1.0f))*linearRand(-1.6f,1.6f);
	vel = glm::cross(normalize(planetPos-pos),vec3(0,0,1.0f)) * glm::sqrt(G*(planetMass/(glm::length(planetPos-pos))));
	//vel *= (rand()&1 ? 1.0f : -1.0f);
	vel *= linearRand(0.9f,1.1f);
	
	vel.z = 0;
	trail.clear();
}

std::vector<Entity*> entities;

Entity * Collide(Unit *u, vec3 orig, vec3 vel)
{
	//vec3 dst = orig+vel;
	for(uint32_t i=0;i<entities.size();i++){
		Entity *e = entities[i];
		if(e==u || e->hp<=0)
			continue;
		/*if(e->pos.x>dst.x+u->size.x||e->pos.x<dst.x-u->size.x)
			continue;
		if(e->pos.y>dst.y+u->size.y||e->pos.y<dst.y-u->size.y)
			continue;*/
		float d = glm::distance(e->pos,u->pos);
		if(d > e->size.x+u->size.x)
			continue;
		return e;
	}
	
	return 0;
}


vec3 GetGravity(Unit *u)
{
	vec3 g=vec3(0);
	g += normalize(planetPos-u->pos)*G*((planetMass*u->mass)/glm::pow(glm::distance(u->pos,planetPos),2.0f));
	for(uint32_t i=0;i<entities.size();i++){
		Entity *e = entities[i];
		if(e==u || e->hp<=0)
			continue;
		vec3 d = e->pos-u->pos;
		g += normalize(d)*G*((e->mass*u->mass)/glm::pow(glm::length(d),2.0f));
	}
	return g;
}

Mesh meshCircle;

class GravityGame: public IGame{
public:
	GravityGame(){}
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "base";
	}
	void OnTouch(float tx, float ty, int ta, int tf);
	void DrawCircle(vec3 pos, float size);
	IRenderer *rend;
	ResourceManager *resMan;
	Camera cam;
	//vec2 pos;

	bool drawTrails;
	Button bAdd;
	Button bTrails;
	Scroll bScroll;
};

IGame *CreateGame(){
	return new GravityGame();
}

Unit *CreateSatellite(float mass, float height, Unit *parent)
{
	Unit *un = new Unit();
	un->mass = mass;
	un->size = vec2(RadiusFromMass(un->mass));
	if(parent)
		un->pos = parent->pos + vec3(0, height, 0);
	else
		un->pos = vec3(0, planetRadius*1.2f+height, 0);
	if(parent)
		un->vel = parent->vel + glm::cross(normalize(parent->pos-un->pos),vec3(0,0,1.0f)) * glm::sqrt(G*(parent->mass/(glm::length(parent->pos-un->pos))));
	else
		un->vel = glm::cross(normalize(planetPos-un->pos),vec3(0,0,1.0f)) * glm::sqrt(G*(planetMass/(glm::length(planetPos-un->pos))));
	un->vel.z = 0;
	un->trail.clear();
	
	//un2->vel = glm::cross(normalize(planetPos-un2->pos),vec3(0,0,1.0f)) * glm::sqrt(G*(planetMass/(glm::length(planetPos-un2->pos))));
	//un2->vel += glm::cross(normalize(un->pos-un2->pos),vec3(0,0,1.0f)) * glm::sqrt(G*(un->mass/(glm::length(un->pos-un2->pos))));

	return un;
}

void GravityGame::Created()
{
	srand(time(0));
	
	Log("GravityGame Created()\n");
	resMan = new ResourceManager();
	resMan->Init();
	rend = CreateRenderer();
	rend->Init(RENDERER_GUI,resMan);
	
	int nv = 32*2;
	float *v = new float[nv*2];
	int cv=0;
	for(int i=0;i<32;i++){
		float a = i/32.0f*M_PI*2;
		float b = (i+1)/32.0f*M_PI*2;
		v[cv++] = sin(a);
		v[cv++] = cos(a);
		v[cv++] = sin(b);
		v[cv++] = cos(b);
	}
	meshCircle = Mesh(v, nv, GL_LINES);
	
	planetRadius = RadiusFromMass(planetMass);
	
	for(int i=0;i<4;i++){
		Unit *un = new Unit();
		un->Respawn();
		entities.push_back(un);
	}
	
	//
	{
		Unit *un1 = CreateSatellite(35, 10, NULL);
		entities.push_back(un1);
		
		Unit *un2 = CreateSatellite(1, 0.8, un1);
		entities.push_back(un2);
		
		Unit *un3 = new Unit();
		entities.push_back(un3);
		un3->mass = 0.04f;
		un3->size = vec2(RadiusFromMass(un3->mass));
		un3->pos = un2->pos+vec3(0,0.09,0);
		un3->vel = glm::cross(normalize(planetPos-un3->pos),vec3(0,0,1.0f)) * glm::sqrt(G*(planetMass/(glm::length(planetPos-un3->pos))));
		un3->vel += glm::cross(normalize(un1->pos-un3->pos),vec3(0,0,1.0f)) * glm::sqrt(G*(un1->mass/(glm::length(un1->pos-un3->pos))));
		un3->vel += glm::cross(normalize(un2->pos-un3->pos),vec3(0,0,1.0f)) * glm::sqrt(G*(un2->mass/(glm::length(un2->pos-un3->pos))));
		un3->vel.z = 0;
		un3->trail.clear();
	}
	
	{
		const float ir = 19;
		const float cw = 2;
		for(int i=0; i<asteroidsCount; i++){
			Unit *un = new Unit();
			un->mass = linearRand(0.04f, 2.0f);
			un->size = vec2(RadiusFromMass(un->mass));
			un->pos = vec3(glm::circularRand(ir), 0.0f);
			un->pos += normalize(un->pos - planetPos)*linearRand(0.0f, cw);
			un->vel = glm::cross(normalize(planetPos-un->pos), vec3(0,0,1.0f)) * glm::sqrt(G*(planetMass/(glm::length(planetPos-un->pos))));
			//un->vel *= linearRand(0.9f,1.1f);
			un->vel.z = 0;
			un->trail.clear();
			entities.push_back(un);
		}
	}
	//
	
	cam = Camera();
	
	drawTrails = true;
	
	bAdd = Button(0.02,0.1,0.2,0.08,"Add");
	bScroll = Scroll(0.8,0.1,0.2,0.9);
	bScroll.pos = 10;
	bTrails = Button(0.25,0.1,0.3,0.08, "Draw trails");
	rend->AddButton(&bAdd);
	rend->AddButton(&bTrails);
	

}

void GravityGame::Changed(int w, int h)
{
	rend->Resize(w,h);
	aspect = rend->aspect;
	//planetPos.x = 0.5f*aspect;
	cam.SetOrtho(aspect,bScroll.pos,1);
}

void GravityGame::DrawCircle(vec3 pos, float size)
{
	mat4 mtx = translate(mat4(1.0f),pos);
	mtx = scale(mtx,vec3(size));
	rend->SetModelMtx(mtx);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, meshCircle.verts);
	glDrawArrays(meshCircle.mode,0,meshCircle.numVerts);
}

void GravityGame::Draw()
{
	if(bAdd.pressed)
	{
		bAdd.pressed=0;
		Unit *un = new Unit();
		un->Respawn();
		entities.push_back(un);
	}
	if(bTrails.pressed)
	{
		bTrails.pressed = false;
		drawTrails = !drawTrails;
		if(drawTrails)
		{
			for(size_t i = 0;i<entities.size();i++){
				entities[i]->trail.clear();
			}
		}
	}
	
	cam.SetOrtho(aspect,bScroll.pos*2,1);
	
	float deltaTime = 0.02;
	int subSteps = 10;
	for(int i=0;i<subSteps;i++){
		for(size_t i = 0;i<entities.size();i++){
			entities[i]->Update(deltaTime/subSteps * timeScale);
		}
		
		for(size_t i = 0;i<entities.size();i++)
		{
			Entity *e = entities[i];
			if(e->hp<=0){
				entities.erase(entities.begin()+i);
				delete e;
				i--;
			}
		}
	}
	static int t=0;
	if(drawTrails){
		t++;
		if(t%10==0){
			for(size_t i = 0;i<entities.size();i++){
				entities[i]->UpdateTail();
			}
		}
	}
	rend->Draw();
	//rend->DrawRect(0.1,0.1,0.1/rend->aspect,0.1);
	//rend->DrawText("test",0.45*rend->aspect,0.1,0.6*rend->aspect);
	
	/*rend->DrawRect(pos.x,pos.y,0.1,0.1*rend->aspect);
	char temp[256];
	snprintf(temp,256,"pos: (%.3f, %.3f)",pos.x,pos.y);
	rend->DrawText(temp,0.1,0.2,0.2*rend->aspect);
	*/
	
	char temp[256];
	snprintf(temp,256,"objects count: %d\nplanet mass %.3f",entities.size(), planetMass);
	rend->DrawText(temp,0.02,0.9,0.2*rend->aspect);
	
	//mat4 mtx(1.0f);
	rend->Set2DMode();
	rend->SetCamera(&cam);
	glDisableVertexAttribArray(2);
	for(size_t i = 0;i<entities.size();i++){
		Entity *e = entities[i];
		//rend->DrawRect(e->pos.x/aspect,e->pos.y,e->size.x/rend->aspect,e->size.y);
		DrawCircle(e->pos,e->size.x);
	}

	rend->SetModelMtx(mat4(1));
	if(drawTrails)
	{
		for(size_t i = 0;i<entities.size();i++){
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, entities[i]->trail.data());
			glDrawArrays(GL_LINE_STRIP,0,entities[i]->trail.size());
		}
	}
	/*
	mtx = translate(mtx,planetPos);
	rend->SetModelMtx(mtx);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, meshPlanet.verts);
	glDrawArrays(meshPlanet.mode,0,meshPlanet.numVerts);
	*/
	DrawCircle(planetPos,planetRadius);
}

Button::Button(float nx, float ny, float nw, float nh, bool adjust):
			x(nx),y(ny),w(nw),h(nh),text(0),active(true),pressed(false)
{
	/*if(adjust){
		if(aspect>1)
			h*=aspect;
		else
			w/=aspect;
	}*/
}

Button::Button(float nx, float ny, float nw, float nh, const char *t, bool adjust):
			x(nx),y(ny),w(nw),h(nh),text(t),active(true),pressed(false)
{
	//if(adjust)
	//	w/=aspect;
}
void Button::Update(){}
bool Button::SetUniform(int loc){return false;}

void GravityGame::OnTouch(float tx, float ty, int ta, int tf){
	float x = tx/rend->width;
#ifdef ANDROID
	float y = (ty-64)/rend->height;
#else
	float y = ty/rend->height;
#endif

	if(ta==0){
		bAdd.Hit(x,y);
		bTrails.Hit(x,y);
		bScroll.Hit(x,y,tf);
	}else if(ta==1){
		bScroll.Release(tf);
	}else if(ta==2){
		bScroll.Move(x,y,tf);
	}
	//pos = vec2(x,y);
}
