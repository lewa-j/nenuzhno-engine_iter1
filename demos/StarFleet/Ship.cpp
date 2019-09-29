
#include <gtc/matrix_transform.hpp>
#include <gtc/random.hpp>
#include "entity.h"
#include "log.h"
#include "renderer/renderer.h"

Ship::Ship(): Entity(),gun(10,6.5f,20,0.35f,0.05f){
	dynamic = true;
	type = eShip;
	state = eShipState_Idle;
	lastShot = 0;
	hp = 100;
	t = 0;
	target = 0;
	targetPos = glm::vec3(0);
	isDead = false;
	bbox = BoundingBox(glm::vec3(-0.6f,-0.5f,-0.8f),glm::vec3(0.6f,0.5f,0.8f));
}

void Ship::Update(float deltaTime)
{
	lastShot += deltaTime;
	t -= deltaTime;
	if(hp <= 0.0f){
		//isDead = true;
		return;
	}
	modelMtx = glm::translate(glm::mat4(1),position);
	modelMtx = glm::rotate(modelMtx,glm::radians(angles.y),glm::vec3(0,1,0));
	modelMtx = glm::rotate(modelMtx,glm::radians(angles.x),glm::vec3(1,0,0));
	if(state != eShipState_Idle){
		glm::vec3 forward = glm::normalize(glm::mat3(modelMtx)[2]);
		position += glm::mat3(modelMtx)*velocity*deltaTime;
		glm::vec3 da;
		if(target&&target->isDead)
			target=0;
		//todo find new target
		if(target){
			toEnemy = target->position - position;
			float dist = glm::length(toEnemy);
			if(dist > 2.0)
				velocity = glm::vec3(0,0,2);
			else
				velocity = glm::vec3(0,0,1);
			//advance aiming
			toEnemy = glm::normalize(target->position+(normalize(glm::vec3(target->modelMtx[2]))*target->velocity.z*(dist/gun.speed))-position);
			if(state==eShipState_Attack&&lastShot>gun.rate){
				if(dist<gun.range && glm::dot(forward,toEnemy)>0.95){
					Shoot();
					lastShot=0;
				}
			}
			newAngles.x = glm::degrees(glm::asin(-toEnemy.y));
			newAngles.y = glm::degrees(glm::atan(toEnemy.x,toEnemy.z));
		}else{
			if(t<0){
				t = glm::linearRand(2.0f,5.0f);
				//newAngles = glm::sphericalRand(180.0f);
				targetPos=glm::ballRand(50.0f);
				glm::vec3 newDir = normalize(targetPos-position);
				newAngles.x = glm::degrees(glm::asin(-newDir.y));
				newAngles.y = glm::degrees(glm::atan(newDir.x,newDir.z));
				newAngles.z = 0;
			}
		}
		//angles = newAngles;
		da = newAngles-angles;
		/*if(da.y>180)
			da.y-=180;
		if(da.y<-180)
			da.y+=180;*/
		if(abs(da.y)>180){
			da.y-= ((int)da.y)/180*180;
		}
		da = glm::clamp(da,glm::vec3(-5.0f),glm::vec3(5.0f));
		//dy = glm::clamp(y-angles.y,-5.0f,+5.0f);
		angles += da*deltaTime*20.0f;
		//angles.y += dy*deltaTime*20.0f;
	}
	displayMtx = glm::rotate(glm::scale(modelMtx,glm::vec3(0.1)),glm::radians(-90.0f),glm::vec3(0,1,0));
}

void Ship::Shoot()
{
	//pew
	CreateProjectile(this,(toEnemy+glm::sphericalRand(gun.spread))*gun.speed,gun.damage);
}

void Ship::Hit(float d)
{
	hp-=d;
	if(hp<=0.0)
	{
		//Log("Ship %p dead\n",this);
		CreateExplosion(this,2.0f);
		//isDead = true;
		//todo respawn
		hp = 100;
		position = glm::ballRand(12.0f);//glm::vec3(0.0f);
		angles = glm::vec3(0.0f);
	}
}

Asteroid::Asteroid(glm::vec3 pos,float s){
	dynamic=true;
	position = pos;
	size = s;
	ang = 0.5;
	modelMtx = glm::translate(glm::mat4(size),pos);
	displayMtx = modelMtx;
}

using glm::abs;

void Asteroid::Update(float deltaTime)
{
	position += velocity*deltaTime;
	if(abs(position.x)>300||abs(position.y)>300||abs(position.z)>300){
		position = glm::ballRand(250.0f);
	}
	angles.x += ang*deltaTime;
	
	modelMtx = glm::translate(glm::mat4(size),position);
	modelMtx = glm::rotate(modelMtx,glm::radians(angles.x),glm::vec3(0.8,0.3,0.5));
	displayMtx = modelMtx;
}

