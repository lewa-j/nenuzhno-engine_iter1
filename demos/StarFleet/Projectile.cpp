
#include <gtc/matrix_transform.hpp>
#include "entity.h"

void TraceLine(Projectile *proj, float deltaTime);

Projectile::Projectile(Entity *s,float aDamage): Entity()
{
	dynamic = true;
	type = eProj;
	life = 0;
	shooter = s;
	damage = aDamage;
}
void Projectile::Update(float deltaTime)
{
	if(remove)
		return;

	life+=deltaTime;
	if(life>4)
	{
		remove = true;
		CreateExplosion(this,0.5f);
		return;
	}
	modelMtx = glm::translate(glm::mat4(1),position);
	modelMtx = glm::rotate(modelMtx,glm::radians(angles.y),glm::vec3(0,1,0));
	modelMtx = glm::rotate(modelMtx,glm::radians(angles.x),glm::vec3(1,0,0));
	position += /*glm::mat3(modelMtx)**/velocity*deltaTime;
	displayMtx = glm::scale(modelMtx,glm::vec3(0.02,0.02,0.1));
	TraceLine(this,deltaTime);
}

