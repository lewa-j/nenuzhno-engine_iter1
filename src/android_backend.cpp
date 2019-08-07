
#include <jni.h>

void Created();
void Changed(int w, int h);
void Draw();
void OnTouch(float tx, float ty, int ta, int tf);
void OnKey(int key, int scancode, int action, int mods);

#include "log.h"
#include "game/IGame.h"
#include "system/FileSystem.h"

#include "stdlib.h"
void EngineError(const char *message)
{
	LOG("EngineError!!! (%s)\n", message);
	exit(-1);
}

void EngineQuit(){
	Log("EngineQuit()\n");
	exit(0);
}

#include <EGL/egl.h>
void EngineSwapBuffers()
{
	eglSwapBuffers(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));
}

#include <time.h>
double GetTime()
{
	timeval tp;
	//timezone tzp;
	static int secbase;

	//gettimeofday(&tp, &tzp);
	gettimeofday(&tp, 0);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000000.0;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
}

void EnableCursor(bool s)
{
	//nothing
}

//#define JNI_PACK Java_ru_lewa_1j_ndkglestest1_GLJNILib_
#define JNI_PACK(s) Java_ru_lewa_1j_nenuzhno_engine_JNILib_ ## s

extern "C"
{
	JNIEXPORT void JNICALL JNI_PACK(created)(JNIEnv * env, jclass cls) 
	{
		GameInit();
		g_fs.Init(pGame->GetGamedir());
		LogInit();
		Created();
	}

	JNIEXPORT void JNICALL JNI_PACK(changed)(JNIEnv * env, jclass cls, jint width, jint height) 
	{
		Changed(width, height);
	}

	JNIEXPORT void JNICALL JNI_PACK(draw)(JNIEnv * env, jclass cls) 
	{
		Draw();
	}

	JNIEXPORT void JNICALL JNI_PACK(ontouch)(JNIEnv * env, jclass cls, jfloat tx, jfloat ty, jint ta, jint tf) 
	{
		OnTouch(tx, ty, ta, tf);
	}
	
	JNIEXPORT void JNICALL JNI_PACK(onkey)(JNIEnv * env, jclass cls, jint k, jint a)
	{
		OnKey(k, 0, a, 0);
	}
}
