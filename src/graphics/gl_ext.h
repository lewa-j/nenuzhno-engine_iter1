
#pragma once

#include <stdint.h>

enum GLExtFlags{
	eGLES=1,
	eCore=2,
	eVertex_array_object=4,
	eTexture_npot=8,
	eTexture_compression_s3tc=16,
	ePacked_depth_stencil=32,
	eTexture_filter_anisotropic=64,
	eCompressed_ETC1_RGB8_texture=128,
	eDepth24=256,
	eDepth32=512,
	eElement_index_uint=1024,
	eFBO_rgb8_rgba8=2048,
	eTexture_format_BGRA8888=4096,
	eProgBin=8192,
	eDepth_texture=16384
};

class GLExtensions{
public:
	static void Init();
	static void InitVAO();
	static void InitBinProg();

	static uint64_t extFlags;
};

#ifdef ANDROID
#include "graphics/platform_gl.h"
#include <GLES2/gl2ext.h>
extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays;
extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays;
extern PFNGLISVERTEXARRAYOESPROC glIsVertexArray;
extern PFNGLGETPROGRAMBINARYOESPROC glGetProgramBinaryOES;
extern PFNGLPROGRAMBINARYOESPROC glProgramBinaryOES;

#define GL_HALF_FLOAT GL_HALF_FLOAT_OES
#endif
