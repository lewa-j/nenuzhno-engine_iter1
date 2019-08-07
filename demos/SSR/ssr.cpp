
#include <gtc/type_ptr.hpp>

#include "log.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/glsl_prog.h"
#include "graphics/ArrayBuffer.h"
#include "graphics/vao.h"
#include "renderer/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "renderer/camera.h"
#include "game/IGame.h"

class SSRGame : public IGame{
public:
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "ssr";
	}
};

IGame *CreateGame(){
	return new SSRGame();
}

GLfloat vertices[] = 
{
	 0.5f, 0.0f,  0.5f, 0,1,0, 1,1,
	 0.5f, 0.0f, -0.5f, 0,1,0, 1,0,
	-0.5f, 0.0f, -0.5f, 0,1,0, 0,0,
	-0.5f, 0.0f,  0.5f, 0,1,0, 0,1
};

int scrWidth = 0;
int scrHeight = 0;

glslProg simpleProg;
glslProg texProg;
glslProg ssrProg;
int u_eyePos=-1;
Mesh *plane;
Mesh *cube;
VertexBufferObject *vboQuad;
VertexArrayObject vaoQuad;
VertexArrayObject vaoSO;
FrameBufferObject fbo1;
FrameBufferObject fboBack;
Texture testTex;
Camera camera;
glm::mat4 mvpMtx(1);

void UnbindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, scrWidth, scrHeight);
}

class SceneObject
{
public:
	Mesh *mesh;
	glm::mat4 modelMtx;

	SceneObject(){}
	SceneObject(Mesh *msh, glm::mat4 mtx)
	{
		mesh = msh;
		modelMtx = mtx;
	}

	void Draw(GLint u_mvpMtx)
	{
		mvpMtx = camera.projMtx * camera.viewMtx * modelMtx;
		glUniformMatrix4fv(u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));

		mesh->Bind();
		mesh->Draw();
		mesh->Unbind();
	}
};

SceneObject planeSO;
SceneObject cubeSO;

void SSRGame::Created()
{
	simpleProg.CreateFromFile("simple", "simple");
	simpleProg.u_mvpMtx = simpleProg.GetUniformLoc("u_mvpMtx");
	if(texProg.CreateFromFile("generic", "col_tex"))
	{
		texProg.u_mvpMtx = texProg.GetUniformLoc("u_mvpMtx");
		texProg.u_color = texProg.GetUniformLoc("u_color");
		texProg.Use();
		glUniform4f(texProg.u_color, 1,1,1,1);
	}
	ssrProg.CreateFromFile("ssr", "ssr");
	ssrProg.u_mvpMtx = ssrProg.GetUniformLoc("u_mvpMtx");
	ssrProg.u_invModelMtx = ssrProg.GetUniformLoc("u_invMVPMtx");
	u_eyePos = ssrProg.GetUniformLoc("u_eyePos");
	GLint u_depthTex = ssrProg.GetUniformLoc("u_depthTex");
	ssrProg.Use();
	glUniform1i(u_depthTex, 1);
	glUseProgram(0);
	CheckGLError("Created shaders", __FILE__, __LINE__);

	plane = new MeshFBO_N3_T2(vertices, 4, GL_TRIANGLE_FAN);
	cube = LoadMeshFile("cube", true);
	vaoSO.Create();

	float quadVerts[] ={
	 1,-1,1,0,
	 1, 1,1,1,
	-1, 1,0,1,
	-1,-1,0,0
	};
	vboQuad = new VertexBufferObject();
	vboQuad->Create();
	vboQuad->Upload(4*4*4, quadVerts);

	vaoQuad.Create();
	vaoQuad.Bind();
	vboQuad->Bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,(void*)8);
	vboQuad->Unbind();
	vaoQuad.Unbind();

	CheckGLError("Created meshes", __FILE__, __LINE__);

	testTex.Create(4,4);
	GLubyte testTexData[] =
	{
		0,0,0, 255,255,255, 0,0,0, 255,255,255,
		255,255,255, 0,0,0, 255,255,255, 0,0,0,
		0,0,0, 255,255,255, 0,0,0, 255,255,255,
		255,255,255, 0,0,0, 255,255,255, 0,0,0
	};
	testTex.Upload(0, GL_RGB, testTexData);

	planeSO = SceneObject(plane,glm::mat4(1.0));
	cubeSO = SceneObject(cube,glm::scale(glm::translate(glm::mat4(1.0),glm::vec3(0,0.1,0)),glm::vec3(0.2)));
	
	camera.pos = glm::vec3(0,0.6f,1);
	camera.rot = glm::vec3(30,0,0);
	camera.UpdateView();

	fbo1.Create();
	fbo1.CreateTexture(64, 64, GL_LINEAR);
	fbo1.CreateDepthTexture();

	fboBack.Create();
	fboBack.CreateTexture(256,256);
	CheckGLError("Created fbo", __FILE__, __LINE__);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_CULL_FACE);
	CheckGLError("Created", __FILE__, __LINE__);
}

