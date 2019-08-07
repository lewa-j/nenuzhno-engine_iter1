
#include "log.h"
#include "mdl_loader.h"
#include <cstring>
#include <stdint.h>
#include "system/FileSystem.h"
#include <renderer/Model.h>
#include <vec2.hpp>
#include <gtc/packing.hpp>
#include <glm.hpp>

typedef unsigned char byte;
typedef glm::vec2 Vector2D;
typedef glm::vec3 Vector;
typedef glm::quat Quaternion;
typedef glm::vec3 RadianEuler;
typedef glm::mat3x4 matrix3x4_t;

#define IDSTUDIOHEADER (('T'<<24)+('S'<<16)+('D'<<8)+'I')
#define MAX_NUM_LODS 8
#define MAX_NUM_BONES_PER_VERT 3

struct studiohdr_t{
	uint32_t			id;
	uint32_t			version;
	uint32_t			checksum;		// this has to be the same in the phy and vtx files to load!
	inline const char *	pszName( void ) const { return name; }
	char				name[64];
	uint32_t			length;
	Vector				eyeposition;	// ideal eye position
	Vector				illumposition;	// illumination center
	Vector				hull_min;		// ideal movement hull size
	Vector				hull_max;
	Vector				view_bbmin;		// clipping bounding box
	Vector				view_bbmax;
	int					flags;
	int					numbones;			// bones
	int					boneindex;
	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;
	int					numhitboxsets;
	int					hitboxsetindex;
	int					numlocalanim;			// animations/poses
	int					localanimindex;		// animation descriptions
	int					numlocalseq;				// sequences
	int					localseqindex;
	mutable int			activitylistversion;	// initialization flag - have the sequences been indexed?
	mutable int			eventsindexed;
	int					numtextures;
	int					textureindex;
	int					numcdtextures;
	int					cdtextureindex;
	int					numskinref;
	int					numskinfamilies;
	int					skinindex;
	int					numbodyparts;
	int					bodypartindex;
	int					numlocalattachments;
	int					localattachmentindex;
	int					numlocalnodes;
	int					localnodeindex;
	int					localnodenameindex;
	int					numflexdesc;
	int					flexdescindex;
	int					numflexcontrollers;
	int					flexcontrollerindex;
	int					numflexrules;
	int					flexruleindex;
	int					numikchains;
	int					ikchainindex;
	int					nummouths;
	int					mouthindex;
	int					numlocalposeparameters;
	int					localposeparamindex;
	int					surfacepropindex;
	int					keyvalueindex;
	int					keyvaluesize;
	int					numlocalikautoplaylocks;
	int					localikautoplaylockindex;
	float				mass;
	int					contents;
	int					numincludemodels;
	int					includemodelindex;
	//mutable void		*virtualModel;
	uint32_t virtualModel;
	int					szanimblocknameindex;
	int					numanimblocks;
	int					animblockindex;
	uint32_t animblockModel;//void*
	int					bonetablebynameindex;
	//void				*pVertexBase;
	uint32_t pVertexBase;
	//void				*pIndexBase;
	uint32_t pIndexBase;
	byte				constdirectionallightdot;
	byte				rootLOD;
	byte				numAllowedRootLODs;
	byte				unused[1];
	int					unused4; // zero out if version < 47
	int					numflexcontrollerui;
	int					flexcontrolleruiindex;
	int					unused3[2];
	int					studiohdr2index;
	int					unused2[1];
	studiohdr_t() {}
private:
	// No copy constructors allowed
	studiohdr_t(const studiohdr_t& vOther);
};

struct mstudio_modelvertexdata_t
{
	// base of external vertex data stores
	//const void			*pVertexData;
	//const void			*pTangentData;
	uint32_t pVertexData;
	uint32_t pTangentData;
};

struct mstudio_meshvertexdata_t
{
	//const mstudio_modelvertexdata_t	*modelvertexdata;
	uint32_t modelvertexdata;
	int			numLODVertexes[MAX_NUM_LODS];
};

struct mstudiomesh_t
{
	int					material;
	int					modelindex;
	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexoffset;		// vertex mstudiovertex_t
	int					numflexes;			// vertex animation
	int					flexindex;
	int					materialtype;
	int					materialparam;
	int					meshid;
	Vector				center;
	mstudio_meshvertexdata_t vertexdata;
	int					unused[8]; // remove as appropriate
	mstudiomesh_t(){}
private:
	// No copy constructors allowed
	mstudiomesh_t(const mstudiomesh_t& vOther);
};

// studio models
struct mstudiomodel_t
{
	inline const char * pszName( void ) const { return name; }
	char				name[64];
	int					type;
	float				boundingradius;
	int					nummeshes;
	int					meshindex;
	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexindex;		// vertex Vector
	int					tangentsindex;		// tangents Vector
	int					numattachments;
	int					attachmentindex;
	int					numeyeballs;
	int					eyeballindex;
	mstudio_modelvertexdata_t vertexdata;
	int					unused[8];		// remove as appropriate
};

struct mstudiobodyparts_t
{
	int					sznameindex;
	int					nummodels;
	int					base;
	int					modelindex; // index into models array
};

struct mstudiobone_t
{
	int					sznameindex;
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none
	Vector				pos;
	Quaternion			quat;
	RadianEuler			rot;
	Vector				posscale;
	Vector				rotscale;
	matrix3x4_t			poseToBone;
	Quaternion			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	mutable int			physicsbone;	// index into physically simulated bone
	int					surfacepropidx;	// index into string tablefor property name
	int					contents;		// See BSPFlags.h for the contents flags
	int					unused[8];		// remove as appropriate
	mstudiobone_t(){}
private:
	// No copy constructors allowed
	mstudiobone_t(const mstudiobone_t& vOther);
};

