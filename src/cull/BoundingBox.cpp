
#include <common.hpp>
#include "BoundingBox.h"
#include "renderer/Model.h"
#include "engine.h"

using glm::vec3;

bool BoundingBox::Intersect(vec3 orig, vec3 dir){
	vec3 tMin = (min - orig) / dir;
	vec3 tMax = (max - orig) / dir;
	vec3 t1 = glm::min(tMin, tMax);
	vec3 t2 = glm::max(tMin, tMax);
	float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
	float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
	//return vec2(tNear, tFar);
	return (tNear > 0.0 && tNear < 1.0 && tNear < tFar);
}

bool BoundingBox::Contains(glm::vec3 pos, float radius){
	//for(int i=0; i<6; i++){
		//if(glm::dot(glm::vec3(planes[i]),pos) + planes[i].w <= -radius)
		//	return false;
	//}
	return true;
}

void BoundingBox::FromVerts(const char *verts, int num, const vertAttrib_t &va){
	if(va.size!=3||va.type!=GL_FLOAT){
		EngineError("BoundingBox::FromVerts(): vertex type not a vec3");
		return;
	}

	min = vec3(999999);
	max = vec3(-999999);

	for(int i=0;i<num;i++){
		vec3 v = *((vec3*)(verts+i*va.stride));

		min.x = glm::min(min.x,v.x);
		min.y = glm::min(min.y,v.y);
		min.z = glm::min(min.z,v.z);
		max.x = glm::max(max.x,v.x);
		max.y = glm::max(max.y,v.y);
		max.z = glm::max(max.z,v.z);
	}
}

