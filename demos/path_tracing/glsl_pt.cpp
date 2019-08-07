
#include "log.h"
#include "engine.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/glsl_prog.h"
#include "graphics/ArrayBuffer.h"
#include "graphics/fbo.h"
#include "system/FileSystem.h"
#include "game/IGame.h"
#include "renderer/font.h"
#include "resource/ResourceManager.h"

#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>

#include <string.h>
#include <cstdlib>
#include <time.h>
#include <math.h>

class rayGame: public IGame{
public:
	rayGame();
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "pt";
	}
	void OnTouch(float tx, float ty, int ta, int tf);
};

IGame *CreateGame(){
	return new rayGame();
}

char quadVert[]=
"#version 100\n"
"precision highp float;\n"
"attribute vec4 a_position;\n"
"varying vec2 v_uv;\n"
"void main(){\n"
"	gl_Position = a_position;\n"
"	v_uv = a_position.xy*0.5+0.5;\n"
"}\n";

char quadFrag[]=
"#version 100\n"
"precision highp float;\n"
"varying vec2 v_uv;\n"
"uniform sampler2D u_tex;\n"
"void main(){\n"
"	gl_FragColor = texture2D(u_tex,v_uv);\n"
"}";

char quadTraceVert[]=
"#version 100\n"
"precision highp float;\n"
"attribute vec4 a_position;\n"
"varying vec2 v_uv;\n"
"void main(){\n"
"	gl_Position = a_position;\n"
"	v_uv = a_position.xy;\n"
"}\n";

