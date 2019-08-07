
#pragma once

void GameInit();

class IGame{
public:
	virtual void Created()=0;
	virtual void Changed(int w, int h)=0;
	virtual void Draw()=0;
	virtual const char* GetGamedir()=0;

	virtual void OnKey(int key, int scancode, int action, int mods){}
	virtual void OnTouch(float tx, float ty, int ta, int tf){}
	virtual void OnMouseMove(float x, float y){}
	virtual void OnScroll(float sx, float sy){}
};

extern IGame *pGame;

//implemented in game code
IGame *CreateGame();