// sequence descriptions
struct mstudioseqdesc_t{
	int					baseptr;
	int					szlabelindex;
	inline char * const pszLabel( void ) const { return ((char *)this) + szlabelindex; }
	int					szactivitynameindex;
	inline char * const pszActivityName( void ) const { return ((char *)this) + szactivitynameindex; }
	int					flags;		// looping/non-looping flags
	int					activity;	// initialized at loadtime to game DLL values
	int					actweight;
	int					numevents;
	int					eventindex;
//	inline mstudioevent_t *pEvent( int i ) const { Assert( i >= 0 && i < numevents); return (mstudioevent_t *)(((byte *)this) + eventindex) + i; };
	Vector				bbmin;		// per sequence bounding box
	Vector				bbmax;
	int					numblends;
	int					animindexindex;
	int					movementindex;	// [blend] float array for blended movement
	int					groupsize[2];
	int					paramindex[2];	// X, Y, Z, XR, YR, ZR
	float				paramstart[2];	// local (0..1) starting value
	float				paramend[2];	// local (0..1) ending value
	int					paramparent;
	float				fadeintime;		// ideal cross fate in time (0.2 default)
	float				fadeouttime;	// ideal cross fade out time (0.2 default)
	int					localentrynode;		// transition node at entry
	int					localexitnode;		// transition node at exit
	int					nodeflags;		// transition rules
	float				entryphase;		// used to match entry gait
	float				exitphase;		// used to match exit gait
	float				lastframe;		// frame that should generation EndOfSequence
	int					nextseq;		// auto advancing sequences
	int					pose;			// index of delta animation between end and nextseq
	int					numikrules;
	int					numautolayers;	//
	int					autolayerindex;
//	inline mstudioautolayer_t *pAutolayer( int i ) const { Assert( i >= 0 && i < numautolayers); return (mstudioautolayer_t *)(((byte *)this) + autolayerindex) + i; };
	int					weightlistindex;
//	inline float		*pBoneweight( int i ) const { return ((float *)(((byte *)this) + weightlistindex) + i); };
//	inline float		weight( int i ) const { return *(pBoneweight( i)); };
	// FIXME: make this 2D instead of 2x1D arrays
	int					posekeyindex;
//	float				*pPoseKey( int iParam, int iAnim ) const { return (float *)(((byte *)this) + posekeyindex) + iParam * groupsize[0] + iAnim; }
//	float				poseKey( int iParam, int iAnim ) const { return *(pPoseKey( iParam, iAnim )); }
	int					numiklocks;
	int					iklockindex;
//	inline mstudioiklock_t *pIKLock( int i ) const { Assert( i >= 0 && i < numiklocks); return (mstudioiklock_t *)(((byte *)this) + iklockindex) + i; };
	// Key values
	int					keyvalueindex;
	int					keyvaluesize;
//	inline const char * KeyValueText( void ) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }
	int					cycleposeindex;		// index of pose parameter to use as cycle index
	int					unused[7];		// remove/add as appropriate (grow back to 8 ints on version change!)
	mstudioseqdesc_t(){}
private:
	// No copy constructors allowed
	mstudioseqdesc_t(const mstudioseqdesc_t& vOther);
};

// animation frames
union mstudioanimvalue_t{
	struct {
		byte	valid;
		byte	total;
	} num;
	short		value;
};

struct mstudioanim_valueptr_t{
	short	offset[3];
	inline mstudioanimvalue_t *pAnimvalue( int i ) const { if (offset[i] > 0) return  (mstudioanimvalue_t *)(((byte *)this) + offset[i]); else return NULL; }
};

#define STUDIO_ANIM_RAWPOS	0x01 // Vector48
#define STUDIO_ANIM_RAWROT	0x02 // Quaternion48
#define STUDIO_ANIM_ANIMPOS	0x04 // mstudioanim_valueptr_t
#define STUDIO_ANIM_ANIMROT	0x08 // mstudioanim_valueptr_t
#define STUDIO_ANIM_DELTA	0x10
#define STUDIO_ANIM_RAWROT2	0x20 // Quaternion64

// per bone per animation DOF and weight pointers
struct mstudioanim_t
{
	byte bone;
	byte flags;		// weighing options
	// valid for animating data only
	inline byte *pData( void ) const { return (((byte *)this) + sizeof( struct mstudioanim_t )); }
	inline mstudioanim_valueptr_t	*pRotV( void ) const { return (mstudioanim_valueptr_t *)(pData()); }
	inline mstudioanim_valueptr_t	*pPosV( void ) const { return (mstudioanim_valueptr_t *)(pData()) + ((flags & STUDIO_ANIM_ANIMROT) != 0); }
	// valid if animation unvaring over timeline
	//inline Quaternion48		*pQuat48( void ) const { return (Quaternion48 *)(pData()); }
	//inline Quaternion64		*pQuat64( void ) const { return (Quaternion64 *)(pData()); }
	//inline Vector48			*pPos( void ) const { return (Vector48 *)(pData() + ((flags & STUDIO_ANIM_RAWROT) != 0) * sizeof( *pQuat48() ) + ((flags & STUDIO_ANIM_RAWROT2) != 0) * sizeof( *pQuat64() ) ); }
	inline glm::quat Quat() const {
		struct t{uint64_t x:21;
				 uint64_t y:21;
				 uint64_t z:21;
				 uint64_t wneg:1;
				}raw = *((t*)pData());
		glm::quat tmp;
		tmp.x = ((int)raw.x - 1048576) * (1 / 1048576.5f);
		tmp.y = ((int)raw.y - 1048576) * (1 / 1048576.5f);
		tmp.z = ((int)raw.z - 1048576) * (1 / 1048576.5f);
		tmp.w = sqrt( 1 - tmp.x * tmp.x - tmp.y * tmp.y - tmp.z * tmp.z );
		if (raw.wneg)
			tmp.w = -tmp.w;
		return tmp;
	}
	inline glm::vec3 PosVec3() const {
		int ofs = ((flags & STUDIO_ANIM_RAWROT2) != 0) * 8 + ((flags & STUDIO_ANIM_RAWROT) != 0) * 6;
		return glm::vec3(glm::unpackHalf1x16(*(short*)(pData()+ofs)),glm::unpackHalf1x16(*(short*)(pData()+ofs+2)),glm::unpackHalf1x16(*(short*)(pData()+ofs+4)));
	}
	short nextoffset;
	inline mstudioanim_t *pNext( void ) const { if (nextoffset != 0) return  (mstudioanim_t *)(((byte *)this) + nextoffset); else return NULL; }
};

