
#include <gtc/packing.hpp>
#include "Model.h"

Model::Model(): vertexFormat(),submeshes(),materials(),colliders(),animations()
{
	verts = 0;
	inds = 0;

	vertexCount = 0;
	indexCount = 0;
}

Model::~Model()
{

}

void Model::Free()
{
	if(verts)
		delete[] verts;
	if(inds)
		delete[] inds;
}

uint32_t Model::GetIndex(int i)
{
	if(indexSize==4)
		return ((uint32_t*)inds)[i];
	else if(indexSize==2)
		return ((uint16_t*)inds)[i];
	return 0;
}

glm::vec3 Model::GetPos(int i)
{
	vertAttrib_t &va = vertexFormat[0];
	char *v = verts+va.offset+va.stride*i;

	if(va.type==GL_FLOAT&&va.size==3){
		return *(vec3*)v;
	}
	return vec3(0);
}

glm::vec3 Model::GetNorm(int i)
{
	vertAttrib_t &va = vertexFormat[1];
	char *v = verts+va.offset+va.stride*i;

	if(va.type==GL_FLOAT&&va.size==3)
		return *(vec3*)v;
	if(va.type==GL_BYTE&&va.norm){
		glm::tvec3<int8_t> bvec = *(glm::tvec3<int8_t>*)v;
		vec3 r=bvec;
		return r/127.0f;
	}
	return vec3(0);
}

glm::vec2 Model::GetUV(int i)
{
	vertAttrib_t &va = vertexFormat[2];
	char *v = verts+va.offset+va.stride*i;

	if(va.type==GL_FLOAT&&va.size==2)
		return *(vec2*)v;
	if(va.type==GL_HALF_FLOAT&&va.size==2){
		return vec2(glm::unpackHalf1x16(*(short*)v),glm::unpackHalf1x16(*(short*)(v+2)));
	}
	return vec2(0);
}
