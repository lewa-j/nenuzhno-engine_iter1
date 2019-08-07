
#include "log.h"
#include "engine.h"
#include "game/IGame.h"
#include "system/config.h"
#include "graphics/gl_utils.h"
#include "graphics/gl_ext.h"
#include "graphics/glsl_prog.h"
#include "graphics/texture.h"
#include "graphics/vao.h"
#include "renderer/Model.h"
#include "renderer/mesh.h"
#include "renderer/camera.h"
#include "renderer/font.h"
#include "button.h"

#include "mdl_loader.h"

#include <mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/random.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::translate;
using glm::scale;
using glm::radians;

void VAOFromModel(Model *mdl, VertexArrayObject *out);
void VAOFromMDL(Model *mdl, VertexArrayObject *out,int a_weight,int a_bones);
void GenRandomFrame(mat4 *out, mat4 *base, mat4 *inv, Model *mdl);
void InterpFrames(float a,mat4 *f1, mat4 *f2, mat4 *out,int num);
void MeshFromModelSkeleton(Mesh *mesh, Model *mdl, mat4 *bonesMtx);
void UpdateSkeletonMesh(Mesh *mesh, Model *mdl, mat4 *bonesMtx);
void DrawModel(Model *mdl,VertexArrayObject *vao);
void DrawMesh(Mesh *mesh);

class skinGame: public IGame{
public:
	skinGame(){}
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "skinning";
	}
	void OnTouch(float tx, float ty, int ta, int tf);

	int width;
	int height;
	float aspect;

	ResourceManager resMan;
	Font *font;
	glslProg progTex;
	glslProg progSkin;
	int a_weight;
	int a_bones;
	int u_bonesMtx;

	Texture texWhite;
	Model *mdlTest;
	VertexArrayObject vaoTest;
	Camera camera;
	mat4 mvpMtx;
	mat4 modelMtx;
	mat4 *bonesBaseMtx;
	mat4 *bonesInvMtx;
	mat4 *bonesMtx;
	mat4 *frame1Mtx;
	mat4 *frame2Mtx;
	mat4 *skelMeshMtx;
	Mesh meshSkeleton;
	float curTime;
	int animIdx;
	
	Joystick joyL;
	Joystick joyM;
	Button bNext;
	Button bPrev;

	double oldTime;
};

IGame *CreateGame(){
	return new skinGame();
}

