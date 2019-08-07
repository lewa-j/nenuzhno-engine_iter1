
#pragma once

#include "resource/ResourceManager.h"

#define DDS_IDENT 542327876
#define DDS_DXT1 827611204
#define DDS_DXT5 894720068

struct DDS_PIXELFORMAT
{
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
};

struct DDS_HEADER
{
  uint32_t           dwSize;
  uint32_t           dwFlags;
  uint32_t           dwHeight;
  uint32_t           dwWidth;
  uint32_t           dwPitchOrLinearSize;
  uint32_t           dwDepth;
  uint32_t           dwMipMapCount;
  uint32_t           dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32_t           dwCaps;
  uint32_t           dwCaps2;
  uint32_t           dwCaps3;
  uint32_t           dwCaps4;
  uint32_t           dwReserved2;
};

class DDSLoader: public ITextureLoader{
public:
	virtual Texture *Load(const char *name);
	virtual bool CheckExt(const char *name);
	virtual const char *GetExt(){return "dds";}
};
