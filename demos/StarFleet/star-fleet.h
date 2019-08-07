
#pragma once

#include "game/IGame.h"
#include "renderer/camera.h"

class ResourceManager;
class IRenderer;
class Scene;

class StarFleetGame: public IGame{
public:
	void Created();
	void Changed(int w, int h);
	void Draw();
	const char *GetGamedir(){
		return "star-fleet";
	}

	void OnKey(int key, int scancode, int action, int mods);
	void OnTouch(float tx, float ty, int ta, int tf);

	ResourceManager *resMan;
	IRenderer *renderer;
	Camera camera;
	Scene *scene;
	
	double oldTime;
	float deltaTime;
	
	void Update();
	void DrawButton(Button *b);
};

enum eGameState
{
	eGameState_Menu,
	eGameState_Play,
	eGameState_Pause
};
extern eGameState gameState;
