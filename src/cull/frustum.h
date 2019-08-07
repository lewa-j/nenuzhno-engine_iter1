
#pragma once

#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include "cull/BoundingBox.h"

class Frustum
{
public:
	glm::vec4 planes[6];
	
	Frustum()
	{}
	
	void Init(glm::mat4 mtx);
	bool Contains(glm::vec3 min, glm::vec3 max);
	bool Contains(BoundingBox bb);
	bool Contains(BoundingBox bb, glm::mat4 mtx);
	bool Contains(glm::vec4 sphere);
};

