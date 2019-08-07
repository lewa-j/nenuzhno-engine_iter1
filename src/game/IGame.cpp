
#include "game/IGame.h"
#include "log.h"

IGame *pGame = 0;

void GameInit(){
	Log("GameInit()\n");
	pGame = CreateGame();
}

void Created(){
	pGame->Created();
}

void Changed(int w, int h){
	pGame->Changed(w, h);
}

void Draw(){
	pGame->Draw();
}

void OnTouch(float x, float y, int a, int tf){
	pGame->OnTouch(x, y, a, tf);
}

void OnMouseMove(float x, float y){
	pGame->OnMouseMove(x, y);
}

void OnScroll(float sx, float sy){
	pGame->OnScroll(sx, sy);
}

void OnKey(int key, int scancode, int action, int mods){
	pGame->OnKey(key, scancode, action, mods);
}
