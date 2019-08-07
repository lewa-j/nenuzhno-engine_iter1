
#include "vao.h"
#include "graphics/platform_gl.h"

/*#ifdef ANDROID
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2ext.h>
#define glBindVertexArray glBindVertexArrayOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#define glGenVertexArrays glGenVertexArraysOES
#endif*/
#include "graphics/gl_ext.h"

VertexArrayObject::VertexArrayObject()
{
	id = 0;
}

VertexArrayObject::~VertexArrayObject()
{
	if(id)
		glDeleteVertexArrays(1,&id);
}

bool VertexArrayObject::Create()
{
	glGenVertexArrays(1,&id);
	glBindVertexArray(id);

	glBindVertexArray(0);
	return true;
}

void VertexArrayObject::Bind()
{
	glBindVertexArray(id);
}

void VertexArrayObject::Unbind()
{
	glBindVertexArray(0);
}

void VertexArrayObject::SetAttribute(int index,int size,int type,bool norm, int stride, void *vert)
{
	glBindVertexArray(id);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index,size,type,norm,stride,vert);
}

