
#pragma once

#include <vec3.hpp>
#include <gtc/matrix_transform.hpp>
#include "scene/Scene.h"
class Camera;
class Model;
class Button;
class Joystick;
class IRenderer;

#include "tpsPhysics.h"
#include "tpsWeapon.h"

class Player: public SceneObject{
public:
	Player(Camera *c,glm::vec3 start,glm::vec3 startRot,Model *m);

	virtual void Update(float deltaTime);
	virtual void Draw(IRenderer *r);
	void SetPos(glm::vec3 p);
	void SetRot(glm::vec3 r);

	glm::vec3 pos;
	glm::vec3 rot;//x,camY,charY
	glm::vec3 view;
	Camera *cam;
	Model *mdl;
	Joystick *move;
	Joystick *look;
	Button *jump;
	Button *bAttack;
	bool tps;
	Weapon *weapon;

	PlayerPhysics *phys;
};
