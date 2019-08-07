
#include <fstream>
using namespace std;
#include "log.h"
#include "system/FileSystem.h"
#include "graphics/platform_gl.h"
#include "graphics/ArrayBuffer.h"
#include "renderer/mesh.h"

Mesh* LoadMeshFile(const char* fileName, bool vbo)
{
	string filePath = "models/"+string(fileName)+".mesh";
	IFile *file = g_fs.Open(filePath.c_str());
	if(!file){
		//Log("File: %s not found!\n", path);
		return 0;
	}

	int numVerts = 0;
	file->Read(&numVerts, 4);//tris
	numVerts*=3;
	Log("LoadMesh %s numverts %d\n", fileName, numVerts);
	GLfloat *verts = new GLfloat[numVerts*8];
	file->Read(verts, numVerts*32);
	g_fs.Close(file);

	Mesh *out = 0;
	if(vbo){
		out = new MeshFBO_N3_T2(verts, numVerts, GL_TRIANGLES);
		delete[] verts;
	}else{
		out = new Mesh_N3_T2(verts, numVerts, GL_TRIANGLES);
	}
	return out;
}
