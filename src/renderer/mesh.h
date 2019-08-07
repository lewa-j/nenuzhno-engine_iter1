
#pragma once

#include "graphics/ArrayBuffer.h"

//TODO implement extension GL_OES_element_index_uint
class Mesh
{
public:
	GLfloat *verts;
	GLuint numVerts;
	GLushort *inds;
	GLuint numInds;
	GLuint mode;

	Mesh(): verts(0), numVerts(0), inds(0), numInds(0), mode(GL_TRIANGLES)
	{}
	Mesh(float *v, int nv, int m): verts(v), numVerts(nv), inds(0), numInds(0), mode(m)
	{}
	Mesh(float *v, int nv, GLushort *i, int ni, int m): verts(v), numVerts(nv), inds(i), numInds(ni), mode(m)
	{}

	virtual ~Mesh(){}
	
	virtual void Bind();
	virtual void Draw();
	virtual void Unbind()
	{}
	void Free();
};

class MeshFBO: public Mesh
{
public:
	VertexBufferObject vbo;
	MeshFBO(): Mesh()
	{}
	MeshFBO(float *v, int nv, int m);
	virtual void Bind();
	//void Draw();
	void Unbind();
};

class Mesh_N3_T2: public Mesh
{
public:
	Mesh_N3_T2(float *v, int nv, int m):Mesh(v, nv, m)
	{}
	Mesh_N3_T2(float *v, int nv, GLushort *i, int ni, int m):Mesh(v, nv, i, ni, m)
	{}
	void Bind();
	//void Draw();
	//void Unbind();
	void DrawPos();
};

class MeshFBO_N3_T2: public MeshFBO
{
public:
	MeshFBO_N3_T2(float *v, int nv, int m);
	MeshFBO_N3_T2(float *v, int nv, GLushort *i, int ni, int m);
	void Bind();
	//void Draw();
	//void Unbind();
	void DrawPos();
};

Mesh* LoadMeshFile(const char* fileName, bool vbo);
Mesh* LoadObjFile(const char* fileName, bool vbo);

