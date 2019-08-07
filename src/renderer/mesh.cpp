
#include "graphics/platform_gl.h"
#include "graphics/ArrayBuffer.h"
#include "mesh.h"

void Mesh::Bind()
{
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, verts);
}

void Mesh::Draw()
{
	glDrawArrays(mode, 0, numVerts);
}

void Mesh::Free()
{
	if(verts)
		delete[] verts;
	verts = 0;
	numVerts = 0;
	if(inds)
		delete[] inds;
	inds = 0;
	numInds = 0;
}

MeshFBO::MeshFBO(float *v, int nv, int m): Mesh(v, nv, m)
{
	vbo.Create();
	vbo.Upload(sizeof(float)*3*numVerts, verts);
}

void MeshFBO::Bind()
{
	vbo.Bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);
}

void MeshFBO::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh_N3_T2::Bind()
{
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, verts);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, verts+3);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, verts+6);
}

void Mesh_N3_T2::DrawPos()
{
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, verts);
	glDrawArrays(mode, 0, numVerts);
}

MeshFBO_N3_T2::MeshFBO_N3_T2(float *v, int nv, int m)
{
	verts = v;
	numVerts = nv;
	inds = 0;
	numInds = 0;
	mode = m;
	vbo.Create();
	vbo.Upload(sizeof(float)*8*numVerts, verts);
}

MeshFBO_N3_T2::MeshFBO_N3_T2(float *v, int nv, GLushort *i, int ni, int m)
{
	verts = v;
	numVerts = nv;
	inds = i;
	numInds = ni;
	mode = m;
	vbo.Create();
	vbo.Upload(sizeof(float)*8*numVerts, verts);
}

void MeshFBO_N3_T2::Bind()
{
	vbo.Bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
}

void MeshFBO_N3_T2::DrawPos()
{
	vbo.Bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, 0);
	glDrawArrays(mode, 0, numVerts);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

