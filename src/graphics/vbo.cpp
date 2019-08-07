
#include "log.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
#include "graphics/ArrayBuffer.h"

ArrayBuffer::ArrayBuffer(){
	id = 0;
	target = GL_ARRAY_BUFFER;
}

ArrayBuffer::~ArrayBuffer(){
	if(id)
		glDeleteBuffers(1, &id);
}

void ArrayBuffer::Create(){
	glGenBuffers(1, &id);
}

void ArrayBuffer::Bind(){
	glBindBuffer(target, id);
}

void ArrayBuffer::Unbind(){
	glBindBuffer(target, 0);
}

void ArrayBuffer::Upload(GLsizeiptr size, const void *data, GLenum type){
	glBindBuffer(target, id);
	glBufferData(target, size, data, type);
	glBindBuffer(target, 0);
	CheckGLError("ArrayBuffer::Upload", __FILE__, __LINE__);
}

void ArrayBuffer::Update(GLintptr offset, GLsizeiptr size, const void *data){
	glBindBuffer(target, id);
	glBufferSubData(target, offset, size, data);
	glBindBuffer(target, 0);
	CheckGLError("ArrayBuffer::Update", __FILE__, __LINE__);
}

//VBO
VertexBufferObject::VertexBufferObject():ArrayBuffer(){
	target = GL_ARRAY_BUFFER;
}

//IBO
IndexBufferObject::IndexBufferObject():ArrayBuffer(){
	target = GL_ELEMENT_ARRAY_BUFFER;
}
