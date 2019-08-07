
#include <vector>
using std::vector;

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
using glm::mat4;

#include "log.h"
#include "renderer/renderer.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/ArrayBuffer.h"
#include "graphics/glsl_prog.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "renderer/font.h"
#include "renderer/Model.h"
#include "renderer/mesh.h"
#include "button.h"
#include "engine.h"

#include "renderer/GLES2Renderer.h"

void LogRenderInfo();

IRenderer *CreateRenderer()
{
	return new GLES2Renderer();
}

void TexMaterial::Bind(IRenderer *r){
	if(tex)
		tex->Bind();
}

GLES2Renderer::GLES2Renderer()
{
	font = 0;
	lighting = 0;
}

void GLES2Renderer::Init(int initFlags,ResourceManager *resMan)
{
	flags = initFlags;
	Log("Init renderer\n");

	LogRenderInfo();

	progTex.CreateFromFile("generic", "col_tex");
	progTex.u_mvpMtx = progTex.GetUniformLoc("u_mvpMtx");
	progTex.u_modelMtx = -1;
	progTex.u_color = progTex.GetUniformLoc("u_color");
	progTex.Use();
	glUniform4f(progTex.u_color,1,1,1,1);

	progDepth.CreateFromFile("generic","null");
	progDepth.u_mvpMtx = progDepth.GetUniformLoc("u_mvpMtx");

	progSkybox.CreateFromFile("skybox","skybox");
	progSkybox.u_mvpMtx = progSkybox.GetUniformLoc("u_mvpMtx");

	meshCube = LoadMeshFile("cube", true);

	GLubyte whiteData[] = {255};
	texWhite.Create(1,1);
	texWhite.Upload(0, GL_LUMINANCE, whiteData);
	
	if(flags&RENDERER_GUI){
		float quadVerts[] ={
		 1,-1,1,0,
		 1, 1,1,1,
		-1, 1,0,1,
		-1,-1,0,0};

		vboQuad.Create();
		vboQuad.Upload(4*4*4, quadVerts);

		if(resMan){
			font = new Font();
			font->LoadBMFont("sansation",resMan);
		}
	}
	if(flags&RENDERER_LIGHT){
		lighting = GetLighting();
		lighting->Init(this);
	}
	
	if(flags&RENDERER_BACKBUFFER){
		backBufferScale = 1;
		backFBO.Create();
		backFBO.CreateTexture(64,64, GL_LINEAR);
		//backFBO.AddDepthBuffer();
		backFBO.CreateDepthTexture(64,64);
	}

	glClearColor(0,0,0,1);
	//glClearColor(0.4f,0.4f,0.4f,1);
	glUseProgram(0);

	modelMtx = glm::mat4(1.0f);
	vpMtx = glm::mat4(1.0f);
	mvpMtx = glm::mat4(1.0f);

	progCur = 0;

	debug = false;
	drawState = eDSDefault;

	CheckGLError("GLES2Renderer::Init", __FILE__, __LINE__);
}

void GLES2Renderer::SetBackBufferScale(float s)
{
	if(!(flags&RENDERER_BACKBUFFER)){
		return;
	}
	if(s==backBufferScale)
		return;
	backBufferScale=s;
	backFBO.Resize(width*s,height*s);
}

FrameBufferObject *GLES2Renderer::GetFBO()
{
	if(!(flags&RENDERER_BACKBUFFER)){
		return NULL;
	}
	return &backFBO;
}

void GLES2Renderer::Resize(int w, int h)
{
	width = w;
	height = h;
	aspect = w/(float)h;
	if(flags&RENDERER_BACKBUFFER){
		backFBO.Resize(width*backBufferScale,height*backBufferScale);
	}
	glViewport(0,0,w,h);
}

