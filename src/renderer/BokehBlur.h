
#pragma once

class glslProg;
class VertexBufferObject;
class Texture;

class BokehBlur{
public:
	BokehBlur();
	~BokehBlur();
	
	bool Init(int w, int h, Texture *src);
	void Render(float size, Texture *src=0);
	
	glslProg *progBokeh;
	int u_size;
	VertexBufferObject *vboGrid;
	int gridW;
	int gridH;
	Texture *texCore;
	Texture *texSrc;
};