void SSRGame::Changed(int w, int h)
{
	scrWidth = w;
	scrHeight = h;
	glViewport(0, 0, w, h);
	float aspect = w/(float)h;
	camera.UpdateProj(75.0f, aspect, 0.1f, 2.0f);

	fbo1.Resize(w,h);
	CheckGLError("Changed", __FILE__, __LINE__);
}

float a = 0;
void SSRGame::Draw()
{
	a+=0.02;
	camera.pos = glm::vec3(glm::sin(a),0.6f,glm::cos(a));
	camera.rot = glm::vec3(30,glm::degrees(a),0);
	camera.UpdateView();
	
	fbo1.Bind();
	CheckGLError("Bind fbo", __FILE__, __LINE__);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	CheckGLError("Clear", __FILE__, __LINE__);
	vaoSO.Bind();
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnableVertexAttribArray(2);
	texProg.Use();
	testTex.Bind();
	cubeSO.Draw(texProg.u_mvpMtx);
	glDisableVertexAttribArray(2);
	CheckGLError("Draw cube", __FILE__, __LINE__);

	simpleProg.Use();
	planeSO.Draw(simpleProg.u_mvpMtx);
	CheckGLError("Draw plane", __FILE__, __LINE__);
	vaoSO.Unbind();
#if 1
	glDisable(GL_DEPTH_TEST);
	//UnbindFBO();
	fboBack.Bind();
	glClear(GL_COLOR_BUFFER_BIT);

	texProg.Use();
	glUniformMatrix4fv(texProg.u_mvpMtx,1,GL_FALSE,glm::value_ptr(glm::mat4(1.0f)));
	fbo1.BindTexture();
	vaoQuad.Bind();
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	vaoQuad.Unbind();

	ssrProg.Use();
	glm::mat4 invMVPMtx = glm::inverse(camera.projMtx * camera.viewMtx);
	glUniformMatrix4fv(ssrProg.u_invModelMtx,1,GL_FALSE,glm::value_ptr(invMVPMtx));
	glUniform3fv(u_eyePos,1,&camera.pos.x);
	glActiveTexture(GL_TEXTURE1);
	fbo1.BindDepthTexture();
	glActiveTexture(GL_TEXTURE0);
	fbo1.BindTexture();

	vaoSO.Bind();
	glEnableVertexAttribArray(1);
	planeSO.Draw(ssrProg.u_mvpMtx);
	glDisableVertexAttribArray(1);

	UnbindFBO();
	glClear(GL_COLOR_BUFFER_BIT);
	texProg.Use();
	glUniformMatrix4fv(texProg.u_mvpMtx,1,GL_FALSE,glm::value_ptr(glm::mat4(1.0f)));
	fboBack.BindTexture();
	vboQuad->Bind();
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,0);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,(void*)8);
	glEnableVertexAttribArray(2);
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	glDisableVertexAttribArray(2);
	vboQuad->Unbind();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(0);
	vaoSO.Unbind();
#endif
	CheckGLError("Draw", __FILE__, __LINE__);
}
