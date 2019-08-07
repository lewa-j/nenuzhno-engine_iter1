
#pragma once

#include "input.h"
#include <vec2.hpp>

class Button
{
public:
	float x,y, w,h;
	int type;
	const char *text;
	bool active;
	bool pressed;
	void (*func)();

	Button():x(0),y(0),w(0),h(0),type(0),text(0),active(0),pressed(0){func=0;}
	Button(float nx, float ny, float nw, float nh, bool adjust = false);
	Button(float nx, float ny, float nw, float nh, const char *t, bool adjust=false);
	
	bool SetUniform(int loc);
	virtual bool Hit(float tx, float ty){
		if(!active)
			return false;
		if(tx>x+w||tx<x)
			return false;
		if(ty>y+h||ty<y)
			return false;
		pressed = true;
		return true;
	}
	virtual void Update();
};

class Scroll: public Button
{
public:
	Scroll():start(0),end(0),vel(0),pos(0),pos0(0),fing(-1){}
	Scroll(float nx, float ny, float nw, float nh):Button(nx,ny,nw,nh){}
	bool Hit(float tx, float ty, int tf){
		bool r = Button::Hit(tx,ty);
		if(r){
			start = ty;
			fing = tf;
			pos0 = pos;
		}
		return r;
	}
	void Move(float tx, float ty, int tf){
		if(pressed&&tf==fing){
			end = ty;
			vel = end-start;
			pos = pos0+vel;
		}
	}
	void Release(int tf){
		if(tf!=fing)
			return;
		pos = pos0+vel;
		fing = -1;
		vel = 0;
		start = 0;
		end = 0;
		pressed = false;
	}
	float start,end,vel,pos,pos0;
	int fing;
};

class Joystick: public Button
{
public:
	Joystick(){}
	Joystick(float nx,float ny, float nw, float nh,float asp=1):Button(nx,ny,nw,nh,false){
		vel = glm::vec2(0);
		start = glm::vec2(0);
		end = glm::vec2(0);
		aspect = asp;
		fing = -1;
	}
	bool Hit(float tx, float ty, int tf){
		bool r = Button::Hit(tx,ty);
		if(r){
			start = glm::vec2(tx,ty);
			fing = tf;
		}
		return r;
	}
	void Move(float tx, float ty, int tf){
		if(pressed&&tf==fing){
			end = glm::vec2(tx,ty);
			vel = end-start;
			vel.y *= -aspect;
		}
	}
	void Release(int tf){
		if(tf!=fing)
			return;
		fing = -1;
		vel = glm::vec2(0);
		start = glm::vec2(0);
		end = glm::vec2(0);
		pressed = false;
	}
	void Update(){}
	
	glm::vec2 start;
	glm::vec2 end;
	glm::vec2 vel;
	float aspect;
	int fing;
};

class KeyJoystick: public Joystick
{
public:
	KeyJoystick(){}
	KeyJoystick(int forward, int back, int right, int left):f(forward),b(back),r(right),l(left){
		vel = glm::vec2(0);
	}
	void OnKey(int k, int a){
		if(a==IN_KEY_PRESS){
			if(k==f){
				vel.y = 1;
			}else if(k==b){
				vel.y = -1;
			}else if(k==l){
				vel.x = -1;
			}else if(k==r){
				vel.x = 1;
			}
		}else if(a==IN_KEY_RELEASE){
			if(k==f){
				vel.y = 0;
			}else if(k==b){
				vel.y = 0;
			}else if(k==l){
				vel.x = 0;
			}else if(k==r){
				vel.x = 0;
			}
		}
	}
	int f,b,r,l;
};

