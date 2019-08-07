
#pragma once

#include <string>
#include <gtc/quaternion.hpp>
#include "system/neArray.h"
#include "graphics/ArrayBuffer.h"
#include "renderer/Material.h"
#include "cull/BoundingBox.h"

using glm::vec2;
using glm::vec3;

class Texture;

struct submesh_t{
	uint32_t offs;
	int count;
	int mat;
};

struct bone_t{
	int parent;
	glm::vec3 pos;
	//vec3 rot;
	glm::quat rot;
};

struct AnimFrame_t{
	AnimFrame_t():bones(){}
	neArray<bone_t> bones;
};

struct Animation_t{
	Animation_t():name(),frames(){}
	std::string name;
	float fps;
	neArray<AnimFrame_t> frames;
};

enum eVertType{
	eVertType_byte=0,
	eVertType_bvec2,
	eVertType_bvec3,
	eVertType_bvec4,
	eVertType_nbyte=4,//normalized
	eVertType_nbvec2,
	eVertType_nbvec3,
	eVertType_nbvec4,
	eVertType_ubyte=8,//unsigned
	eVertType_ubvec2,
	eVertType_ubvec3,
	eVertType_ubvec4,
	eVertType_nubyte=12,//normalized unsigned
	eVertType_nubvec2,
	eVertType_nubvec3,
	eVertType_nubvec4,

	eVertType_short=16,
	eVertType_svec2,
	eVertType_svec3,
	eVertType_svec4,
	eVertType_nshort=20,//normalized
	eVertType_nsvec2,
	eVertType_nsvec3,
	eVertType_nsvec4,
	eVertType_ushort=24,//unsigned
	eVertType_usvec2,
	eVertType_usvec3,
	eVertType_usvec4,
	eVertType_nushort=28,//normalized unsigned
	eVertType_nusvec2,
	eVertType_nusvec3,
	eVertType_nusvec4,

	eVertType_float=32,
	eVertType_vec2,
	eVertType_vec3,
	eVertType_vec4,

	eVertType_hfloat=48,//half float
	eVertType_hvec2,
	eVertType_hvec3,
	eVertType_hvec4
};

struct vertAttrib_t{
	vertAttrib_t(){}
	vertAttrib_t(int i, int s, int t, bool n, int st, uint64_t o):
			id(i),size(s),type(t),norm(n),stride(st),offset(o){}
	int id;
	int size;
	int type;
	bool norm;
	int stride;
	uint64_t offset;
};

//Temp
struct nmfMaterial{
	nmfMaterial():mat(0){}
	std::string name;
	IMaterial *mat;
};

#define NMF_COLLIDER_BOX 1

struct nmfCollider_t{
	uint16_t type;
	uint16_t flags;
	vec3 pos;
	vec3 rot;
	vec3 size;
};

class Model
{
public:
	Model();
	~Model();
	void Free();

	BoundingBox bbox;
	int vertexCount;
	//int vertexStride;
	neArray<vertAttrib_t> vertexFormat;
	char *verts;
	VertexBufferObject vbo;
	int indexCount;
	int indexSize;
	int indexType;
	char *inds;
	IndexBufferObject ibo;

	neArray<submesh_t> submeshes;
	neArray<nmfMaterial> materials;
	neArray<nmfCollider_t> colliders;
	neArray<bone_t> skeleton;
	neArray<Animation_t> animations;

	uint32_t GetIndex(int i);
	vec3 GetPos(int i);
	vec3 GetNorm(int i);
	vec2 GetUV(int i);
};
