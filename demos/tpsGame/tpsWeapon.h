
#pragma once

#include "scene/Scene.h"
#include <mat4x4.hpp>
#include <vec3.hpp>
using glm::vec3;

class IRenderer;
class Model;

void ShootBullet(vec3 orig, vec3 dir);

enum eWeaponType{
	eWeapNone,
	eWeapMellee,
	eWeapPistol
};

class Weapon{
public:
	Weapon();
	Weapon(eWeaponType tp, Model *m, float dmg, float rt);

	bool Attack(vec3 pos, vec3 vel);
	
	Model *mdl;
	glm::mat4 offsetMtx;
	float damage;
	float rate;
	eWeaponType type;
	bool autoFire;
};

class Projectile: public SceneObject{
public:
	Projectile(vec3 p, vec3 v);
	virtual void Update(float deltaTime);
	virtual void Draw(IRenderer *r);
	
	vec3 pos;
	vec3 vel;
	float t;
};

