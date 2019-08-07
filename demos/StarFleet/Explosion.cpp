
#include <gtc/matrix_transform.hpp>
#include "entity.h"

Explosion::Explosion(glm::vec3 pos, float aSize): Entity(){
	dynamic = true;
	type=eExpl;
	life=0;
	position=pos;
	size=aSize;
	
	modelMtx = glm::translate(glm::mat4(1),position);
	displayMtx = glm::scale(modelMtx,glm::vec3(size));
}

void Explosion::Update(float deltaTime){
	life+=deltaTime;
	
	if(life>1.0f){
		remove = true;
		return;
	}
	displayMtx = glm::scale(modelMtx,glm::vec3(size*pow((1.0-life),3.0)));
}