void skinGame::Created()
{
	srand(time(NULL));
	Log("skinning test Created()\n");

	GLExtensions::Init();

	resMan.Init();
	resMan.AddModelLoader(new MDLLoader(&resMan));

	progTex.CreateFromFile("generic","col_tex");
	progTex.u_mvpMtx = progTex.GetUniformLoc("u_mvpMtx");
	progTex.u_color = progTex.GetUniformLoc("u_color");

	progSkin.CreateFromFile("skinning","skinning");
	progSkin.u_mvpMtx = progSkin.GetUniformLoc("u_mvpMtx");
	progSkin.u_modelMtx = progSkin.GetUniformLoc("u_modelMtx");
	progSkin.u_color = progSkin.GetUniformLoc("u_color");
	u_bonesMtx = progSkin.GetUniformLoc("u_bonesMtx");
	a_weight = progSkin.GetAttribLoc("a_weight");
	a_bones = progSkin.GetAttribLoc("a_bones");

	CheckGLError("Created shaders", __FILE__, __LINE__);

	font = new Font();
	font->LoadBMFont("sansation",&resMan);

	uint8_t texWhiteData[1] = {255};
	texWhite.Create(1, 1);
	texWhite.Upload(0, GL_LUMINANCE, texWhiteData);
	glBindTexture(GL_TEXTURE_2D, 0);
	CheckGLError("Created textures", __FILE__, __LINE__);

	ConfigFile cfg;
	cfg.Load("config.txt");
	animIdx = cfg.GetInt("anim");
	modelMtx = scale(mat4(1),cfg.GetVec3("scale"));
		modelMtx = rotate(modelMtx,radians(-90.0f),vec3(1,0,0));

	mdlTest = resMan.GetModel(cfg["model"].c_str());
	if(cfg["model"]=="models/player.mdl"){
		mdlTest->materials.Resize(6);
		mdlTest->materials[0].mat=new TexMaterial(resMan.GetTexture("chell/chell_head_diffuse.vtf"));
		mdlTest->materials[1].mat=new TexMaterial(resMan.GetTexture("chell/gambler_eyes.vtf"));
		mdlTest->materials[2].mat=new TexMaterial(resMan.GetTexture("chell/gambler_eyes.vtf"));
		mdlTest->materials[3].mat=new TexMaterial(resMan.GetTexture("chell/chell_torso_diffuse.vtf"));
		mdlTest->materials[4].mat=new TexMaterial(resMan.GetTexture("chell/chell_legs_diffuse.vtf"));
		mdlTest->materials[5].mat=new TexMaterial(resMan.GetTexture("chell/chell_hair.vtf"));

		modelMtx = rotate(modelMtx,radians(90.0f),vec3(1,0,0));
	}
	if(cfg["model"]=="models/turret.mdl"){
		mdlTest->materials.Resize(4);
		mdlTest->materials[0].mat=new TexMaterial(resMan.GetTexture("turret/turret_frame02.vtf"));
		mdlTest->materials[1].mat=new TexMaterial(resMan.GetTexture("turret/turret_frame01.vtf"));
		mdlTest->materials[2].mat=new TexMaterial(resMan.GetTexture("turret/turret_casing.vtf"));
		mdlTest->materials[3].mat=new TexMaterial(resMan.GetTexture("turret/turret_eye.vtf"));
	}
	if(cfg["model"]=="models/headcrabclassic.mdl"){
		mdlTest->materials.Resize(1);
		mdlTest->materials[0].mat=new TexMaterial(resMan.GetTexture("headcrabsheet.vtf"));
	}
	if(cfg["model"]=="models/male_07.mdl"){
		mdlTest->materials.Resize(8);
		mdlTest->materials[2].mat=new TexMaterial(resMan.GetTexture("group03/mike_facemap.vtf"));
		mdlTest->materials[3].mat=new TexMaterial(resMan.GetTexture("group03/citizen_sheet.vtf"));
	}
	VAOFromMDL(mdlTest,&vaoTest,a_weight,a_bones);
	//mdlTest = resMan.GetModel("cube.nmf");
	//VAOFromModel(mdlTest,&vaoTest);

	bonesBaseMtx = new mat4[mdlTest->skeleton.size];
	bonesInvMtx = new mat4[mdlTest->skeleton.size];
	bonesMtx = new mat4[mdlTest->skeleton.size];
	frame1Mtx = new mat4[mdlTest->skeleton.size];
	frame2Mtx = new mat4[mdlTest->skeleton.size];
	skelMeshMtx = new mat4[mdlTest->skeleton.size];
	for(int i=0;i<mdlTest->skeleton.size;i++){
		bonesBaseMtx[i] = glm::mat4_cast(mdlTest->skeleton[i].rot);
		bonesBaseMtx[i][3] = vec4(mdlTest->skeleton[i].pos,1.0f);
		if(mdlTest->skeleton[i].parent!=-1)
			bonesBaseMtx[i] = bonesBaseMtx[mdlTest->skeleton[i].parent] * bonesBaseMtx[i];
		bonesInvMtx[i] = glm::inverse(bonesBaseMtx[i]);
		bonesMtx[i] = mat4(1);
	}
	MeshFromModelSkeleton(&meshSkeleton,mdlTest,bonesBaseMtx);
	curTime=0;

	glClearColor(0.3,0.3,0.3,1.0);

	camera.pos = vec3(0,1.5,1.6);
	camera.rot = vec3(10,0,0);
	camera.UpdateView();

	joyL = Joystick(0,0.5,0.5,0.5);
	joyM = Joystick(0.5,0.5,0.5,0.5);
	bNext = Button(0,0,0.1,0.1);
	bPrev = Button(0.2,0,0.1,0.1);

	oldTime = GetTime();
}

void GenRandomFrame(mat4 *out, mat4 *base, mat4 *inv, Model *mdl){
	for(int i=0;i<mdl->skeleton.size;i++){
		if(i!=0){
			out[i] = base[i]*glm::mat4_cast(glm::quat(glm::linearRand(vec3(-0.2f),vec3(0.2f))))*inv[i];
			if(mdl->skeleton[i].parent!=-1){
				out[i] = out[mdl->skeleton[i].parent] * out[i];
			}
		}
	}
}
/*
void InterpFramesMtx(float a,mat4 *f1, mat4 *f2, mat4 *out,int num){
	for(int i=0;i<num;i++){
		out[i] = glm::mix(f1[i],f2[i],a);
	}
}
*/
void SetupFrame(mat4 *out, Animation_t& anim, float frame){
	int a = floor(frame);
	int b = ceil(frame);
	if(b>=anim.frames.size)
		b = anim.frames.size-1;
	float s = frame-a;
	AnimFrame_t &fr1 = anim.frames[a];
	AnimFrame_t &fr2 = anim.frames[b];
	for(int i=0;i<fr1.bones.size;i++){
		out[i] = glm::mat4_cast(glm::mix(fr1.bones[i].rot,fr2.bones[i].rot,s));
		out[i][3] = vec4(glm::mix(fr1.bones[i].pos,fr2.bones[i].pos,s),1.0f);
		if(fr1.bones[i].parent!=-1)
			out[i] = out[fr1.bones[i].parent] * out[i];
	}
}

