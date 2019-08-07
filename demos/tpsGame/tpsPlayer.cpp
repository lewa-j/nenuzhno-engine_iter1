
#include "tpsPlayer.h"
#include "renderer/camera.h"
#include "renderer/Model.h"
#include "renderer/renderer.h"
#include "button.h"

#include <ext/vec1.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::translate;

void GameLog(const char *s);

Player::Player(Camera *c,glm::vec3 start,glm::vec3 startRot,Model *m):pos(start),rot(startRot),cam(c),mdl(m),move(0),look(0),jump(0),bAttack(0),tps(true),weapon(0),phys(0){
	dynamic = true;
	modelMtx = glm::translate(glm::mat4(1),pos);
	modelMtx = glm::rotate(modelMtx,glm::radians(rot.y),glm::vec3(0,1,0));
}

void Player::Update(float deltaTime)
{
	float cameraHeight = 1.6;
	//cameraHeight = 1;
	float moveSpeed = 4;


	pos = phys->getPosition()+vec3(0,-0.9,0);
	if(jump->pressed){
		jump->pressed=false;
		phys->jump(btVector3(0,1,0));
		//GameLog("jump\n");
	}

#ifdef ANDROID
	rot.y+=-glm::clamp(look->vel.x,-0.4f,0.4f)*270.0f*deltaTime;
	if(glm::abs(rot.y)>180.0f){
		rot.y = rot.y-(int(rot.y)/360)*360.0f;
	}
	rot.x+=look->vel.y*40.0f*deltaTime;
#else
	rot.y += -look->vel.x*270.0f;
	rot.x += look->vel.y*160.0f;
#endif
	rot.x = glm::clamp(rot.x,-85.0f,85.0f);

	glm::mat4 rotMtx(1.0);
	rotMtx = glm::rotate(rotMtx,glm::radians(rot.y),glm::vec3(0,1,0));

	glm::vec3 vel(move->vel.x,0.0f,-move->vel.y);
	if(glm::length(vel)>0.01f){
		vel*=8.0;//joystick sense
		if(glm::length(vel)>moveSpeed){
			vel=glm::normalize(vel)*moveSpeed;
		}
		vel = glm::mat3(rotMtx)*vel;
		//noclip
		//pos += vel*deltaTime;
		phys->setWalkDirection(*((btVector3*)&vel));

		if(glm::abs(rot.y-rot.z)>deltaTime*500.0f){
			rot.z += glm::sign(rot.y-rot.z)*500.0f*deltaTime;
		}else{
			rot.z = rot.y;
		}
	}else{
		phys->setWalkDirection(btVector3(0,0,0));
	}

	if(!tps){
		rot.z = rot.y;
	}

	rotMtx = glm::mat4(1.0);
	rotMtx = glm::translate(rotMtx,glm::vec3(0,cameraHeight,0));
	rotMtx = glm::rotate(rotMtx,glm::radians(rot.y),glm::vec3(0,1,0));
	rotMtx = glm::rotate(rotMtx,glm::radians(-rot.x),glm::vec3(1,0,0));

	view = glm::mat3(rotMtx)*glm::vec3(0,0,-1);

	if(weapon&&bAttack->pressed){
		vec3 weapPos = pos+vec3(rotMtx*vec4(0.1f,-0.1f,0.0f,1.0f));
		weapon->Attack(weapPos,view);
		if(!weapon->autoFire)
			bAttack->pressed = false;
	}

	vec3 camOffs;
	if(tps)
		camOffs = glm::vec3(rotMtx*glm::vec4(0.55,0,1.5,1));
	else
		camOffs = vec3(0,1.7,0);

	cam->rot = glm::vec3(rot.x,rot.y,0);
	cam->pos = pos+camOffs;
	cam->UpdateView();
	modelMtx = glm::translate(glm::mat4(1),pos);
	modelMtx = glm::rotate(modelMtx,glm::radians(rot.z),glm::vec3(0,1,0));
}

void Player::Draw(IRenderer *r){
	if(tps)
		r->DrawModel(mdl, modelMtx);
	if(weapon&&weapon->mdl){
		mat4 mtx = modelMtx;
		mtx = glm::translate(mtx,glm::vec3(0,1.65,-0.05));
		mtx = glm::rotate(mtx,glm::radians(-rot.x),glm::vec3(1,0,0));
		mtx = mtx*weapon->offsetMtx;
		r->DrawModel(weapon->mdl,mtx);
	}
}

void Player::SetPos(vec3 p){
	pos = p;
	phys->warp(*((btVector3*)&p));
}

void Player::SetRot(vec3 r){
	rot=vec3(r.x,r.y,r.y);
}

