
#pragma once

#include <vector>
#include "engine.h"
#include "button.h"
#include "system/config.h"

class IRenderer;

class tpsSettings{
public:
	tpsSettings();
	void Load(ConfigFile &cfg);
	bool tps;
	bool drawTouch;
	bool debugPhysics;
	bool drawBbox;
	float screenScale;
};

enum eMenuState{
	eMenuMain = 1,
	eMenuSettings = 2,
	eMenuPause = 3
};
/*
class MenuButton: public Button{
public:
	MenuButton(float nx,float ny, float nw, float nh, const char *t){
		x=nx;y=ny;w=nw;h=nh;
		text = t;
		pressed = false;
	}
};
*/
class tpsMenu{
public:
	void Init(tpsSettings *s,IRenderer *r);
	void Resize(float aspect);
	void SetState(eMenuState s);
	void Update(float deltaTime);
	void Draw(IRenderer *r);
	void OnTouch(float x, float y, int a, int f);
	
	eMenuState state;
	eMenuState prevState;
	Button bPlay;
	Button bResume;
	Button bSettings;
	Button bQuit;
	Button bView;
	Button bTouch;
	Button bScreenScale;
	Button bBack;
	std::vector<Button*> buttons;
	tpsSettings *sets;
	int screenScaleState;
	float tempScreenScale;
};