struct mstudioanimdesc_t{
	int					baseptr;
	//inline studiohdr_t	*pStudiohdr( void ) const { return (studiohdr_t *)(((byte *)this) + baseptr); }
	int					sznameindex;
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
	float				fps;		// frames per second	
	int					flags;		// looping/non-looping flags
	int					numframes;
	// piecewise movement
	int					nummovements;
	int					movementindex;
	//inline mstudiomovement_t * const pMovement( int i ) const { return (mstudiomovement_t *)(((byte *)this) + movementindex) + i; };
	int					unused1[6];			// remove as appropriate (and zero if loading older versions)	
	int					animblock;
	int					animindex;	 // non-zero when anim data isn't in sections
	//mstudioanim_t *pAnimBlock( int block, int index ) const; // returns pointer to a specific anim block (local or external)
	//mstudioanim_t *pAnim( int *piFrame, float &flStall ) const; // returns pointer to data and new frame index
	//mstudioanim_t *pAnim( int *piFrame ) const; // returns pointer to data and new frame index
	int					numikrules;
	int					ikruleindex;	// non-zero when IK data is stored in the mdl
	int					animblockikruleindex; // non-zero when IK data is stored in animblock file
	//mstudioikrule_t *pIKRule( int i ) const;
	int					numlocalhierarchy;
	int					localhierarchyindex;
	//mstudiolocalhierarchy_t *pHierarchy( int i ) const;
	int					sectionindex;
	int					sectionframes; // number of frames used in each fast lookup section, zero if not used
	//inline mstudioanimsections_t * const pSection( int i ) const { return (mstudioanimsections_t *)(((byte *)this) + sectionindex) + i; }
	short				zeroframespan;	// frames per span
	short				zeroframecount; // number of spans
	int					zeroframeindex;
	//byte				*pZeroFrameData( ) const { if (zeroframeindex) return (((byte *)this) + zeroframeindex); else return NULL; };
	mutable float		zeroframestalltime;		// saved during read stalls
	mstudioanimdesc_t(){}
private:
	// No copy constructors allowed
	mstudioanimdesc_t(const mstudioanimdesc_t& vOther);
};

struct mstudiohitboxset_t{
	int					sznameindex;
	//inline char * const	pszName( void ) const { return ((char *)this) + sznameindex; }
	int					numhitboxes;
	int					hitboxindex;
	//inline mstudiobbox_t *pHitbox( int i ) const { return (mstudiobbox_t *)(((byte *)this) + hitboxindex) + i; };
};

struct mstudiobbox_t{
	int					bone;
	int					group;				// intersection group
	Vector				bbmin;				// bounding box
	Vector				bbmax;	
	int					szhitboxnameindex;	// offset to the name of the hitbox.
	int					unused[8];
	mstudiobbox_t() {}
private:
	// No copy constructors allowed
	mstudiobbox_t(const mstudiobbox_t& vOther);
};

// skin info
struct mstudiotexture_t{
	int						sznameindex;
	inline char * const		pszName( void ) const { return ((char *)this) + sznameindex; }
	int						flags;
	int						used;
    int						unused1;
	//mutable IMaterial		*material;  // fixme: this needs to go away . .isn't used by the engine, but is used by studiomdl
	uint32_t material;
	//mutable void			*clientmaterial;	// gary, replace with client material pointer if used
	uint32_t clientmaterial;
	int						unused[10];
};

// demand loaded sequence groups
struct mstudiomodelgroup_t{
	int	szlabelindex;	// textual name
	inline char * const pszLabel( void ) const { return ((char *)this) + szlabelindex; }
	int sznameindex;	// file name
	inline char * const pszName( void ) const { return ((char *)this) + sznameindex; }
};

//vvd
struct mstudioboneweight_t
{
	float	weight[MAX_NUM_BONES_PER_VERT];
	char	bone[MAX_NUM_BONES_PER_VERT];
	byte	numbones;
};

// NOTE: This is exactly 48 bytes
struct mstudiovertex_t
{
	mstudioboneweight_t	m_BoneWeights;
	Vector				m_vecPosition;
	Vector				m_vecNormal;
	Vector2D			m_vecTexCoord;
	mstudiovertex_t() {}
private:
	// No copy constructors allowed
	mstudiovertex_t(const mstudiovertex_t& vOther);
};

struct vertexFileHeader_t
{
	int		id;								// MODEL_VERTEX_FILE_ID
	int		version;						// MODEL_VERTEX_FILE_VERSION
	uint32_t	checksum;						// same as studiohdr_t, ensures sync
	int		numLODs;						// num of valid lods
	int		numLODVertexes[MAX_NUM_LODS];	// num verts for desired root lod
	int		numFixups;						// num of vertexFileFixup_t
	int		fixupTableStart;				// offset from base to fixup table
	int		vertexDataStart;				// offset from base to vertex block
	int		tangentDataStart;				// offset from base to tangent block
};

// apply sequentially to lod sorted vertex and tangent pools to re-establish mesh order
struct vertexFileFixup_t
{
	int		lod;				// used to skip culled root lod
	int		sourceVertexID;		// absolute index from start of vertex/tangent blocks
	int		numVertexes;
};

//vtx
#pragma pack(1)
struct vtxFileHeader_t
{
	int version;
	int vertCacheSize;
	unsigned short maxBonesPerStrip;
	unsigned short maxBonesPerTri;
	int maxBonesPerVert;
	uint32_t checkSum;
	int numLODs; // garymcthack - this is also specified in ModelHeader_t and should match
	int materialReplacementListOffset;
	int numBodyParts;
	int bodyPartOffset;
};

struct vtxBodyPartHeader_t
{
	int numModels;
	int modelOffset;
};

// This maps one to one with models in the mdl file.
// There are a bunch of model LODs stored inside potentially due to the qc $lod command
struct vtxModelHeader_t
{
	int numLODs; // garymcthack - this is also specified in FileHeader_t
	int lodOffset;
};

