
#pragma once

class ResourceManager;
class Texture;

struct fontChar_t
{
	float x;
	float y;
	float width;
	float height;
	float xoffset;
	float yoffset;
	float xAdv;
};

class Font
{
public:
	Texture *tex;
	bool bmfont;
	fontChar_t chars[256];
	Font():tex(0),bmfont(0),base(0){}
	Font(Texture *t):tex(t),bmfont(0),base(0){}
	bool LoadBMFont(const char *fileName,ResourceManager *resMan);
	void Print(const char *text,float x, float y, float size);
private:
	void PrintBMFont(const char *text,float x, float y, float size);
	void PrintOLD(const char *text,float x, float y, float size);

	float base;
};