void skinGame::Changed(int w, int h){
	width = w;
	height = h;
	aspect = w/(float)h;
	glViewport(0,0,w,h);
	camera.UpdateProj(80,aspect,0.1,100);
	CheckGLError("Changed", __FILE__, __LINE__);
}

void skinGame::Draw()
{
	double startTime = GetTime();
	float deltaTime = (startTime-oldTime);
	oldTime = startTime;

	if(bNext.pressed){
		bNext.pressed=false;
		if(mdlTest->animations.size)
			animIdx=(animIdx+1)%mdlTest->animations.size;
		curTime=0;
		/*GenRandomFrame(bonesMtx,bonesBaseMtx,bonesInvMtx,mdlTest);
		for(int i=0;i<mdlTest->skeleton.size;i++){
			skelMeshMtx[i] = bonesMtx[i]*bonesBaseMtx[i];
		}
		UpdateSkeletonMesh(&meshSkeleton,mdlTest,skelMeshMtx);*/
	}
	if(bPrev.pressed){
		bPrev.pressed=false;
		if(mdlTest->animations.size)
			animIdx=(animIdx-1+mdlTest->animations.size)%mdlTest->animations.size;
		curTime=0;
	}

	curTime+=deltaTime*0.5;
/*
	if(curTime>=1){
		memcpy(frame1Mtx,frame2Mtx,sizeof(mat4)*mdlTest->skeleton.size);
		GenRandomFrame(frame2Mtx,bonesBaseMtx,bonesInvMtx,mdlTest);
		curTime = 0;
	}
	InterpFramesMtx(curTime,frame1Mtx,frame2Mtx,bonesMtx,mdlTest->skeleton.size);
	for(int i=0;i<mdlTest->skeleton.size;i++){
		skelMeshMtx[i] = bonesMtx[i]*bonesBaseMtx[i];
	}
	UpdateSkeletonMesh(&meshSkeleton,mdlTest,skelMeshMtx);
*/
	float curFrame = 0;
	if(mdlTest->animations.size){
		curFrame = curTime*mdlTest->animations[animIdx].fps;
		while(curFrame>=mdlTest->animations[animIdx].frames.size)
			curFrame -= mdlTest->animations[animIdx].frames.size;
		SetupFrame(skelMeshMtx,mdlTest->animations[animIdx],curFrame);
		UpdateSkeletonMesh(&meshSkeleton,mdlTest,skelMeshMtx);
		for(int i=0;i<mdlTest->skeleton.size;i++){
			bonesMtx[i] = skelMeshMtx[i]*bonesInvMtx[i];
		}
	}

	camera.pos += glm::inverse(mat3(camera.viewMtx))*vec3(joyM.vel.x,0,-joyM.vel.y*aspect)*5.0f*deltaTime;
	camera.rot += vec3(-joyL.vel.y*aspect,-joyL.vel.x,0)*90.0f*deltaTime;
	camera.UpdateView();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	mvpMtx = camera.projMtx*camera.viewMtx*modelMtx;

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	texWhite.Bind();
	glslProg *progCur = &progSkin;
	//progCur = &progTex;
	progCur->Use();
	progCur->UniformMat4(progCur->u_mvpMtx,mvpMtx);
	if(progCur->u_modelMtx>=0)
		progCur->UniformMat4(progCur->u_modelMtx,modelMtx);
	if(progCur->u_color>=0)
		progCur->UniformVec4(progCur->u_color,vec4(1));
	if(u_bonesMtx>=0){
		glUniformMatrix4fv(u_bonesMtx,mdlTest->skeleton.size,false,glm::value_ptr(bonesMtx[0]));
	}

	DrawModel(mdlTest,&vaoTest);

	mvpMtx = camera.projMtx*camera.viewMtx*modelMtx;
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnableVertexAttribArray(0);
	progCur = &progTex;
	progCur->Use();
	progCur->UniformMat4(progCur->u_mvpMtx,mvpMtx);
	if(progCur->u_color>=0)
		progCur->UniformVec4(progCur->u_color,vec4(1));
	texWhite.Bind();
	DrawMesh(&meshSkeleton);

	glDisableVertexAttribArray(1);

	glEnableVertexAttribArray(2);

	mat4 mtx2D = mat4(1);
	mtx2D = glm::translate(mtx2D,glm::vec3(-1,-1,0));
	mtx2D = glm::scale(mtx2D,glm::vec3(2.0f,2.0f*aspect,0));
	progCur->UniformMat4(progCur->u_mvpMtx,mtx2D);
	font->Print("Next",bNext.x,(0.95-bNext.y)/aspect,0.5/aspect);
	font->Print("Prev",bPrev.x,(0.95-bPrev.y)/aspect,0.5/aspect);
	char temp[256]={0};
	if(mdlTest->animations.size){
		snprintf(temp,256,"animation %d/%d: %s\n",animIdx,mdlTest->animations.size,mdlTest->animations[animIdx].name.c_str());
		font->Print(temp,0,0.1/aspect,0.5/aspect);
		snprintf(temp,256,"frame %d/%d\n",(int)curFrame,mdlTest->animations[animIdx].frames.size);
		font->Print(temp,0,0.15/aspect,0.5/aspect);
	}
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(2);
	glUseProgram(0);
	CheckGLError("Draw", __FILE__, __LINE__);
}