struct vtxModelLODHeader_t
{
	int numMeshes;
	int meshOffset;
	float switchPoint;
};

// a collection of locking groups:
// up to 4:
// non-flexed, hardware skinned
// flexed, hardware skinned
// non-flexed, software skinned
// flexed, software skinned
//
// A mesh has a material associated with it.
struct vtxMeshHeader_t
{
	int numStripGroups;
	int stripGroupHeaderOffset;
	unsigned char flags;
};

// a locking group
// a single vertex buffer
// a single index buffer
struct vtxStripGroupHeader_t
{
	// These are the arrays of all verts and indices for this mesh.  strips index into this.
	int numVerts;
	int vertOffset;
	int numIndices;
	int indexOffset;
	int numStrips;
	int stripOffset;
	unsigned char flags;
};

struct vtxVertex_t
{
	// these index into the mesh's vert[origMeshVertID]'s bones
	unsigned char boneWeightIndex[MAX_NUM_BONES_PER_VERT];
	unsigned char numBones;
	unsigned short origMeshVertID;
	// for sw skinned verts, these are indices into the global list of bones
	// for hw skinned verts, these are hardware bone indices
	char boneID[MAX_NUM_BONES_PER_VERT];
};
#pragma pack()

Model *MDLLoader::Load(const char *name){
	Log("Loading model: %s\n", name);
	std::string filePath = name;
	loadMDLOut_t mdlData;
	if(!LoadMDL(filePath.c_str(),mdlData))
		return NULL;
	int ext = filePath.find(".mdl");
	filePath = filePath.substr(0,ext);

	Model *mdl = new Model();
	int lod = 0;
	LoadVVD((filePath+".vvd").c_str(),mdl,lod);
	filePath+=".vtx";
	if(!g_fs.FileExists(filePath.c_str()))
		filePath = filePath.substr(0,ext)+".dx90.vtx";
	LoadVTX(filePath.c_str(),mdl,mdlData,lod);

	mdl->vbo.Create();
	mdl->vbo.Upload(mdl->vertexFormat[0].stride*mdl->vertexCount,mdl->verts);
	mdl->ibo.Create();
	mdl->ibo.Upload(mdl->indexSize*mdl->indexCount,mdl->inds);

	mdl->submeshes.Resize(mdlData.numMeshes);
	for(int i=0;i<mdlData.numMeshes;i++){
		mdl->submeshes[i]={(uint32_t)mdlData.meshes[i].indOffs,mdlData.meshes[i].indCount,i};
	}

	mdl->skeleton.Resize(mdlData.bones.size);
	for(int i=0;i<mdlData.bones.size;i++){
		mdl->skeleton[i] = mdlData.bones[i];
	}

	mdl->animations.Resize(mdlData.anims.size);
	for(int i=0;i<mdlData.anims.size;i++){
		mdl->animations[i].name = mdlData.anims[i].name;
		mdl->animations[i].fps = mdlData.anims[i].fps;
		mdl->animations[i].frames.Resize(mdlData.anims[i].frames.size);
		for(int f=0;f<mdlData.anims[i].frames.size;f++){
			mdl->animations[i].frames[f].bones.Resize(mdlData.anims[i].frames[f].bones.size);
			for(int b=0;b<mdlData.anims[i].frames[f].bones.size;b++){
				mdl->animations[i].frames[f].bones[b] = mdlData.anims[i].frames[f].bones[b];
			}
		}
	}

	return mdl;
}

bool MDLLoader::CheckExt(const char *name){
	return strstr(name,".mdl")!=0;
}

