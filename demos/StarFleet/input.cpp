
#include "button.h"
#include "log.h"
#include "star-fleet.h"

extern float aspect;
extern Button bPause;
extern Button bResume;

void Button::Update(){
	if(!active || !func)
		return;
	if(pressed){
		pressed = false;
		func();
	}
}

Button::Button(float nx, float ny, float nw, float nh, bool adjust):
			x(nx),y(ny),w(nw),h(nh),text(0),active(true),pressed(false)
{
	if(adjust){
		if(aspect>1)
			h*=aspect;
		else
			w/=aspect;
	}
	//Log("Created button %f %f %f %f\n",x,y,w,h);
}

Button::Button(float nx, float ny, float nw, float nh, const char *t, bool adjust):
			x(nx),y(ny),w(nw),h(nh),text(t),active(true),pressed(false)
{
	if(adjust)
		w/=aspect;
	
	//Log("Created text button %f %f %f %f %s\n",x,y,w,h,t);
}
/*
bool Button::Hit(float tx, float ty)
{
	if(!active)
		return false;
	if(tx>x+w||tx<x)
		return false;
	if(ty>y+h||ty<y)
		return false;
	pressed = true;
	return true;
}*/

#ifndef ANDROID
#include <GLFW/glfw3.h>
void StarFleetGame::OnKey(int key, int scancode, int action, int mods)
{
	if(action==GLFW_PRESS){
		if(key==GLFW_KEY_ESCAPE){
			if(gameState==eGameState_Play)
				bPause.pressed = true;
			else if(gameState==eGameState_Pause)
				bResume.pressed = true;
		}
	}
}
#else
void StarFleetGame::OnKey(int key, int scancode, int action, int mods)
{
	
}
#endif
