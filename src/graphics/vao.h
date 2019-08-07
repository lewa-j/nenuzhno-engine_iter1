
#pragma once

class VertexBufferObject;

class VertexArrayObject
{
public:
	VertexArrayObject();
	~VertexArrayObject();

	bool Create();
	void Bind();
	static void Unbind();
	void SetAttribute(int index,int size,int type,bool norm, int stride, void *vert);
	//void SetVertsVBO(int index,int size,int type,bool norm, int stride, int offset, VertexBufferObject &vbo);

	unsigned int id;
};

