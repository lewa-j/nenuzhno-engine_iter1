
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;

#define GLEW_STATIC 1
#define GLEW_NO_GLU 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "log.h"
#include "game/IGame.h"
#include "system/FileSystem.h"

void Created();
void Changed(int w, int h);
void Draw();
void OnKey(int key, int scancode, int action, int mods);
void OnTouch(float tx, float ty, int ta, int tf);
void OnMouseMove(float mx, float my);
void OnScroll(float sx, float sy);

GLFWwindow *window;
void EngineSwapBuffers()
{
	glfwSwapBuffers(window);
}

void EngineError(const char *message)
{
	LOG("EngineError!!! (%s)\n", message);
	exit(-1);
}

void EngineQuit(){
	glfwSetWindowShouldClose(window, 1);
}

void EnableCursor(bool state)
{
	if(state)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

double GetTime()
{
	return glfwGetTime();
}

void FramebufferSizeCallback(GLFWwindow *win, int w, int h)
{
	Changed(w,h);
	Draw();
	glfwSwapBuffers(win);
}

void error_callback(int error, const char* description)
{
	Log("glfw error %d : %s\n",error,description);
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods)
{
	//if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
	//	glfwSetWindowShouldClose(win, 1);

	OnKey(key, scancode, action, mods);
}

int cursorPosX = 0;
int cursorPosY = 0;
int cursorState = 1;

void cursorPos_callback(GLFWwindow* win, double x, double y)
{
	//LOG("cursor %f %f\n", x, y);
	cursorPosX = x;
	cursorPosY = y;
	if(!cursorState)
		OnTouch(cursorPosX,cursorPosY,2,0);
	OnMouseMove(x,y);
}

void mouseButton_callback(GLFWwindow* win, int button, int action, int mods)
{
	cursorState = (action+1)%2;
	//LOG("mouse button %d %d %d\n", button, action, mods);
	OnTouch(cursorPosX,cursorPosY,cursorState,0);
}

void scroll_callback(GLFWwindow* win, double xoffset, double yoffset)
{
	OnScroll(xoffset, yoffset);
}

int main(int argc, char* argv[])
{
	bool fullscreen = false;
	bool gles = false;
	bool gl_core = false;
	int glv[2] = {2,0};
	bool vsync = false;
	const char *cgamedir=0;
	int msaa = 0;
	GLuint scrWidth = 800;
	GLuint scrHeight = 480;

	for(int i=1; i<argc; i++)
	{
		if(!strcmp(argv[i],"-fullscreen"))
			fullscreen = true;
		else if(!strcmp(argv[i],"-msaa"))
			msaa = atoi(argv[++i]);
		else if(!strcmp(argv[i],"-w"))
			scrWidth = atoi(argv[++i]);
		else if(!strcmp(argv[i],"-h"))
			scrHeight = atoi(argv[++i]);
		else if(!strcmp(argv[i],"-gles")){
			gles = true;
			sscanf(argv[++i],"%d.%d",glv,glv+1);
		}else if(!strcmp(argv[i],"-glcore")){
			gl_core = true;
			sscanf(argv[++i],"%d.%d",glv,glv+1);
		}else if(!strcmp(argv[i],"-vsync"))
			vsync = true;
		else if(!strcmp(argv[i],"-game")){
			cgamedir = argv[++i];
		}else if(!strcmp(argv[i],"-help")){
			cout << "nenuzhno-engine\nArguments:\n"
				<<"-fullscreen - Enable fullscreen mode\n"
				<<"-msaa <samples> - Enable multisampling with <samples>\n"
				<<"-w <width> -h <height> - Screen size\n"
				<<"-glcore x.x - Init core OpenGL context"
				<<"-gles x.x (experimental) - Init gles context\n"
				<<"-vsync - Enable vertical sync\n"
				<<"-game <gamedir> - Set gamdir\n"
				<<"-help - Print this message\n";
		}
	}

	glfwSetErrorCallback(error_callback);
	if(!glfwInit())
	{
		cerr<<"Init error\n";
		return -1;
	}

	if(gl_core||gles){
		if(gl_core){
			Log("Init core gl context (%d.%d)\n",glv[0],glv[1]);
			if(glv[0]>=3&&glv[1]>=2)
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		}else{
			Log("Init gles context (%d.%d)\n",glv[0],glv[1]);
			glfwWindowHint(GLFW_CLIENT_API,GLFW_OPENGL_ES_API);
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glv[0]);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glv[1]);
	}

	glfwWindowHint(GLFW_SAMPLES, msaa);

	if(!fullscreen)
		window = glfwCreateWindow(scrWidth,scrHeight,"Nenuzhno engine",NULL,NULL);
	else
		window = glfwCreateWindow(scrWidth,scrHeight,"Nenuzhno engine",glfwGetPrimaryMonitor(),NULL);

	if(!window){
		Log("CreateWindow error\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	if(!vsync)
		glfwSwapInterval(0);
	else
		glfwSwapInterval(1);

	GLenum err = glewInit();
	if(err != GLEW_OK){
		Log("GLEW Error: %s\n", glewGetErrorString(err));
	}
	glfwSetFramebufferSizeCallback(window,FramebufferSizeCallback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursorPos_callback);
	glfwSetMouseButtonCallback(window, mouseButton_callback);
	glfwSetScrollCallback(window, scroll_callback);

	Log("glfw %s\n", glfwGetVersionString());
	GameInit();
	if(!cgamedir && pGame)
		cgamedir = pGame->GetGamedir();

	g_fs.Init(cgamedir);
	LogInit();
	//InputSystem Init
	Created();
	FramebufferSizeCallback(window,scrWidth,scrHeight);//Changed

	while(!glfwWindowShouldClose(window))
	{
		//InputSystem Update
		Draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Log("Done\n");
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
