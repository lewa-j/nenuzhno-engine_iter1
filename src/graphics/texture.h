
#pragma once
//TODO split interface and implementation

#include "graphics/platform_gl.h"

#ifdef ANDROID
#include "GLES2/gl2ext.h"
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#define GL_RGBA16F GL_RGBA
#define GL_HALF_FLOAT GL_HALF_FLOAT_OES
//GL_EXT_texture_format_BGRA8888
#define GL_BGRA GL_BGRA_EXT
#else
#include "GL/glext.h"
#endif
#define GL_COMPRESSED_RGB_S3TC_DXT1 GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT

#define FMT_BGR8 0x321
#define FMT_ARGB8 0x4321
#define FMT_V8U8 0xF8E8

void ResampleBGR(uint8_t *data, int size);
void ResampleBGRA(uint8_t *data, int size);

class Texture
{
public:
	int width;
	int height;
	GLuint id;
	GLuint target;
	GLuint type;
	GLuint fmt;
	GLuint infmt;

	Texture();
	~Texture();

	bool Create(int w, int h);
	void Bind();
	void SetWrap(int wrap);
	void SetFilter(int min, int mag);
	void Upload(GLint level, GLuint format, const void *data);
	void Upload(GLint level, GLuint iformat, GLuint format, GLuint tp, const void *data);
	void Upload(GLint level, int w, int h, const void *data);
	void UploadCompressed(GLuint iformat, int size, const void *data);
};