#if 1
bool MDLLoader::LoadMDL(const char *name, loadMDLOut_t &mdlData){
	IFile *mdlfile = g_fs.Open(name);
	if(!mdlfile){
		return false;
	}

	uint32_t fileLen = mdlfile->GetLen();
	Log("MDL: file len %d\n",fileLen);
	char *data = new char[fileLen];
	mdlfile->Seek(0);
	mdlfile->Read(data,fileLen);
	g_fs.Close(mdlfile);
	studiohdr_t *mdlHeader = (studiohdr_t*)data;

	if(mdlHeader->id != IDSTUDIOHEADER){
		Log("MDL: wrongh id(%X, must be %X)\n",mdlHeader->id,IDSTUDIOHEADER);
		return false;
	}
	if(mdlHeader->length != fileLen){
		Log("MDL: wrongh length(%d)\n",mdlHeader->length);
		return false;
	}

	if(mdlHeader->numbodyparts != 1)
		Log("MDL: %s num bodyparts %d\n", name, mdlHeader->numbodyparts);

	Log("MDL: numincludemodels %d\n",mdlHeader->numincludemodels);
	if(mdlHeader->numincludemodels){
		mstudiomodelgroup_t *incs = (mstudiomodelgroup_t*)(data+mdlHeader->includemodelindex);
		for(int i=0; i<mdlHeader->numincludemodels; i++){
			Log( "include model %d: label %s, name %s\n",i,incs[i].pszLabel(),incs[i].pszName());
		}
	}
	if(mdlHeader->szanimblocknameindex)
		Log("anim block name %s\n",data+mdlHeader->szanimblocknameindex);

	Log("MDL: num bones %d\n",mdlHeader->numbones);
	mdlData.bones.Resize(mdlHeader->numbones);
	mstudiobone_t *bones = (mstudiobone_t*)(data+mdlHeader->boneindex);
	for(int i=0; i<mdlHeader->numbones; i++){
		mstudiobone_t &b = bones[i];
		char *bonename = data+mdlHeader->boneindex+sizeof(mstudiobone_t)*i+b.sznameindex;
		Log( "bone %d: name %s, parent %d, pos (%.3f,%.3f,%.3f), rot(%.3f,%.3f,%.3f)\n",i,bonename,b.parent,b.pos.x,b.pos.y,b.pos.z, b.rot.x,b.rot.y,b.rot.z);
		mdlData.bones[i]={b.parent,b.pos,b.quat};
	}

	Log("MDL: numtextures %d, numcdtextures %d\n",mdlHeader->numtextures,mdlHeader->numcdtextures);
	mstudiotexture_t *texs = (mstudiotexture_t*)(data+mdlHeader->textureindex);
	for(int i=0; i<mdlHeader->numtextures; i++){
		Log("tex %d: %s\n",i,texs[i].pszName());
	}
	
	Log("MDL: numlocalanim %d, numlocalseq %d, numanimblocks %d\n",mdlHeader->numlocalanim,mdlHeader->numlocalseq,mdlHeader->numanimblocks);
	if(mdlHeader->numlocalseq){
		mstudioseqdesc_t *seqs = (mstudioseqdesc_t*)(data+mdlHeader->localseqindex);
		for(int i=0; i<mdlHeader->numlocalseq; i++){
			Log( "seq %d: label %s, activity name %s\n",i,seqs[i].pszLabel(),seqs[i].pszActivityName());
		}
	}
	if(mdlHeader->numlocalanim){
		mstudioanimdesc_t *anims = (mstudioanimdesc_t*)(data+mdlHeader->localanimindex);
		mdlData.anims.Resize(mdlHeader->numlocalanim);
		for(int i=0; i<mdlHeader->numlocalanim; i++){
			mstudioanimdesc_t &ad=anims[i];
			Log( "animdesc %d: label %s, num frames %d, fps %.3f, flags %d, block %d, index %d, section frames %d\n",i,ad.pszName(),ad.numframes,ad.fps,ad.flags,ad.animblock,ad.animindex,ad.sectionframes);
			//if(i<4)
			{
				mdlData.anims[i].name = ad.pszName();
				mdlData.anims[i].fps = ad.fps;
				mdlData.anims[i].frames.Resize(ad.numframes);

				for(int f=0;f<ad.numframes;f++){
					mdlData.anims[i].frames[f].bones.Resize(mdlHeader->numbones);
				}
				mstudioanim_t *pAnim = (mstudioanim_t*)((char*)(anims+i)+anims[i].animindex);
				for(int b=0;b<mdlHeader->numbones;b++){
					for(int f=0;f<ad.numframes;f++){
						bone_t bone;
						//bone.parent = mdlData.bones[b].parent;
						bone = mdlData.bones[b];
						if(pAnim&&pAnim->bone==b){
							//if(f==0/* && i==1*/) Log("anim %p: bone %d, flags %d, next %d\n",pAnim,pAnim->bone,pAnim->flags,pAnim->nextoffset);
							if(pAnim->flags&STUDIO_ANIM_RAWPOS){
								bone.pos = pAnim->PosVec3();
							}
							//TODO: STUDIO_ANIM_RAWROT
							if(pAnim->flags&STUDIO_ANIM_RAWROT2)
								bone.rot = pAnim->Quat();
							if(pAnim->flags&STUDIO_ANIM_ANIMROT){
								vec3 animAng=bones[b].rot;
								mstudioanim_valueptr_t *rotV = pAnim->pRotV();
								for(int j=0;j<3;j++){
									mstudioanimvalue_t *animVal = rotV->pAnimvalue(j);
									if(animVal){
										int k = glm::min(f,animVal->num.valid-1);
										animAng[j] += animVal[k+1].value*bones[b].rotscale[j];
									}
								}
								bone.rot = glm::quat(animAng);
							}
							if(pAnim->flags&STUDIO_ANIM_ANIMPOS){
								vec3 animPos=bones[b].pos;
								mstudioanim_valueptr_t *posV = pAnim->pPosV();
								for(int j=0;j<3;j++){
									mstudioanimvalue_t *animVal = posV->pAnimvalue(j);
									if(animVal){
										int k = glm::min(f,animVal->num.valid-1);
										//Log("animVal %d: %d num %d %d\n",j,animVal[1].value,animVal->num.total,animVal->num.valid);
										animPos[j] += animVal[k+1].value*bones[b].posscale[j];
									}
								}
								bone.pos = animPos;
							}
						}else{
							bone = mdlData.bones[b];
						}

						mdlData.anims[i].frames[f].bones[b] = bone;
					}
					if(pAnim&&pAnim->bone==b)
						pAnim = pAnim->pNext();
				}
			}
		}
	}

	mstudiobodyparts_t *bodyparts = (mstudiobodyparts_t*)(data+mdlHeader->bodypartindex);
	//for(int bpi = 0; bpi<mdlHeader->numbodyparts; bpi++)
	int bpi = 0;
	{
		mstudiobodyparts_t &bp = bodyparts[bpi];
		if(bp.nummodels!=1)
			Log("MDL: %s bodyparts[%d] num models %d\n", name, bpi, bp.nummodels);

		mstudiomodel_t *bpmodels = (mstudiomodel_t*)((char*)(bodyparts+bpi)+bp.modelindex);

		//for(int mi = 0;mi<bp.nummodels; mi++)
		int mi = 0;
		{
			mstudiomodel_t &bpm = bpmodels[mi];
			mdlData.numMeshes = bpm.nummeshes;
			if(bpm.nummeshes<=0){
				Log( "  mdl model %d without meshes",bpi);
			}else{
				Log("MDL: nummeshes %d\n",bpm.nummeshes);
				mstudiomesh_t *mdlMeshes = (mstudiomesh_t*)((char*)(bpmodels+mi)+bpm.meshindex);
				mdlData.meshes = new loadMDLOut_t::mdlOutMesh_t[bpm.nummeshes];
				for(int mshi = 0; mshi<bpm.nummeshes; mshi++){
					mstudiomesh_t &msh = mdlMeshes[mshi];
					//Log("  mesh %d: material %d, vertices: num %d offset %d\n",mshi,msh.material, msh.numvertices, msh.vertexoffset);

					//get materials

					mdlData.meshes[mshi].vertOffs=msh.vertexoffset;
					mdlData.meshes[mshi].vertCount=msh.numvertices;
				}
			}
		}
	}

	//g_fs.Close(mdlfile);
	delete[] data;
	return true;
}

