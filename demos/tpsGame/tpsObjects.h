
#pragma once

#include <vec3.hpp>
class Model;
class IRenderer;
class Player;

extern Model *g_mdlBox;

class ActivatingObject: public SceneObject{
public:
	virtual void Active()=0;
};

#define DOOR_CLOSED 0
#define DOOR_CLOSING 1
#define DOOR_OPEN 2
#define DOOR_OPENING 3

class DoorObject: public ActivatingObject{
public:
	DoorObject(glm::vec3 p,Model *m,glm::vec3 s=glm::vec3(0.5,1,0.1));
	virtual void Active();
	virtual void Update(float deltaTime);
	virtual void Draw(IRenderer *r);
	
	glm::vec3 pos;
	glm::vec3 openPos;
	glm::vec3 closePos;
	glm::vec3 size;
	Model *mdl;
	int state;
	float t;
};

#define BUTTON_NONE 0
#define BUTTON_ACTIVE 1
#define BUTTON_TP 2
class ButtonObject: public SceneObject{
public:
	ButtonObject(glm::vec3 p);
	~ButtonObject();
	virtual void Update();
	virtual void Draw(IRenderer *r);
	virtual void Press(Player *pl);
	
	glm::vec3 pos;
	float rot;
	std::string text;
	Model *mdl;
	ActivatingObject *target;
	glm::vec3 targetPos;
	glm::vec3 targetRot;
	int type;
};

