
#pragma once

#include <string>
#include <vector>
#include <mat4x4.hpp>
#include <vec3.hpp>

class IRenderer;
class Model;
class Texture;

class SceneObject
{
public:
	SceneObject():modelMtx(1.0f), parent(0), dynamic(false),name(){
	}
	virtual void Draw(IRenderer *r){}
	virtual void Update(float delta){}
	void SetMtx(glm::mat4 &mtx){
		modelMtx = mtx;
	}

	glm::mat4 modelMtx;
	SceneObject *parent;
	bool dynamic;//TODO flags
	std::string name;
};

enum eLightType
{
	eDirectional=1,
	ePoint=2,
	eSpot=3
};

class LightObject
{
public:
	LightObject():type(ePoint),pos(),dir(),color(),radius(5),fov(90){}
	LightObject(eLightType t,glm::vec3 p, glm::vec3 c,float r):type(t),pos(p),color(c),radius(r),fov(90){}
	eLightType type;
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 color;
	float radius;
	float fov;
};

class StaticModel:public SceneObject
{
public:
	StaticModel();
	
	virtual void Draw(IRenderer *r);
	Model *mdl;
};

class Scene
{
public:
	Scene();
	
	void Update(float delta);
	void Draw(IRenderer *r);
	void AddObject(SceneObject *obj);
	//SceneObject *NewObject<typename T>();
	void RemoveObject(SceneObject *obj);
	SceneObject *Find(const char *name);
	void AddLight(LightObject *l);
	
	std::vector<SceneObject*> objects;
	std::vector<SceneObject*> dynamicObjects;
	std::vector<LightObject*> lights;

	glm::vec3 startPos;
	glm::vec3 startRot;
	Texture *skyBox;//cubemap
	glm::vec3 sunDirection;
};