#else
//OLD variant
bool MDLLoader::LoadMDL(const char *name, loadMDLOut_t &mdlData){
	IFile *mdlfile = g_fs.Open(name);
	if(!mdlfile){
		return false;
	}

	studiohdr_t mdlHeader;
	mdlfile->Read(&mdlHeader,sizeof(studiohdr_t));

	if(mdlHeader.id != IDSTUDIOHEADER)
		return false;

	if(mdlHeader.numbodyparts != 1)
		Log("MDL: %s num bodyparts %d\n", name, mdlHeader.numbodyparts);

	Log("MDL: num bones %d\n",mdlHeader.numbones);
	mdlData.bones.Resize(mdlHeader.numbones);
	mstudiobone_t *bones = new mstudiobone_t[mdlHeader.numbones];
	mdlfile->Seek(mdlHeader.boneindex);
	mdlfile->Read(bones,sizeof(mstudiobone_t)*mdlHeader.numbones);
	for(int b=0; b<mdlHeader.numbones; b++){
		/*char bonename[64];
		mdlfile->Seek(mdlHeader.boneindex+sizeof(mstudiobone_t)*b+bones[b].sznameindex);
		mdlfile->GetString(bonename,64);
		Log( "bone %d: name %s, parent %d, pos (%f,%f,%f), rot(%f,%f,%f)\n",b,bonename,bones[b].parent,bones[b].pos.x,bones[b].pos.y,bones[b].pos.z, bones[b].rot.x,bones[b].rot.y,bones[b].rot.z);
		*/
		mdlData.bones[b]={bones[b].parent,bones[b].pos,bones[b].quat};
	}
	delete[] bones;

	Log("MDL: numhitboxsets %d\n",mdlHeader.numhitboxsets);
	if(mdlHeader.numhitboxsets){
		mstudiohitboxset_t *hbsets = new mstudiohitboxset_t[mdlHeader.numhitboxsets];
		mdlfile->Seek(mdlHeader.hitboxsetindex);
		mdlfile->Read(hbsets,sizeof(mstudiohitboxset_t)*mdlHeader.numhitboxsets);
		for(int i=0; i<mdlHeader.numhitboxsets; i++){
			char temp[64];
			mdlfile->Seek(mdlHeader.hitboxsetindex+sizeof(mstudiohitboxset_t)*i+hbsets[i].sznameindex);
			mdlfile->GetString(temp,64);
			Log("hitbox std %d: %s num %d\n",i,temp,hbsets[i].numhitboxes);
		}

		mstudiobbox_t *hitboxes = new mstudiobbox_t[hbsets[0].numhitboxes];
		mdlfile->Seek(mdlHeader.hitboxsetindex+hbsets[0].hitboxindex);
		mdlfile->Read(hitboxes,sizeof(mstudiobbox_t)*hbsets[0].numhitboxes);
		for(int i=0;i<hbsets[0].numhitboxes;i++){
			char temp[64]="(null)";
			int of = sizeof(mstudiobbox_t)*i+hitboxes[i].szhitboxnameindex;
			if(hitboxes[i].szhitboxnameindex){
				mdlfile->Seek(mdlHeader.hitboxsetindex+hbsets[0].hitboxindex+of);
				mdlfile->GetString(temp,64);
			}
			Log("bbox %d: %s(%d) bone %d, min(%.2f,%.2f,%.2f) max(%.2f,%.2f,%.2f)\n",i,temp,of,hitboxes[i].bone,hitboxes[i].bbmin.x,hitboxes[i].bbmin.y,hitboxes[i].bbmin.z,hitboxes[i].bbmax.x,hitboxes[i].bbmax.y,hitboxes[i].bbmax.z);
		}
		delete[] hitboxes;
		delete[] hbsets;
	}

	Log("MDL: numlocalanim %d, numlocalseq %d\n",mdlHeader.numlocalanim,mdlHeader.numlocalseq);
	if(mdlHeader.numlocalseq){
		mstudioseqdesc_t *seqs = new mstudioseqdesc_t[mdlHeader.numlocalseq];
		mdlfile->Seek(mdlHeader.localseqindex);
		mdlfile->Read(seqs,sizeof(mstudioseqdesc_t)*mdlHeader.numlocalseq);
		for(int i=0; i<mdlHeader.numlocalseq; i++){
			char temp[64];
			char temp2[64];
			mdlfile->Seek(mdlHeader.localseqindex+sizeof(mstudioseqdesc_t)*i+seqs[i].szlabelindex);
			mdlfile->GetString(temp,64);
			mdlfile->Seek(mdlHeader.localseqindex+sizeof(mstudioseqdesc_t)*i+seqs[i].szactivitynameindex);
			mdlfile->GetString(temp2,64);
			Log( "seq %d: label %s, activity name %s\n",i,temp,temp2);
		}
		delete[] seqs;
	}
	if(mdlHeader.numlocalanim){
		mstudioanimdesc_t *anims = new mstudioanimdesc_t[mdlHeader.numlocalanim];
		mdlfile->Seek(mdlHeader.localanimindex);
		mdlfile->Read(anims,sizeof(mstudioanimdesc_t)*mdlHeader.numlocalanim);
		for(int i=0; i<mdlHeader.numlocalanim; i++){
			mstudioanimdesc_t &ad=anims[i];
			char temp[64];
			mdlfile->Seek(mdlHeader.localanimindex+sizeof(mstudioanimdesc_t)*i+ad.sznameindex);
			mdlfile->GetString(temp,64);
			Log( "animdesc %d: label %s, num franes %d, block %d, index %d, section frames %d\n",i,temp,ad.numframes,ad.animblock,ad.animindex,ad.sectionframes);
		}
		mstudioanim_t anim0 = {};
		mdlfile->Seek(mdlHeader.localanimindex+anims[0].animindex);
		mdlfile->Read(&anim0,sizeof(mstudioanim_t));
		Log("anim 0: bone %d, flags %d, next %d\n",anim0.bone,anim0.flags,anim0.nextoffset);

		delete[] anims;
	}

	Log("MDL: numtextures %d, numcdtextures %d\n",mdlHeader.numtextures,mdlHeader.numcdtextures);

	Log("MDL: numincludemodels %d\n",mdlHeader.numincludemodels);
	if(mdlHeader.numincludemodels){
		mstudiomodelgroup_t *incs = new mstudiomodelgroup_t[mdlHeader.numincludemodels];
		mdlfile->Seek(mdlHeader.includemodelindex);
		mdlfile->Read(incs,sizeof(mstudiomodelgroup_t)*mdlHeader.numincludemodels);
		for(int i=0; i<mdlHeader.numincludemodels; i++){
			char temp[64];
			char temp2[64];
			mdlfile->Seek(mdlHeader.includemodelindex+sizeof(mstudiomodelgroup_t)*i+incs[i].szlabelindex);
			mdlfile->GetString(temp,64);
			mdlfile->Seek(mdlHeader.includemodelindex+sizeof(mstudiomodelgroup_t)*i+incs[i].sznameindex);
			mdlfile->GetString(temp2,64);
			Log( "include model %d: label %s, name %s\n",i,temp,temp2);
		}
		delete[] incs;
	}

	mstudiobodyparts_t *bodyparts = new mstudiobodyparts_t[mdlHeader.numbodyparts];
	mdlfile->Seek(mdlHeader.bodypartindex);
	mdlfile->Read(bodyparts,sizeof(mstudiobodyparts_t)*mdlHeader.numbodyparts);

	//for(int bpi = 0; bpi<mdlHeader.numbodyparts; bpi++)
	int bpi = 0;
	{
		mstudiobodyparts_t &bp = bodyparts[bpi];
		if(bp.nummodels!=1)
			Log("MDL: %s bodyparts[%d] num models %d\n", name, bpi, bp.nummodels);

		mstudiomodel_t *bpmodels = new mstudiomodel_t[bp.nummodels];
		memset(bpmodels,0,sizeof(mstudiomodel_t));
		mdlfile->Seek(mdlHeader.bodypartindex+(sizeof(mstudiobodyparts_t)*bpi)+bp.modelindex);
		mdlfile->Read(bpmodels,sizeof(mstudiomodel_t)*bp.nummodels);
		//for(int mi = 0;mi<bp.nummodels; mi++)
		int mi = 0;
		{
			mstudiomodel_t &bpm = bpmodels[mi];
			mdlData.numMeshes = bpm.nummeshes;
			if(bpm.nummeshes<=0){
				Log( "  mdl model %d without meshes",bpi);
			}else{
				mstudiomesh_t *mdlMeshes = new mstudiomesh_t[bpm.nummeshes];
				mdlfile->Seek(mdlHeader.bodypartindex+
						(sizeof(mstudiobodyparts_t)*bpi)+bp.modelindex+
						(sizeof(mstudiomodel_t)*mi)+bpm.meshindex);
				mdlfile->Read(mdlMeshes,sizeof(mstudiomesh_t)*bpm.nummeshes);

				mdlData.meshes = new loadMDLOut_t::mdlOutMesh_t[bpm.nummeshes];
				for(int mshi = 0; mshi<bpm.nummeshes; mshi++){
					mstudiomesh_t &msh = mdlMeshes[mshi];
					//Log("  mesh %d: material %d, vertices: num %d offset %d\n",mshi,msh.material, msh.numvertices, msh.vertexoffset);

					//get materials

					mdlData.meshes[mshi].vertOffs=msh.vertexoffset;
					mdlData.meshes[mshi].vertCount=msh.numvertices;
				}

				//TODO replace this by small structure
				//bpm.meshindex = (int)mdlMeshes;
				delete[] mdlMeshes;
			}
		}
		delete[] bpmodels;
	}
	delete[] bodyparts;

	g_fs.Close(mdlfile);
	return true;
}
#endif

