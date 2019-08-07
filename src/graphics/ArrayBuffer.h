
#pragma once

#include "graphics/platform_gl.h"

class ArrayBuffer{
public:
	GLuint id;
	GLenum target;

	ArrayBuffer();
	virtual ~ArrayBuffer();

	virtual void Create();
	virtual void Bind();
	virtual void Unbind();
	virtual void Upload(GLsizeiptr size, const void *data, GLenum type = GL_STATIC_DRAW);
	virtual void Update(GLintptr offset, GLsizeiptr size, const void *data);
};

class VertexBufferObject: public ArrayBuffer
{
public:
	VertexBufferObject();
};

class IndexBufferObject: public ArrayBuffer
{
public:
	IndexBufferObject();
};
