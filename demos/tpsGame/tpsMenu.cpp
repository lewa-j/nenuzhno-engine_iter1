
#include <stdlib.h>
#include "tpsMenu.h"
#include "graphics/platform_gl.h"
#include "renderer/renderer.h"

void GameLog(const char *s);
void tpsStartGame();
void tpsResumeGame();

tpsSettings::tpsSettings(){
	tps = true;
	drawTouch = true;
	debugPhysics = false;
	drawBbox = false;
	screenScale = 1;
}

void tpsSettings::Load(ConfigFile &cfg){
	tps = cfg.values["view"]=="tps";
	drawTouch = cfg.values["drawTouch"]!="0";
	debugPhysics = cfg.values["debugPhysics"]!="0";
	drawBbox = cfg.values["drawBbox"]!="0";
	screenScale = atof(cfg.values["screenScale"].c_str());
}


void tpsMenu::Init(tpsSettings *s,IRenderer *r){
	sets = s;
	tempScreenScale = sets->screenScale;

	bPlay = Button(0.1,0.5,0.6,0.08,"Play");
	bResume = Button(0.1,0.5,0.6,0.08,"Resume");
	bSettings = Button(0.1,0.6,0.6,0.08,"Settings");
	bQuit = Button(0.1,0.7,0.6,0.08,"Quit");
	bView = Button(0.04,0.6,0.5,0.08,"View:");
	bTouch = Button(0.04,0.5,0.5,0.08,"Draw touch:");
	bScreenScale = Button(0.04,0.4,0.5,0.08,"Screen scale");
	bBack = Button(0.04,0.7,0.5,0.08,"Back");

	//r->AddButton(&bPlay);
	buttons.push_back(&bPlay);
	buttons.push_back(&bResume);
	buttons.push_back(&bSettings);
	buttons.push_back(&bQuit);
	buttons.push_back(&bView);
	buttons.push_back(&bTouch);
	buttons.push_back(&bScreenScale);
	buttons.push_back(&bBack);

	SetState(eMenuMain);
}

void tpsMenu::Resize(float aspect){
	bPlay.w = 0.4/aspect;
	bResume.w = 0.4/aspect;
	bSettings.w = 0.4/aspect;
	bQuit.w = 0.4/aspect;
	bView.x = 0.04/aspect;
	bView.w = 0.35/aspect;
	bTouch.x = 0.04/aspect;
	bTouch.w = 0.35/aspect;
	bScreenScale.x = 0.04/aspect;
	bScreenScale.w = 0.35/aspect;
	bBack.x = 0.04/aspect;
	bBack.w = 0.35/aspect;
}
	
void tpsMenu::SetState(eMenuState s)
{
	prevState = state;
	state = s;

	for(uint32_t i=0;i<buttons.size();i++){
		buttons[i]->active = false;
	}

	switch(s){
		case eMenuMain:
			bPlay.active = true;
		case eMenuPause:
			bSettings.active = true;
			bQuit.active = true;
			if(s==eMenuPause)
				bResume.active = true;
		break;
		case eMenuSettings:
			bTouch.active = true;
			bView.active = true;
			bScreenScale.active = true;
			bBack.active = true;
		break;
	}
}

void tpsMenu::Update(float deltaTime)
{
	if(bPlay.pressed){
		bPlay.pressed = false;
		SetState(eMenuPause);
		GameLog("play\n");
		//game->StartGame();
		tpsStartGame();
	}
	if(bResume.pressed){
		bResume.pressed = false;
		tpsResumeGame();
	}
	if(bSettings.pressed){
		bSettings.pressed = false;
		SetState(eMenuSettings);
	}
	if(bQuit.pressed){
		bQuit.pressed = false;
		EngineQuit();
	}

	if(state==eMenuSettings){
		if(bBack.pressed){
			bBack.pressed = false;
			SetState(prevState);
			sets->screenScale = tempScreenScale;
		}
		if(bView.pressed){
			bView.pressed = false;
			GameLog("change view\n");
			sets->tps=!sets->tps;
		}
		if(bTouch.pressed){
			bTouch.pressed = false;
			sets->drawTouch = !sets->drawTouch;
		}
		if(bScreenScale.pressed){
			bScreenScale.pressed = false;
			screenScaleState++;
			screenScaleState%=32;
			screenScaleState = screenScaleState%32;
			tempScreenScale = ((32-screenScaleState)/32.0f);
		}
	}
}

void tpsMenu::Draw(IRenderer *r)
{
	if(state==eMenuMain||state==eMenuPause){
		r->DrawText("Game title",0.04,0.25,1.5f*r->aspect);
	}else if(state==eMenuSettings){
		r->DrawText("Settings",0.04,0.25,1.0f*r->aspect);
		if(sets->tps)
			r->DrawText("Third person",0.4,bView.y+0.05,0.4);
		else
			r->DrawText("First person",0.4,bView.y+0.05,0.4);
		if(sets->drawTouch)
			r->DrawText("On",0.4,bTouch.y+0.05,0.4);
		else
			r->DrawText("Off",0.4,bTouch.y+0.05,0.4);
			
		char str[64];
		snprintf(str,64,"%.3f",tempScreenScale);
		r->DrawText(str,0.45,bScreenScale.y+0.05,0.4);
	}

	glEnable(GL_BLEND);
	for(uint32_t i=0;i<buttons.size();i++){
		Button *b = buttons[i];
		if(b->active){
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			r->SetColor(1,1,1,0.2);
			r->DrawRect(b->x,b->y,b->w,b->h);
			r->SetColor(1,1,1,1);
			if(b->text){
				glBlendFunc(1,1);
				//r->DrawText(b->text,b->x*r->aspect+0.01,1-b->y-0.05,0.5);
				r->DrawText(b->text,b->x*r->aspect+0.01,b->y+0.05,0.5);
			}
		}
	}
	glDisable(GL_BLEND);
}

void tpsMenu::OnTouch(float x, float y, int a, int f){
	if(a==IN_PRESS){
		for(uint32_t i=0;i<buttons.size();i++){
			buttons[i]->Hit(x,y);
		}
	}
}