void GLES2Renderer::Draw()
{
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	Clear();
	
	progCur = 0;
	UseProg(&progTex);
	texWhite.Bind();
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	if(scene){
		if(flags&RENDERER_BACKBUFFER){
			glBindTexture(GL_TEXTURE_2D,0);
			backFBO.Bind();
			Clear();
		}
		if(scene->skyBox){
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			DrawSkyBox(scene->skyBox);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			UseProg(&progTex);
			texWhite.Bind();
		}
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		UpdateMtx();
		if(debug)
			debugBBox.clear();
		scene->Draw(this);
		if(debug){
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			DrawDebugScene();
			glEnableVertexAttribArray(2);
		}
		if(flags&RENDERER_BACKBUFFER){
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, width, height);
			mvpMtx = glm::mat4(1.0f);
			UseProg(&progTex);
			SetColor(1,1,1,1);
			backFBO.BindTexture();
			glDisable(GL_DEPTH_TEST);
			vboQuad.Bind();
			glDisableVertexAttribArray(1);
			glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,0);
			glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,(void*)8);
			glDrawArrays(GL_TRIANGLE_FAN,0,4);
			vboQuad.Unbind();
		}
	}
	glDisableVertexAttribArray(1);

	//2d
	UseProg(&progTex);
	texWhite.Bind();
	Set2DMode();
	SetColor(1,1,1,1);
	glDisable(GL_CULL_FACE);
	for(uint32_t i=0;i<buttons.size();i++){
		Button *b=buttons[i];
		if(b->active&&b->text){
			DrawText(b->text,b->x*aspect+0.01,b->y+b->h-0.01,0.5);
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(1,1);
	glUniform4f(progTex.u_color,0.3,0.3,0.3,0.3);
	for(uint32_t i=0;i<buttons.size();i++){
		Button *b=buttons[i];
		if(b->active&&b->text){
			DrawRect(b->x,b->y,b->w,b->h);
		}
	}
	glUniform4f(progTex.u_color,1,1,1,1);
	glDisable(GL_BLEND);

	//glUniformMatrix4fv(progTex.u_mvpMtx,1,false,glm::value_ptr(mtx));

	CheckGLError("GLES2Renderer::Draw", __FILE__, __LINE__);
}

void GLES2Renderer::RenderDepth(Camera *cam)
{
	if(!scene){
		return;
	}
	drawState = eDSDepth;
	glEnableVertexAttribArray(0);
	UseProg(&progDepth);
	glBindTexture(GL_TEXTURE_2D,0);
	glEnable(GL_DEPTH_TEST);
	
	vpMtx = cam->projMtx*cam->viewMtx;
	mvpMtx = vpMtx*modelMtx;
	glUniformMatrix4fv(progCur->u_mvpMtx,1,false,glm::value_ptr(mvpMtx));

	scene->Draw(this);

	UseProg(&progTex);
	texWhite.Bind();
	drawState = eDSDefault;
	CheckGLError("GLES2Renderer::RenderDepth", __FILE__, __LINE__);
}

void GLES2Renderer::DrawDebugScene()
{
	glDisable(GL_DEPTH_TEST);
	UseProg(&progTex);
	texWhite.Bind();
	SetColor(1,0.5,0.2,1);
	for(uint32_t i=0; i<debugBBox.size(); i++){
		debugBBox_t db = debugBBox[i];
		SetModelMtx(db.mtx);
		DrawBBox(db.bbox);
	}
	SetColor(0.5,0.5,1.0,1);
	for(uint32_t i=0;i<scene->lights.size(); i++){
		LightObject &l=*(scene->lights[i]);
		mat4 mtx(1);
		SetModelMtx(mtx);
		BoundingBox bbox(l.pos-l.radius,l.pos+l.radius);
		DrawBBox(&bbox);
		bbox = BoundingBox(l.pos-vec3(0.1),l.pos+vec3(0.1));
		DrawBBox(&bbox);
	}

	glEnable(GL_DEPTH_TEST);
	CheckGLError("GLES2Renderer::DrawDebugScene", __FILE__, __LINE__);
}

void GLES2Renderer::DrawBBox(BoundingBox *bbox)
{
	float r = bbox->max.x;
	float l = bbox->min.x;
	float u = bbox->max.y;
	float d = bbox->min.y;
	float f = bbox->max.z;
	float b = bbox->min.z;

	GLfloat vertices[] =
	{
		r, d, f,
		r, d, b,
		l, d, b,
		l, d, f,
		r, u, f,
		r, u, b,
		l, u, b,
		l, u, f
	};
	uint8_t inds[] =
	{
		0,1, 1,2, 2,3, 3,0,
		4,5, 5,6, 6,7, 7,4,
		0,4, 1,5, 2,6, 3,7
	};

	//glDisableVertexAttribArray(1);
	//glDisableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, vertices);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, inds);

	CheckGLError("GLES2Renderer::DrawBBox", __FILE__, __LINE__);
}

void GLES2Renderer::DrawCube()
{
	meshCube->Bind();
	meshCube->Draw();
	meshCube->Unbind();
	CheckGLError("GLES2Renderer::DrawCube", __FILE__, __LINE__);
}

void GLES2Renderer::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void GLES2Renderer::DrawModel(Model *mdl,const glm::mat4 &mtx)
{
	//TODO: render list from vbsp

	if(drawState!=eDSDepth && debug)
		debugBBox.push_back(debugBBox_t(&mdl->bbox,mtx));
	
	SetModelMtx(mtx);
	
	//TODO: vertexFormat
	mdl->vbo.Bind();
	/*glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,mdl->vertexStride,0);
	if(drawState!=eDSDepth){
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,mdl->vertexStride,(void*)12);
		glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,mdl->vertexStride,(void*)24);
	}*/
	vertAttrib_t &va = mdl->vertexFormat[0];
	glVertexAttribPointer(va.id,va.size,va.type,va.norm,va.stride,(void*)va.offset);
	if(drawState!=eDSDepth){
		for(int i=1; i<mdl->vertexFormat.size; i++){
			vertAttrib_t &va = mdl->vertexFormat[i];
			glVertexAttribPointer(va.id,va.size,va.type,va.norm,va.stride,(void*)va.offset);
		}
	}
	mdl->vbo.Unbind();

	for(int i=0;i<mdl->submeshes.size;i++){
		IMaterial *mat = mdl->materials[mdl->submeshes[i].mat].mat;
		if(drawState!=eDSDepth && mat){
			mat->Bind(this);
			if(mat->lit&&lighting){
				lighting->SetDirectional();
			}else{
				UseProg(&progTex);
			}
		}
		//else
		//	defaultMat->Bind(this);
		if(mdl->indexCount){
			mdl->ibo.Bind();
			glDrawElements(GL_TRIANGLES,mdl->submeshes[i].count,mdl->indexType,(void *)(mdl->submeshes[i].offs*mdl->indexSize));
			mdl->ibo.Unbind();
		}else{
			glDrawArrays(GL_TRIANGLES, mdl->submeshes[i].offs, mdl->submeshes[i].count);
		}
		
		//add lights
		if(drawState!=eDSDepth && mat && mat->lit&& lighting&& !scene->lights.empty()){
			lighting->AddLights(mtx,mdl->submeshes[i]);
		}
	}
}

void GLES2Renderer::UseProg(glslProg *p){
	progCur = p;
	if(!p)
		return;
	p->Use();

	glUniformMatrix4fv(progCur->u_mvpMtx,1,false,glm::value_ptr(mvpMtx));
	if(progCur->u_modelMtx!=-1)
		glUniformMatrix4fv(progCur->u_modelMtx,1,false,glm::value_ptr(modelMtx));
	if(progCur->u_cameraPos!=-1){
		glUniform3fv(progCur->u_cameraPos, 1, glm::value_ptr(camera->pos));
	}
}

void GLES2Renderer::SetScene(Scene *sc){
	scene=sc;
	if(lighting)
		lighting->SetScene(sc);
}

void GLES2Renderer::SetCamera(Camera *cam)
{
	camera = cam;
	if(lighting)
		lighting->SetCamera(cam);
	vpMtx = cam->projMtx*cam->viewMtx;
}

void GLES2Renderer::SetModelMtx(const glm::mat4 &mtx)
{
	modelMtx = mtx;
	mvpMtx = vpMtx*mtx;
	glUniformMatrix4fv(progCur->u_mvpMtx,1,false,glm::value_ptr(mvpMtx));
	if(progCur->u_modelMtx!=-1)
		glUniformMatrix4fv(progCur->u_modelMtx,1,false,glm::value_ptr(mtx));
}

void GLES2Renderer::UpdateMtx()
{
	if(camera)
		vpMtx = camera->projMtx*camera->viewMtx;
	mvpMtx = vpMtx*modelMtx;
	glUniformMatrix4fv(progCur->u_mvpMtx,1,false,glm::value_ptr(mvpMtx));
	if(progCur->u_modelMtx!=-1)
		glUniformMatrix4fv(progCur->u_modelMtx,1,false,glm::value_ptr(modelMtx));
}

void GLES2Renderer::ResetViewport(){
	glViewport(0,0,width,height);
}

void GLES2Renderer::Set2DMode(bool m){
	if(m){
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		UseProg(&progTex);
		glm::mat4 mtx(1.0f);
		mtx = glm::translate(mtx,glm::vec3(-1.0,-1.0,0.0));
		vpMtx = glm::scale(mtx,glm::vec3(2.0/aspect,2.0f,1.0f));
		mvpMtx = vpMtx;
		glUniformMatrix4fv(progCur->u_mvpMtx,1,false,glm::value_ptr(mvpMtx));
	}else{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		UpdateMtx();
	}
}

void GLES2Renderer::SetColor(float r, float g, float b, float a){
	glUniform4f(progCur->u_color,r,g,b,a);
}

void GLES2Renderer::AddButton(Button *b)
{
	buttons.push_back(b);
}

void GLES2Renderer::DrawText(const char *t,float x,float y,float s){
	if(!font)
		return;

	UseProg(&progTex);
	font->Print(t,x,1-y,s);
	texWhite.Bind();
}

void GLES2Renderer::DrawRect(float x, float y, float w, float h){
	if(!(flags&RENDERER_GUI)){
		EngineError("Renderer::DrawRect without RENDERER_GUI");
		return;
	}
	glm::mat4 mtx(1.0f);
	mtx = glm::translate(mtx,glm::vec3(x*2.0-(1-w),-(y*2.0-(1-h)),0));
	mtx = glm::scale(mtx,glm::vec3(w,h,1));
	glUniformMatrix4fv(progCur->u_mvpMtx,1,false,glm::value_ptr(mtx));

	vboQuad.Bind();
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,16,0);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,16,(void*)8);
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	vboQuad.Unbind();
}

