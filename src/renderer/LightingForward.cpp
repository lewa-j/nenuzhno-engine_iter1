
#include "renderer/renderer.h"
#include "graphics/platform_gl.h"
#include "graphics/glsl_prog.h"
#include "renderer/Model.h"

using glm::vec3;

class LightingForward: public ILighting{
public:
	LightingForward(){
		r = 0;
		camera = 0;
	}

	virtual void Init(IRenderer *rend){
		r = rend;

		progLightDir.CreateFromFile("generic","lightDir");
		progLightDir.u_mvpMtx = progLightDir.GetUniformLoc("u_mvpMtx");
		progLightDir.u_modelMtx = progLightDir.GetUniformLoc("u_modelMtx");
		progLightDir.u_cameraPos = progLightDir.GetUniformLoc("u_camPos");
		u_lightDir = progLightDir.GetUniformLoc("u_lightDir");

		progLightPoint.CreateFromFile("lightPoint","lightPoint");
		progLightPoint.u_mvpMtx = progLightPoint.GetUniformLoc("u_mvpMtx");
		progLightPoint.u_modelMtx = progLightPoint.GetUniformLoc("u_modelMtx");
		progLightPoint.u_cameraPos = progLightPoint.GetUniformLoc("u_camPos");
		u_lightPos = progLightPoint.GetUniformLoc("u_lightPos");
		u_lightColor = progLightPoint.GetUniformLoc("u_lightColor");
		u_lightSize = progLightPoint.GetUniformLoc("u_lightSize");
	}
	virtual void Draw(){

	}
	virtual void SetDirectional()
	{
		r->UseProg(&progLightDir);
		glUniform3fv(progLightDir.u_cameraPos,1,&(camera->pos.x));
		glUniform3fv(u_lightDir,1,&(scene->sunDirection.x));
	}
	virtual void SetCamera(Camera *cam){
		camera=cam;
	}
	virtual void SetScene(Scene *s){
		scene = s;
	}
	virtual void AddLights(const mat4 &mtx, const submesh_t &submesh){
		glDepthFunc(GL_EQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(1,1);
		r->UseProg(&progLightPoint);
		glUniform3fv(progLightPoint.u_cameraPos,1,&(camera->pos.x));
		vec3 modelPos = vec3(mtx[3]);
		for(uint32_t l=0; l<scene->lights.size();l++){
			LightObject *li=(scene->lights[l]);
			float dist = glm::length(li->pos-modelPos);
			if(dist > li->radius)
				continue;
			glUniform3fv(u_lightPos,1,&(li->pos.x));
			glUniform3fv(u_lightColor,1,&(li->color.x));
			glUniform1f(u_lightSize,li->radius);
			glDrawArrays(GL_TRIANGLES, submesh.offs, submesh.count);
		}
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
	}

	IRenderer *r;
	Camera *camera;
	Scene *scene;
	glslProg progLightDir;
	int u_lightDir;
	glslProg progLightPoint;
	int u_lightPos;
	int u_lightColor;
	int u_lightSize;
};

ILighting *GetLighting(){
	return new LightingForward();
}