char quadTraceFrag[]=
"#version 100\n"
"precision highp float;\n"
"varying vec2 v_uv;\n"
"uniform float u_textureWeight;\n"
"uniform sampler2D texture;\n"
"uniform sampler2D u_worldTex;\n"
"uniform float u_timeSinceStart;\n"
"uniform int u_numCubes;\n"
"const float glossiness = 0.8;\n"
"vec2 intersectCube(vec3 origin, vec3 ray, vec3 cubeMin, vec3 cubeMax){\n"
"   vec3 tMin = (cubeMin - origin) / ray;\n"
"   vec3 tMax = (cubeMax - origin) / ray;\n"
"   vec3 t1 = min(tMin, tMax);\n"
"   vec3 t2 = max(tMin, tMax);\n"
"   float tNear = max(max(t1.x, t1.y), t1.z);\n"
"   float tFar = min(min(t2.x, t2.y), t2.z);\n"
"   return vec2(tNear, tFar);\n"
"}\n"
"vec3 normalForCube(vec3 hit, vec3 cubeMin, vec3 cubeMax){\n"
"   if(hit.x < cubeMin.x + 0.0001) return vec3(-1.0, 0.0, 0.0);\n"
"   else if(hit.x > cubeMax.x - 0.0001) return vec3(1.0, 0.0, 0.0);\n"
"   else if(hit.y < cubeMin.y + 0.0001) return vec3(0.0, -1.0, 0.0);\n"
"   else if(hit.y > cubeMax.y - 0.0001) return vec3(0.0, 1.0, 0.0);\n"
"   else if(hit.z < cubeMin.z + 0.0001) return vec3(0.0, 0.0, -1.0);\n"
"   else return vec3(0.0, 0.0, 1.0);\n"
"}\n"
"float random(vec3 scale, float seed){\n"
"   return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) * 43758.5453 + seed);\n"
"}\n"
"vec3 cosineWeightedDirection(float seed, vec3 normal){\n"
"   float u = random(vec3(12.9898, 78.233, 151.7182), seed);\n"
"   float v = random(vec3(63.7264, 10.873, 623.6736), seed);\n"
"   float r = sqrt(u);\n"
"   float angle = 6.283185307179586 * v;\n"
// compute basis from normal
"   vec3 sdir, tdir;\n"
"   if (abs(normal.x)<.5){\n"
"     sdir = cross(normal, vec3(1,0,0));\n"
"   }else{\n"
"     sdir = cross(normal, vec3(0,1,0));\n"
"   }\n"
"   tdir = cross(normal, sdir);\n"
"   return r*cos(angle)*sdir + r*sin(angle)*tdir + sqrt(1.-u)*normal;\n"
"}\n"
"vec3 uniformlyRandomDirection(float seed){\n"
"	float u = random(vec3(12.9898, 78.233, 151.7182), seed);\n"
"	float v = random(vec3(63.7264, 10.873, 623.6736), seed);\n"
"	float z = 1.0 - 2.0 * u;\n"
"	float r = sqrt(1.0 - z * z);\n"
"	float angle = 6.283185307179586 * v;\n"
"	return vec3(r * cos(angle), r * sin(angle), z);\n"
"}\n"
"vec3 uniformlyRandomVector(float seed){\n"
"	return uniformlyRandomDirection(seed) * sqrt(random(vec3(36.7539, 50.3658, 306.2759), seed));\n"
"}\n"
"float shadow(vec3 origin, vec3 ray) {\n"
"	for(int c = 0; c<u_numCubes;c++){\n"
"		vec3 cubemin = texture2D(u_worldTex,vec2(0,float(c)/float(u_numCubes-1))).xyz;\n"
"		vec3 cubemax = texture2D(u_worldTex,vec2(1,float(c)/float(u_numCubes-1))).xyz;\n"
"		vec2 tCube1 = intersectCube(origin, ray, cubemin,cubemax);\n"
"		if(tCube1.x > 0.0 && tCube1.x < 1.0 && tCube1.x < tCube1.y)\n"
"			return 0.0;\n"
"	}\n"
"	return 1.0;\n"
"}\n"
//"vec3 cube1min = vec3(-0.2,-0.2,-0.2);\n"
//"vec3 cube1max = vec3(0.2,0.2,0.2);\n"
//texture2D(u_worldTex,vec2(0,0)).xyz, texture2D(u_worldTex,vec2(0,0)).xyz
"vec3 calculateColor(vec3 origin, vec3 ray, vec3 light,float seed){\n"
"	vec3 colorMask = vec3(1.0);\n"
"	vec3 result = vec3(0.0);\n"
"   for(int bounce = 0; bounce < 4; bounce++){\n"
"		float t = 9999.9;\n"
"		vec3 normal;\n"
"		vec2 tRoom = intersectCube(origin, ray, vec3(-1.5,-1.0,-1.0), vec3(1.0));\n"
"		if(tRoom.x < tRoom.y) t = tRoom.y;\n"
"		vec3 hit = origin + ray * t;\n"
"		for(int c = 0; c<u_numCubes;c++){\n"
"			vec3 cubemin = texture2D(u_worldTex,vec2(0,float(c)/float(u_numCubes-1))).xyz;\n"
"			vec3 cubemax = texture2D(u_worldTex,vec2(1,float(c)/float(u_numCubes-1))).xyz;\n"
"			vec2 tCube1 = intersectCube(origin, ray, cubemin,cubemax);\n"
"			if(tCube1.x > 0.0 && tCube1.x < tCube1.y && tCube1.x < t){\n"
"				t = tCube1.x;\n"
"				hit = origin + ray * t;\n"
"				normal = normalForCube(hit, cubemin,cubemax);\n"
"			}\n"
"		}\n"
"		vec3 surfaceColor = vec3(0.75);\n"
"		if(t == tRoom.y){\n"
"			normal = -normalForCube(hit, vec3(-1.5,-1.0,-1.0), vec3(1.0));\n"
"			if(hit.x < -1.4999) surfaceColor = vec3(1.0, 0.3, 0.1);\n" // red
"			else if(hit.x > 0.9999) surfaceColor = vec3(0.3, 1.0, 0.1);\n" // green
"			ray = cosineWeightedDirection(seed + float(bounce), normal);\n"
//"			ray = reflect(ray, normal);\n"
//"			ray = normalize(reflect(ray, normal)) + uniformlyRandomVector(seed + float(bounce)) * glossiness;\n"
"		}else if(t == 9999.9){\n"
"			break;\n"
"		}else{\n"
//"			if(t == tCube1.x && tCube1.x < tCube1.y)"
//"				normal = normalForCube(hit, texture2D(u_worldTex,vec2(0,0)).xyz,texture2D(u_worldTex,vec2(1,0)).xyz);\n"
//"			ray = reflect(ray, normal);\n"
"			ray = normalize(reflect(ray, normal)) + uniformlyRandomVector(seed + float(bounce)) * glossiness;\n"
"			surfaceColor = vec3(0.5, 0.5, 0.9);\n"
"		}\n"
"		vec3 toLight = light - hit;\n"
"		float diffuse = max(0.0, dot(normalize(toLight), normal));\n"
"		float shadowIntensity = shadow(hit + normal * 0.0001, toLight);\n"
"		colorMask *= surfaceColor;\n"
"		result += colorMask*diffuse*0.5*shadowIntensity;\n"
"		origin = hit;\n"
"	}\n"
"	return result;\n"
"}\n"
"void main(){\n"
"	vec3 dir = normalize(vec3(v_uv,-1.0));\n"
"	vec3 newLight = vec3(-1.4,0.1,-0.1) + uniformlyRandomVector(u_timeSinceStart - 53.0) * 0.1;\n"
"	vec3 sample1 = calculateColor(vec3(-0.2,0.15,1.5), dir, newLight,u_timeSinceStart);\n"
"	newLight = vec3(-1.4,0.1,-0.1) + uniformlyRandomVector(u_timeSinceStart - 27.4) * 0.1;\n"
"	sample1 = 0.5*(sample1+calculateColor(vec3(-0.2,0.15,1.5), dir, newLight,u_timeSinceStart+65.3));\n"
"	vec3 textureCol = texture2D(texture, v_uv*0.5+0.5).rgb;\n"
"	gl_FragColor = vec4(mix(sample1, textureCol, u_textureWeight), 1.0);\n"
"}\n";

