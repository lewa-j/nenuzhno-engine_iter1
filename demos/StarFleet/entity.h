
#pragma once

#include <mat4x4.hpp>
#include <vec3.hpp>
#include "cull/BoundingBox.h"
#include "scene/Scene.h"

class IRenderer;
class Entity;
void CreateProjectile(Entity *shooter,glm::vec3 dir,float damage);
void CreateExplosion(Entity *owner,float size);

enum entType{
	eNone,
	eExpl,
	eProj,
	eShip
};

class Entity: public SceneObject
{
public:
	glm::mat4 displayMtx;
	glm::vec3 velocity;
	glm::vec3 position;
	glm::vec3 angles;
	bool remove;
	int type;
	
	Entity(){
		type=eNone;
		remove=false;
		modelMtx=glm::mat4(1.0f);
		displayMtx=glm::mat4(1.0f);
	}
	virtual ~Entity(){}
	
	virtual void Update(float deltaTime){}
};

class Asteroid: public Entity{
public:
	Asteroid(glm::vec3 pos, float size);
	virtual void Update(float deltaTime);
	virtual void Draw(IRenderer *r);
	
	float size;
	float ang;
};//in ship.cpp

class Explosion: public Entity
{
public:
	Explosion(glm::vec3 pos, float aSize);
	float life;
	float size;
	
	virtual void Update(float deltaTime);
	virtual void Draw(IRenderer *r);
};

class Projectile: public Entity
{
public:
	float life;
	float damage;
	Entity *shooter;
	
	Projectile(Entity *s,float damage);
	~Projectile(){}
	
	virtual void Update(float deltaTime);
	virtual void Draw(IRenderer *r);
};

struct shipGun_t{
	float range;
	float speed;
	float damage;
	float rate;
	float spread;

	shipGun_t(float aRange, float aSpeed,float aDamage,float aRate, float aSpread){
		range = aRange;
		speed = aSpeed;
		damage = aDamage;
		rate = aRate;
		spread = aSpread;
	}
};

enum eShipState
{
	eShipState_Idle,
	eShipState_Pursue,
	eShipState_Attack,
	//eShipState_LastEnum
};
class Ship: public Entity
{
public:
	eShipState state;
	float lastShot;
	float hp;
	float t;
	Ship *target;
	glm::vec3 targetPos;
	glm::vec3 toEnemy;
	bool isDead;
	glm::vec3 newAngles;
	BoundingBox bbox;
	shipGun_t gun;
	
	Ship();
	~Ship(){}

	virtual void Update(float deltaTime);
	virtual void Draw(IRenderer *r);
	void Shoot();
	void Hit(float d);
};
