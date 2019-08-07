
#include "renderer/camera.h"

Camera::Camera()
{
	pos = glm::vec3(0);
	rot = glm::vec3(0);
	projMtx = glm::mat4(1);
	viewMtx = glm::mat4(1);
}

void Camera::UpdateView()
{
	viewMtx = glm::mat4(1.0f);
	viewMtx = glm::rotate( viewMtx, glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
	viewMtx = glm::rotate( viewMtx, glm::radians(rot.y), glm::vec3(0.0f, -1.0f, 0.0f));
	viewMtx = glm::translate( viewMtx, -pos);
}

void Camera::UpdateProj(float fov, float aspect, float near, float far)
{
	projMtx = glm::perspective(glm::radians(fov), aspect, near, far);
}

void Camera::SetOrtho(float aspect, float size, float depth)
{
	//projMtx = glm::ortho(-size*aspect, size*aspect, -size, size, size, -size);
	projMtx = glm::ortho(size*aspect, -size*aspect, -size, size, depth, -depth);
}

void Camera::UpdateFrustum()
{
	frustum.Init(projMtx*viewMtx);
}

void Camera::LookAt(glm::vec3 orig, glm::vec3 to, glm::vec3 up)
{
	viewMtx = glm::lookAt(orig,to,up);
}