float verts[]={
	-1,-1,0,
	-1,1,0,
	1,1,0,
	1,-1,0,
	0.5f,-0.25f,0
};

float world[]={
	-0.2,-0.2,-0.2, 0.2,0.2,0.2,
	-1.5,-1.0,-1.0, -1.2,-0.2,1.0,
	-1.5,-0.2,-1.0, -1.2,0.2,-0.2,
	-1.5,-0.2,0.2, -1.2,0.2,1.0,
	-1.5,0.2,-1.0, -1.2,1.0,1.0
};

glslProg progTex;
glslProg progQuad;
glslProg progQuadTrace;
GLuint u_textureWeight;
GLuint u_timeSinceStart;
GLuint u_worldTex;
GLuint u_numCubes;

VertexBufferObject vbo;
FrameBufferObject fbo;
Texture textures[2];
Texture texWhite;

int texSize = 512;

Texture worldTex;
int scrW;
int scrH;
bool needRedraw = true;
int samples = 0;
int maxSamples = 32;

ResourceManager g_resMan;
Font font;

double oldTime;
float deltaTime;
int fps;
float curTime;
int curFrames;

void flip_tex(Texture *val)
{
	int t = val[0].id;
	val[0].id = val[1].id;
	val[1].id = t;
}

rayGame::rayGame()
{
	
}

void rayGame::Created()
{
	g_resMan.Init();
	font.LoadBMFont("sansation",&g_resMan);
	
	Log("%s\n",glGetString(GL_VERSION));
	
	/*g_fs.WriteAll("shaders/quadTrace.vs",quadTraceVert);
	g_fs.WriteAll("shaders/quadTrace.fs",quadTraceFrag);
	g_fs.WriteAll("shaders/quad.vs",quadVert);
	g_fs.WriteAll("shaders/quad.fs",quadFrag);
	*/
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	/*
	progQuad = glslProg(quadVert, quadFrag);
	CheckGLError("Create progQuad", __FILE__, __LINE__);
	progQuadTrace = glslProg(quadTraceVert, quadTraceFrag);
	CheckGLError("Create progQuadTrace", __FILE__, __LINE__);*/
	progTex.CreateFromFile("tex","tex");
	progTex.u_mvpMtx = progTex.GetUniformLoc("u_mvpMtx");
	progQuad.CreateFromFile("quad","quad");
	progQuadTrace.CreateFromFile("quadTrace","quadTrace");
	u_textureWeight = progQuadTrace.GetUniformLoc("u_textureWeight");
	u_timeSinceStart = progQuadTrace.GetUniformLoc("u_timeSinceStart");
	u_worldTex = progQuadTrace.GetUniformLoc("u_worldTex");
	u_numCubes = progQuadTrace.GetUniformLoc("u_numCubes");
	CheckGLError("CreatePrograms", __FILE__, __LINE__);
	
	progQuadTrace.Use();
	glUniform1i(u_worldTex,1);
	glUniform1i(u_numCubes,5);
	
	vbo.Create();
	vbo.Upload(5*3*4,verts);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	
	for(int i=0;i<2;i++){
		textures[i].Create(texSize,texSize);
		textures[i].Bind();
			
		//glPixelStorei(GL_UNPACK_ALIGNMENT,4);
		//glPixelStorei(GL_PACK_ALIGNMENT,1);
		
		textures[i].SetFilter(GL_NEAREST,GL_LINEAR);
		
		//glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,texSize,texSize,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
		textures[i].Upload(0, GL_RGB, NULL);
	}
	
	worldTex.Create(2,5);
	//glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,2,5,0,GL_RGB,GL_FLOAT,world);
	worldTex.Upload(0,GL_RGB32F,GL_RGB,GL_FLOAT,(GLubyte*)world);
	
	GLubyte whitePix[]={255,255,255};
	texWhite.Create(1,1);
	texWhite.Upload(0,GL_RGB,whitePix);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	fbo.Create();
	//fbo.CreateTexture(texSize, texSize, GL_LINEAR);
//	glBindFramebuffer(GL_FRAMEBUFFER,fbo);
//	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,textures[0],0);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	
	CheckGLError("Created", __FILE__, __LINE__);
	srand(time(0));
	
	oldTime = GetTime();
	curTime=0;
	curFrames=0;
}