bool MDLLoader::LoadVVD(const char *name, Model *mdl, int lod)
{
	IFile *vvdfile = g_fs.Open(name);
	if(!vvdfile){
		return NULL;
	}

	vertexFileHeader_t vvdHeader;
	vvdfile->Read(&vvdHeader,sizeof(vertexFileHeader_t));
	//Log( "vvd header: version %d, numLODs %d, numLODVertexes[0] %d numFixups %d\n",vvdHeader.version, vvdHeader.numLODs, vvdHeader.numLODVertexes[0],vvdHeader.numFixups);
	if(lod >= vvdHeader.numLODs)
		lod = vvdHeader.numLODs-1;

	int numVerts = vvdHeader.numLODVertexes[lod];
	char *verts = new char[sizeof(mstudiovertex_t)*numVerts];
	if(!vvdHeader.numFixups){
		vvdfile->Seek(vvdHeader.vertexDataStart);
		vvdfile->Read(verts,sizeof(mstudiovertex_t)*numVerts);
	}else{
		//Log("Start fixup vertices %s\n",fileName);
		vertexFileFixup_t *fixupTable = new vertexFileFixup_t[vvdHeader.numFixups];
		vvdfile->Seek(vvdHeader.fixupTableStart);
		vvdfile->Read(fixupTable,sizeof(vertexFileFixup_t)*vvdHeader.numFixups);

		int target = 0;
		for(int f=0; f<vvdHeader.numFixups; f++){
			if(fixupTable[f].lod < lod)
				continue;
			//Log( "fixup %d, lod %d, id %d, num %d\n", f, fixupTable[f].lod, fixupTable[f].sourceVertexID, fixupTable[f].numVertexes);

			vvdfile->Seek(vvdHeader.vertexDataStart + fixupTable[f].sourceVertexID*sizeof(mstudiovertex_t));
			vvdfile->Read((verts+target*sizeof(mstudiovertex_t)),fixupTable[f].numVertexes*sizeof(mstudiovertex_t));
			target += fixupTable[f].numVertexes;
		}
		//Log( "read %d fixup verts\n",target);
		delete[] fixupTable;
	}
	Log( "VVD: read %d verts\n",numVerts);

	mdl->vertexCount = numVerts;
	mdl->vertexFormat.Resize(5);
	mdl->vertexFormat[0]=vertAttrib_t(0,3,GL_FLOAT,false,sizeof(mstudiovertex_t),16);//pos
	mdl->vertexFormat[1]=vertAttrib_t(1,3,GL_FLOAT,false,sizeof(mstudiovertex_t),28);//norm
	mdl->vertexFormat[2]=vertAttrib_t(2,2,GL_FLOAT,false,sizeof(mstudiovertex_t),40);//uv
	mdl->vertexFormat[3]=vertAttrib_t(3,3,GL_FLOAT,false,sizeof(mstudiovertex_t),0);//weight
	mdl->vertexFormat[4]=vertAttrib_t(4,4,GL_UNSIGNED_BYTE,false,sizeof(mstudiovertex_t),12);//bone ids
	mdl->verts = verts;

	g_fs.Close(vvdfile);
	return true;
}

