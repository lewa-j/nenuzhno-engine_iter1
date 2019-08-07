
#include "cull/frustum.h"
#include <glm.hpp>

void Frustum::Init(glm::mat4 mtx)
{
	mtx = glm::transpose(mtx);
	planes[0] = mtx[3]+mtx[0];
	planes[1] = mtx[3]-mtx[0];
	planes[2] = mtx[3]+mtx[1];
	planes[3] = mtx[3]-mtx[1];
	planes[4] = mtx[3]+mtx[2];
	planes[5] = mtx[3]-mtx[2];


	for(int i=0; i<6; i++){
		float l = glm::length(glm::vec3(planes[i]));
		planes[i] /= l;
	}
}

bool Frustum::Contains(glm::vec3 min, glm::vec3 max)
{
	for(int i=0; i<6; i++){
		if(glm::dot(glm::vec3(planes[i]),min) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(min.x,min.y,max.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(min.x,max.y,min.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(min.x,max.y,max.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(max.x,min.y,min.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(max.x,min.y,max.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(max.x,max.y,min.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),max) + planes[i].w > 0)
			continue;
		return false;
	}
	return true;
}

bool Frustum::Contains(BoundingBox bb)
{
	for(int i=0; i<6; i++)
	{
		if(glm::dot(glm::vec3(planes[i]),bb.min) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(bb.min.x,bb.min.y,bb.max.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(bb.min.x,bb.max.y,bb.min.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(bb.min.x,bb.max.y,bb.max.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(bb.max.x,bb.min.y,bb.min.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(bb.max.x,bb.min.y,bb.max.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(bb.max.x,bb.max.y,bb.min.z)) + planes[i].w > 0)
			continue;
		if(glm::dot(glm::vec3(planes[i]),bb.max) + planes[i].w > 0)
			continue;
		return false;
	}
	return true;
}

bool Frustum::Contains(BoundingBox bb, glm::mat4 mtx)
{
	glm::vec3 vecs[8]={
		bb.min,
		glm::vec3(bb.min.x,bb.min.y,bb.max.z),
		glm::vec3(bb.min.x,bb.max.y,bb.min.z),
		glm::vec3(bb.min.x,bb.max.y,bb.max.z),
		glm::vec3(bb.max.x,bb.min.y,bb.min.z),
		glm::vec3(bb.max.x,bb.min.y,bb.max.z),
		glm::vec3(bb.max.x,bb.max.y,bb.min.z),
		bb.max
	};
	for(int v=0;v<8;v++)
	{
		vecs[v] = glm::vec3(mtx*glm::vec4(vecs[v],1.0));
	}
	for(int i=0; i<6; i++)
	{
		int in = 8;
		for(int v=0;v<8;v++)
		{
			if(glm::dot(glm::vec3(planes[i]),vecs[v]) + planes[i].w < 0)
				in--;
		}
		if(in==0)
			return false;
	}
	return true;
}

bool Frustum::Contains(glm::vec4 sphere)
{
	for(int i=0; i<6; i++){
		/*float dist = glm::dot(glm::vec3(sphere),glm::vec3(planes[i]))+planes[i].w;
			if(dist < -sphere.w)
				return false;
			if(glm::abs(dist) < sphere.w)
				return true;
			*/
		if(glm::dot(glm::vec3(planes[i]),glm::vec3(sphere)) + planes[i].w <= -sphere.w)
			return false;
	}
	return true;
}