void ResizeTextures(int r){
	texSize = r;
	textures[0].Bind();
	textures[0].Upload(0, r, r, NULL);
	textures[1].Bind();
	textures[1].Upload(0, r, r, NULL);
}

void rayGame::Changed(int w, int h){
//	scrW = w;
//	scrH = h;
	scrW = scrH = fmin(w, h);
	glViewport(0, 0, scrW, scrH);
	samples = 0;
	needRedraw = true;
	ResizeTextures(scrH);
}

void Update(){
}

void rayGame::Draw(){
	
	double startTime = GetTime();
	float deltaTime = (startTime-oldTime);
	oldTime = startTime;
	curTime+=deltaTime;
	if(curTime>=1){
		curTime-=1;
		fps = curFrames;
		curFrames=0;
	}
	curFrames++;
	vbo.Bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,12,0);
	//CheckGLError("Draw1", __FILE__, __LINE__);
	
	while(needRedraw){
	//if(needRedraw){
		progQuadTrace.Use();
		
		glUniform1f(u_timeSinceStart,(float)rand()/9276714.73f);
		glUniform1f(u_textureWeight,(float)samples/(samples+1));
		//CheckGLError("Draw2", __FILE__, __LINE__);
		
		glActiveTexture(GL_TEXTURE1);
		worldTex.Bind();
		glActiveTexture(GL_TEXTURE0);
		textures[0].Bind();
		//CheckGLError("Draw3", __FILE__, __LINE__);

		fbo.Bind();
		//CheckGLError("Draw4", __FILE__, __LINE__);
		glViewport(0,0,texSize,texSize);
		//glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,textures[1],0);
		fbo.AttachTexture(&textures[1]);

		//glClear(GL_COLOR_BUFFER_BIT);
		
		glDrawArrays(GL_TRIANGLE_FAN,0,4);
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		
		glBindTexture(GL_TEXTURE_2D,0);

		flip_tex(textures);
		samples++;
	
		if(samples>=maxSamples)
			needRedraw=false;
	}
	//Log("Draw %d\n", __LINE__);
//	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glViewport(0,0,scrW,scrH);
	
	progQuad.Use();
	textures[0].Bind();
	//
	//worldTex.Bind();
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	
	glBindTexture(GL_TEXTURE_2D,0);

	
	progTex.Use();
	glm::mat4 mtx(1.0f);
	mtx = glm::translate(mtx,glm::vec3(-1.0,-1.0,0.0));
	float aspect = 1;//TODO
	mtx = glm::scale(mtx,glm::vec3(2.0/aspect,2.0f,1.0f));
	glUniformMatrix4fv(progTex.u_mvpMtx,1,false,glm::value_ptr(mtx));
	
	glActiveTexture(GL_TEXTURE0);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);
	
	char t[256];
	snprintf(t,256,"fps: %d\n""res: %dx%d\n""samples: %d",fps,texSize,texSize,samples);
	font.Print(t,0.02,0.12,0.5);
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(2);
	
	needRedraw = true;
	samples=0;
	
	CheckGLError("Draw", __FILE__, __LINE__);
}

void rayGame::OnTouch(float tx, float ty, int ta, int tf){
	
	samples = 0;
	needRedraw = true;
}
