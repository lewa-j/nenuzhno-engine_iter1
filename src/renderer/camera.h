
#pragma once

#include <gtc/matrix_transform.hpp>
#include "cull/frustum.h"

class Camera
{
public:
	glm::vec3 pos;
	glm::vec3 rot;
	glm::mat4 projMtx;
	glm::mat4 viewMtx;
	Frustum frustum;

	Camera();
	void UpdateView();
	void UpdateProj(float fov, float aspect, float near, float far);
	void SetOrtho(float aspect, float size, float depth);
	void UpdateFrustum();
	void LookAt(glm::vec3 orig, glm::vec3 to, glm::vec3 up);
};
