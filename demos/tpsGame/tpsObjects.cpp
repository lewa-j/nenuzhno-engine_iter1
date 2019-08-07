
#include <vector>

#include "renderer/Model.h"
#include "renderer/renderer.h"
#include "graphics/platform_gl.h"

#include "tpsObjects.h"
#include "tpsPlayer.h"

using glm::vec3;
using glm::mat4;
using glm::translate;
using glm::mix;
using glm::scale;
using glm::rotate;
using glm::radians;

void GameLog(const char *s);
extern std::vector<ButtonObject*> g_buttons;

DoorObject::DoorObject(vec3 p,Model *m,vec3 s):pos(p),mdl(m){
	dynamic = true;
	modelMtx = translate(mat4(1.0f),p);
	state = DOOR_CLOSED;
	closePos = p;
	t = 0;
	size = s;
	openPos = p+vec3(0,s.y*2.0-0.05,0);
}
void DoorObject::Active(){
	GameLog("Door active\n");
	if(state&1)//moving
		return;
	if(state==DOOR_CLOSED)
		state=DOOR_OPENING;
	else if(state==DOOR_OPEN)
		state = DOOR_CLOSING;
}
void DoorObject::Update(float deltaTime){
	if((state&1)==0)
		return;
	t+=deltaTime;
	if(t>1){
		t=1;
	}
	if(state==DOOR_OPENING){
		pos=mix(closePos,openPos,t);
	}else{
		pos=mix(openPos,closePos,t);
	}
	modelMtx = translate(mat4(1.0f),pos);
	if(t==1){
		t = 0;
		if(state==DOOR_OPENING)
			state=DOOR_OPEN;
		else
			state=DOOR_CLOSED;
		GameLog("door stop\n");
	}
}
void DoorObject::Draw(IRenderer *r){
	if(mdl){
		r->DrawModel(mdl,scale(modelMtx,size));
	}
}

ButtonObject::ButtonObject(vec3 p):SceneObject(){
	dynamic = true;
	modelMtx = translate(mat4(1.0f),p);
	pos = p;
	rot = 0;
	target = 0;
	type = BUTTON_NONE;
	g_buttons.push_back(this);
}
ButtonObject::~ButtonObject(){
	for(std::vector<ButtonObject*>::iterator it = g_buttons.begin();it!=g_buttons.end();it++){
		if(*it==this){
			g_buttons.erase(it);
			break;
		}
	}
}
void ButtonObject::Update(){
	
}
void ButtonObject::Draw(IRenderer *r){
	r->SetModelMtx(rotate(scale(modelMtx,vec3(1,1,1)),radians(rot),vec3(0,1,0)));
	//TODO fix (move to material?)
	glDisable(GL_CULL_FACE);
	r->DrawText(name.c_str(),-0.2,0.15,1);
	glEnable(GL_CULL_FACE);
	if(mdl){
		r->DrawModel(mdl,scale(modelMtx,vec3(0.1)));
	}
}
void ButtonObject::Press(Player *pl){
	GameLog("button pressed\n");
	if(type==BUTTON_ACTIVE&&target)
		target->Active();
	else if(type==BUTTON_TP){
		pl->SetPos(targetPos);
		pl->SetRot(targetRot);
	}
}