void skinGame::OnTouch(float tx, float ty, int ta, int tf){
	float x = tx/width;
#ifdef ANDROID
	float y = (ty-64)/height;
#else
	float y = ty/height;
#endif
	if(ta==0){
		bNext.Hit(x,y);
		bPrev.Hit(x,y);
		joyL.Hit(x,y,tf);
		joyM.Hit(x,y,tf);
	}else if(ta==1){
		joyL.Release(tf);
		joyM.Release(tf);
	}else if(ta==2){
		joyL.Move(x,y,tf);
		joyM.Move(x,y,tf);
	}
}

void VAOFromModel(Model *mdl, VertexArrayObject *out){
	out->Create();
	out->Bind();
	//TODO separate function for vbo attribs
	mdl->vbo.Bind();
	for(int i=0; i<mdl->vertexFormat.size; i++){
		vertAttrib_t &va = mdl->vertexFormat[i];
		out->SetAttribute(va.id,va.size,va.type,va.norm,va.stride,(void*)va.offset);
	}
	mdl->vbo.Unbind();
	if(mdl->indexCount){
		mdl->ibo.Bind();
	}
	out->Unbind();
}

void VAOFromMDL(Model *mdl, VertexArrayObject *out,int a_weight,int a_bones){
	out->Create();
	out->Bind();
	mdl->vbo.Bind();

	for(int i=0; i<mdl->vertexFormat.size; i++){
		vertAttrib_t &va = mdl->vertexFormat[i];
		int id = va.id;
		if(va.id == 3)
			id = a_weight;
		else if(va.id == 4)
			id = a_bones;
		out->SetAttribute(id,va.size,va.type,va.norm,va.stride,(void*)va.offset);
	}
	mdl->vbo.Unbind();
	if(mdl->indexCount){
		mdl->ibo.Bind();
	}
	out->Unbind();
}

void MeshFromModelSkeleton(Mesh *mesh, Model *mdl,mat4 *bonesMtx){
	 int nv = mdl->skeleton.size*2;
	 float *verts = new float[nv*3];
	 *mesh = Mesh(verts,nv,GL_LINES);
	 UpdateSkeletonMesh(mesh,mdl,bonesMtx);
}

void UpdateSkeletonMesh(Mesh *mesh, Model *mdl, mat4 *bonesMtx){
	for(int i=0;i<mdl->skeleton.size;i++){
		glm::vec3 *v = (glm::vec3*)(mesh->verts+i*6);
		v[0] = vec3(bonesMtx[i]*vec4(0,0,0,1));
		int p = mdl->skeleton[i].parent;
		v[1] = p!=-1 ? vec3(bonesMtx[p]*vec4(0,0,0,1)) : v[0];
	}
}

void DrawModel(Model *mdl,VertexArrayObject *vao){
	/*
	mdl->vbo.Bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,mdl->vertexStride,0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,mdl->vertexStride,(void*)12);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,mdl->vertexStride,(void*)24);
	mdl->vbo.Unbind();
	*/
	vao->Bind();

	for(int i=0; i<mdl->submeshes.size; i++){
		if(mdl->materials.size&&mdl->materials[mdl->submeshes[i].mat].mat)
			mdl->materials[mdl->submeshes[i].mat].mat->Bind(0);
		if(mdl->indexCount){
			//mdl->ibo.Bind();
			glDrawElements(GL_TRIANGLES,mdl->submeshes[i].count,mdl->indexType,(void*)(mdl->submeshes[i].offs*mdl->indexSize));
			//mdl->ibo.Unbind();
		}else{
			glDrawArrays(GL_TRIANGLES, mdl->submeshes[i].offs, mdl->submeshes[i].count);
		}
	}
	//glDrawArrays(GL_POINTS, 0, mdl->vertexCount);
	vao->Unbind();
}

void DrawMesh(Mesh *mesh){
	mesh->Bind();
	mesh->Draw();
	mesh->Unbind();
}

Button::Button(float nx, float ny, float nw, float nh, bool adjust):
			x(nx),y(ny),w(nw),h(nh),text(0),active(true),pressed(false)
{
}
void Button::Update(){}
bool Button::SetUniform(int loc){return false;}
