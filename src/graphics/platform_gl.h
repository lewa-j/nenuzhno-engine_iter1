
#pragma once

#ifndef ANDROID
#if !USE_CORE_GL_HEADER
	#define GLEW_NO_GLU 1
	#define GLEW_STATIC 1
	#include <GL/glew.h>
#else
	#define GL_GLEXT_PROTOTYPES 1
	#include "GL/glcorearb.h"
	#include "GL/gl.h"
#endif
#else
	#include <GLES2/gl2.h>
#endif
