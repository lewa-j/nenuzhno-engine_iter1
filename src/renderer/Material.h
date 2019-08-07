
#pragma once

class IRenderer;
class Texture;

class IMaterial{
public:
	virtual void Bind(IRenderer *r)=0;
	bool lit;//TODO flags?
};

//in renderer.cpp
class TexMaterial: public IMaterial{
public:
	TexMaterial(Texture *t,bool light=false):tex(t){lit=light;}
	virtual void Bind(IRenderer *r);
	Texture *tex;
};