void GLES2Renderer::DrawSkyBox(Texture *s){
	meshCube->Bind();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glm::mat4 skyboxViewMtx = camera->viewMtx;
	skyboxViewMtx[3] = glm::vec4(0,0,0,1);
	glm::mat4 mvpMtx = camera->projMtx * skyboxViewMtx;
	UseProg(&progSkybox);
	glUniformMatrix4fv(progSkybox.u_mvpMtx,1,GL_FALSE,glm::value_ptr(mvpMtx));
	s->Bind();
	meshCube->Draw();
	meshCube->Unbind();
}

void LogRenderInfo(){
	Log("%s %s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER));
	Log("%s\n",glGetString(GL_VERSION));
	Log("%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
	//Log("%s\n",glGetString(GL_EXTENSIONS));
	//GLExtensions::Init();
	
	const char *t = (const char*)glGetString(GL_EXTENSIONS);
	const char *end = 0;
	char ext[256]={0};
	int n=0;
	while(*t){
		end = strstr(t," ");
		if(end){
			strncpy(ext,t,end-t);
			ext[end-t]=0;
		}
		else
			break;
		//Log("ext %d: %s\n",n,ext);
		t=end+1;
		n++;
	}
	Log("Exts count: %d\n",n);
	
	int rb, gb, bb;
	glGetIntegerv(GL_RED_BITS, &rb);
	glGetIntegerv(GL_GREEN_BITS, &gb);
	glGetIntegerv(GL_BLUE_BITS, &bb);
	int bits=-1;
	glGetIntegerv(GL_ALPHA_BITS, &bits);
	Log( "Color bits: %d %d %d %d\n", rb,gb,bb,bits);
	glGetIntegerv(GL_DEPTH_BITS, &bits);
	Log( "DEPTH BITS %d\n", bits);
	glGetIntegerv(GL_STENCIL_BITS, &bits);
	Log( "STENCIL BITS %d\n", bits);

	int rfmt=0;
	int rtp=0;
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT,&rfmt);
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE,&rtp);
	Log("Color read format %X, type %X\n",rfmt,rtp);
	
	
}

