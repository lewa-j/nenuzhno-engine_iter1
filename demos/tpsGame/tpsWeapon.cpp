
#include "tpsWeapon.h"
#include "renderer/Model.h"
#include "renderer/renderer.h"
#include <gtc/matrix_transform.hpp>
using glm::translate;
using glm::scale;

Weapon::Weapon(): mdl(0),offsetMtx(1.0f),damage(0),rate(1),type(eWeapNone){
	
}

Weapon::Weapon(eWeaponType tp, Model *m, float dmg, float rt):mdl(m),offsetMtx(1.0f),damage(dmg),rate(rt),type(tp){
	
}

bool Weapon::Attack(vec3 pos, vec3 vel){
	ShootBullet(pos,vel*50.0f);
	return true;
}

Projectile::Projectile(vec3 p, vec3 v):pos(p),vel(v),t(0){
	dynamic = true;
}

void Projectile::Update(float deltaTime){
	t += deltaTime;
	if(t>5)
		return;
	pos += vel*deltaTime;
	modelMtx = glm::translate(glm::mat4(1),pos);
	modelMtx = glm::scale(modelMtx,vec3(0.02f));
}

extern Model *g_mdlBox;

void Projectile::Draw(IRenderer *r){
	if(t>5)
		return;
	r->DrawModel(g_mdlBox, modelMtx);
}

