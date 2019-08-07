
#include <gtc/type_ptr.hpp>

#include "log.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/glsl_prog.h"
#include "graphics/ArrayBuffer.h"
#include "renderer/mesh.h"
#include "renderer/camera.h"
#include "game/IGame.h"
#include "engine.h"

class CubeGame : public IGame{
public:
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "base";
	}
};

IGame *CreateGame(){
	return new CubeGame();
}

GLfloat vertices[] = 
{
	 0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	 0.5f, 0.5f,  0.5f,
	 0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f,  0.5f
};

int inds[] =
{
	0,1, 1,2, 2,3, 3,0,
	4,5, 5,6, 6,7, 7,4,
	0,4, 1,5, 2,6, 3,7
};
int ni=24;

int scrWidth = 0;
int scrHeight = 0;

glslProg simpleProg;
Mesh *cube;
glm::mat4 modelMtx;
Camera camera;
glm::mat4 mvpMtx(1);

void CubeGame::Created()
{
	Log("%s\n",glGetString(GL_VERSION));

	simpleProg.CreateFromFile("simple", "simple");
	simpleProg.u_mvpMtx = simpleProg.GetUniformLoc("u_mvpMtx");
	
	glUseProgram(0);
	CheckGLError("Created shaders", __FILE__, __LINE__);

	//cube = LoadMeshFile("cube", true);
	cube = new Mesh(vertices,8,(GLushort*)inds,ni,GL_LINES);
	modelMtx = glm::scale(glm::mat4(1.0),glm::vec3(1.2,1.0,0.5));
	CheckGLError("Created meshes", __FILE__, __LINE__);

	//glm::scale(glm::translate(glm::mat4(1.0),glm::vec3(0,0.1,0)),glm::vec3(0.2))
	camera.pos = glm::vec3(0);
	camera.rot = glm::vec3(30,30,0);
	camera.UpdateView();

	glBindTexture(GL_TEXTURE_2D, 0);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//	glEnable(GL_CULL_FACE);
	CheckGLError("Created", __FILE__, __LINE__);
}

void CubeGame::Changed(int w, int h)
{
	scrWidth = w;
	scrHeight = h;
	glViewport(0, 0, w, h);
	float aspect = w/(float)h;
	//camera.UpdateProj(75.0f, aspect, 0.1f, 2.0f);
	camera.SetOrtho(aspect, 2.0f, 1.0f);
}

void CubeGame::Draw()
{
	camera.UpdateView();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	CheckGLError("Clear", __FILE__, __LINE__);
	glEnableVertexAttribArray(0);

	simpleProg.Use();
	mvpMtx = camera.projMtx * camera.viewMtx * modelMtx;
	glUniformMatrix4fv(simpleProg.u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));

	cube->Bind();
	//cube->Draw();
	glDrawElements(GL_LINES, ni, GL_UNSIGNED_INT, inds);
	cube->Unbind();
	if(CheckGLError("Draw cube", __FILE__, __LINE__))
		EngineError("stop");

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(0);
	CheckGLError("Draw", __FILE__, __LINE__);
}
