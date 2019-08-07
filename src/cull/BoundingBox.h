
#pragma once

#include <vec3.hpp>

struct vertAttrib_t;

class BoundingBox
{
public:
	glm::vec3 min;
	glm::vec3 max;
	
	BoundingBox()
	{}
	BoundingBox(glm::vec3 mn, glm::vec3 mx): min(mn),max(mx)
	{}
	BoundingBox(short mn[3], short mx[3])
	{
		min = glm::vec3(mn[0],mn[1],mn[2]);
		max = glm::vec3(mx[0],mx[1],mx[2]);
	}
	
	bool Intersect(glm::vec3 orig, glm::vec3 dir);
	bool Contains(glm::vec3 pos, float radius);
	
	void FromVerts(const char *verts,int num, const vertAttrib_t &va);
	//void AddVert(glm::vec3 v)
	//{}
};

