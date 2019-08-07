
#include "cstdlib"
#include "log.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"

const char *GetGLErrorString(int err)
{
	switch(err)
	{
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	default:
		return "???";
	}
}

int CheckGLError(const char *func, const char* file, int line)
{
	GLenum err = glGetError();
	if(err)
	{
		Log("gl Error %x(%s) on %s %s(%d)\n",err,GetGLErrorString(err),func,file,line);
		//TODO glError Fatal option
		//exit(err);
	}
	return err;
}