int CalcVTXIndsCount(vtxFileHeader_t *vtxHeader, int lod){
	int numInds = 0;

	//for(int bpi=0; bpi<vtxHeader->numBodyParts; pbi++)
	int bpi = 0;
	{
		vtxBodyPartHeader_t *bp = (vtxBodyPartHeader_t*)((char*)vtxHeader+vtxHeader->bodyPartOffset)+bpi;
		//for(int mi=0; mi<bp->numModels; mi++)
		int mi = 0;
		{
			vtxModelHeader_t *model = (vtxModelHeader_t*)(((char*)bp)+bp->modelOffset)+mi;

			vtxModelLODHeader_t *lodHdr = (vtxModelLODHeader_t*)(((char*)model)+model->lodOffset)+lod;

			for(int mshi=0; mshi<lodHdr->numMeshes; mshi++)
			{
				vtxMeshHeader_t *msh = (vtxMeshHeader_t*)(((char*)lodHdr)+lodHdr->meshOffset)+mshi;
				//if(msh->numStripGroups!=1)
				//	Log("VTX: mesh %d numStripGroups %d\n",mshi,msh->numStripGroups);
				for(int sgi=0; sgi<msh->numStripGroups; sgi++)
				//int sgi=0;
				{
					vtxStripGroupHeader_t *sg = ((vtxStripGroupHeader_t*)(((char*)msh)+msh->stripGroupHeaderOffset))+sgi;

					numInds += sg->numIndices;
				}
			}
		}
	}

	return numInds;
}

bool MDLLoader::LoadVTX(const char *name, Model *mdl, loadMDLOut_t &mdlData, int lod){
	IFile *vtxfile = g_fs.Open(name);
	if(!vtxfile){
		return false;
	}

	char *data = new char[vtxfile->GetLen()];
	vtxfile->Seek(0);
	vtxfile->Read(data,vtxfile->GetLen());
	g_fs.Close(vtxfile);

	char *inds = NULL;
	int numInds = 0;

	vtxFileHeader_t *vtxHeader = (vtxFileHeader_t*)data;
	//Log( "vtx header: version %d, numLODs %d, bodyparts num %d offset 0x%x\n",vtxHeader.version, vtxHeader.numLODs, vtxHeader.numBodyParts, vtxHeader.bodyPartOffset);

	numInds = CalcVTXIndsCount(vtxHeader, lod);
	Log("VTX: inds num %d\n",numInds);
	inds = new char[numInds*sizeof(uint32_t)];

	int curInd = 0;

	//for(int bpi=0; bpi<vtxHeader->numBodyParts; pbi++)
	int bpi = 0;
	{
		vtxBodyPartHeader_t *bp = (vtxBodyPartHeader_t*)(data+vtxHeader->bodyPartOffset)+bpi;
		//for(int mi=0; mi<bp->numModels; mi++)
		int mi = 0;
		{
			vtxModelHeader_t *model = (vtxModelHeader_t*)(((char*)bp)+bp->modelOffset)+mi;

			vtxModelLODHeader_t *lodHdr = (vtxModelLODHeader_t*)(((char*)model)+model->lodOffset)+lod;

			for(int mshi=0; mshi<lodHdr->numMeshes; mshi++)
			//int mshi=0;
			{
				vtxMeshHeader_t *msh = (vtxMeshHeader_t*)(((char*)lodHdr)+lodHdr->meshOffset)+mshi;
				//Log("VTX: mesh %d: numStripGroups %d, flags %d\n",mshi,msh->numStripGroups,msh->flags);

				mdlData.meshes[mshi].indOffs = curInd;
				mdlData.meshes[mshi].indCount = 0;
				for(int sgi=0; sgi<msh->numStripGroups; sgi++)
				//int sgi=0;
				{
					vtxStripGroupHeader_t *sg = ((vtxStripGroupHeader_t*)(((char*)msh)+msh->stripGroupHeaderOffset))+sgi;

					//Log( "    vtx strip group %d: Verts num %d offset 0x%x, Indices num %d offset 0x%x, Strips num %d offset 0x%x, flags %d\n",sgi,sg->numVerts,sg->vertOffset,sg->numIndices,sg->indexOffset,sg->numStrips,sg->stripOffset,sg->flags);
					vtxVertex_t *vtxVerts = (vtxVertex_t*)(((char*)sg)+sg->vertOffset);
					uint16_t *vtxInds = (uint16_t*)(((char*)sg)+sg->indexOffset);

					for(int ind=0; ind<sg->numIndices; ind++){
						//int vertOffs = ((mstudiomesh_t*)((mstudiomodel_t*)bodyparts[bph].modelindex)[mh].meshindex)[msh].vertexoffset;
						int vertOffs = mdlData.meshes[mshi].vertOffs;
						((uint32_t*)inds)[curInd+ind] = vertOffs + vtxVerts[vtxInds[ind]].origMeshVertID;
					}
					curInd += sg->numIndices;
					mdlData.meshes[mshi].indCount += sg->numIndices;
				}
			}
		}
	}

	delete[] data;

	mdl->inds = inds;
	mdl->indexCount = numInds;
	mdl->indexType = GL_UNSIGNED_INT;
	mdl->indexSize = 4;

	return true;
}
