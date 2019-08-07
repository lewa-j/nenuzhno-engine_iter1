
#include <string.h>

#include "log.h"
#include "graphics/platform_gl.h"
#include "graphics/texture.h"

void DecompressDXT(Texture *tex, const GLubyte *data, GLubyte *out, int texSize, int inFormat);
void UploadDXT(Texture *tex, const GLubyte *data, int texSize, int inFormat);

Texture::Texture()
{
	width = 0;
	height = 0;
	id = 0;
	target = GL_TEXTURE_2D;
	type = GL_UNSIGNED_BYTE;
	fmt = GL_RGB;
	infmt = GL_RGB;
}

Texture::~Texture(){
	if(id)
		glDeleteTextures(1,&id);
}

bool Texture::Create(int w, int h)
{
	glGenTextures(1, &id);
	if(!id)
		return false;
	//LOG("Created texture(%dx%d) %d\n", w, h, id);
	glBindTexture(target, id);
	/*
	glTexImage2D(target, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, NULL);
	if(glGetError())
		return false;
	*/
	width = w;
	height = h;
	SetWrap(GL_CLAMP_TO_EDGE);
	SetFilter(GL_NEAREST, GL_NEAREST);

	//glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void Texture::Bind()
{
	glBindTexture(target, id);
}

void Texture::SetWrap(int wrap)
{
	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
}

void Texture::SetFilter(int min, int mag)
{
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag);
}

//TODO fix levels
void Texture::Upload(GLint level, GLuint format, const void *data)
{
	fmt=format;
	infmt=format;

	glTexImage2D(target, level, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
}

void Texture::Upload(GLint level, GLuint iformat, GLuint format, GLuint tp, const void *data)
{
	infmt=iformat;
	fmt=format;
	type=tp;
	
	glTexImage2D(target, level, iformat, width>>level, height>>level, 0, format, type, data);
}

void Texture::Upload(int lvl, int w, int h, const void *data)
{
	if(lvl==0){
		width=w;
		height=h;
	}
	glTexImage2D(target, lvl, infmt, w, h, 0, fmt, type, data);
}

void Texture::UploadCompressed(GLuint iformat, int size, const void *data)
{
#ifdef ANDROID
	if(iformat==GL_COMPRESSED_RGB_S3TC_DXT1||
		iformat==GL_COMPRESSED_RGBA_S3TC_DXT5)
	{
		UploadDXT(this, (const GLubyte*)data, size, iformat);
		return;
	}
#endif
	infmt = iformat;
	glCompressedTexImage2D(target, 0, iformat, width, height, 0, size, data);
}

void ResampleBGR(uint8_t *data, int size)
{
	int t;
	for(int i=0; i<size; i+=3)
	{
		t = data[i];
		data[i]=data[i+2];
		data[i+2]=t;
	}
}

void ResampleBGRA(uint8_t *data, int size)
{
	int t;
	for(int i=0; i<size; i+=4)
	{
		t = data[i];
		data[i]=data[i+2];
		data[i+2]=t;
	}
}

//DXT decompression

void UploadDXT(Texture *tex, const GLubyte *data, int texSize, int inFormat)
{
	int newSize;
	int format;
	if(inFormat==GL_COMPRESSED_RGB_S3TC_DXT1)
	{
		newSize = texSize*6;
		format = GL_RGB;
	}
	else if(inFormat==GL_COMPRESSED_RGBA_S3TC_DXT5)
	{
		newSize = texSize*4;
		format = GL_RGBA;
	}
	GLubyte *newData = new GLubyte[newSize];
	DecompressDXT(tex,data,newData,texSize,inFormat);
	tex->Upload(0, format, newData);
	delete[] newData;
}

void DecompressDXT(Texture *tex, const GLubyte *data, GLubyte *out, int texSize, int inFormat)
{
	//Log("Software dxt decompressor: s %d f %d %dx%d\n",texSize,inFormat,tex->width,tex->height);
	int blockSize;
	int colOffs;
	int stride;
	int outputStride;
	if(inFormat==GL_COMPRESSED_RGB_S3TC_DXT1)
	{
		blockSize = 8;
		stride = 3;
		outputStride = tex->width*3;
		colOffs = 0;
	}
	else //dxt5
	{
		blockSize = 16;
		stride = 4;
		outputStride = tex->width*4;
		colOffs = 8;
	}

	for(int i=0; i<(int)texSize/blockSize; i++)
	{
		GLushort col0 = *(GLushort*)(data+(i*blockSize+colOffs));
		GLushort col1 = *(GLushort*)(data+i*blockSize+colOffs+2);
		uint64_t code = *(GLuint*)(data+i*blockSize+colOffs+4);
		int offs = i*4*stride + i/(tex->width/4)*outputStride*3;

		GLubyte r0 = (col0>>11)&0x1F;
		GLubyte g0 = (col0>>5)&0x3F;
		GLubyte b0 = col0&0x1F;
		r0 = (r0<<3)|(r0>>2);
		g0 = (g0<<2)|(g0>>4);
		b0 = (b0<<3)|(b0>>2);
		GLubyte r1 = (col1>>11)&0x1F;
		GLubyte g1 = (col1>>5)&0x3F;
		GLubyte b1 = col1&0x1F;
		r1 = (r1<<3)|(r1>>2);
		g1 = (g1<<2)|(g1>>4);
		b1 = (b1<<3)|(b1>>2);
		GLubyte r,g,b;

		GLubyte poscode;
		for(int y=0;y<4;y++)
		{
			for(int x=0;x<4;x++)
			{
				poscode = code>>2*(4*y+x)&3;
				if(col0>col1)
				{
					switch(poscode)
					{
					case 0:
						r = r0;
						g = g0;
						b = b0;
						break;
					case 1:
						r = r1;
						g = g1;
						b = b1;
						break;
					case 2:
						r = (2*r0+r1)/3;
						g = (2*g0+g1)/3;
						b = (2*b0+b1)/3;
						break;
					case 3:
						r = (r0+2*r1)/3;
						g = (g0+2*g1)/3;
						b = (b0+2*b1)/3;
						break;
					}
				}
				else
				{
					switch(poscode)
					{
						case 0:
							r = r0;
							g = g0;
							b = b0;
							break;
						case 1:
							r = r1;
							g = g1;
							b = b1;
							break;
						case 2:
							r = (r0+r1)/2;
							g = (g0+g1)/2;
							b = (b0+b1)/2;
							break;
						case 3:
							r = g = b = 0;
							break;
					}
				}

				out[offs+y*outputStride+x*stride] = r;
				out[offs+y*outputStride+x*stride+1] = g;
				out[offs+y*outputStride+x*stride+2] = b;
			}
		}

		if(inFormat==GL_COMPRESSED_RGB_S3TC_DXT1)
			continue;

		GLubyte a0 = *(data+i*blockSize);
		GLubyte a1 = *(data+i*blockSize+1);
		code = *(uint64_t*)(data+i*blockSize);
		GLubyte a[8];

		a[0] = a0;
		a[1] = a1;

		/*if(a0>a1)
		{
			a[2] = (6.0f*a0 + a1)/7.0f;
			a[3] = (5.0f*a0 + 2.0f*a1)/7.0f;
			a[4] = (4.0f*a0 + 3.0f*a1)/7.0f;
			a[5] = (3.0f*a0 + 4.0f*a1)/7.0f;
			a[6] = (2.0f*a0 + 5.0f*a1)/7.0f;
			a[7] = (a0 + 6.0f*a1)/7.0f;
		}
		else
		{
			a[2] = (4.0f*a0+a1)/5.0f;
			a[3] = (3.0f*a0+2.0f*a1)/5.0f;
			a[4] = (2.0f*a0+3.0f*a1)/5.0f;
			a[5] = (a0+4.0f*a1)/5.0f;
			a[6] = 0;
			a[7] = 1.0f;
		}*/
		if( a0 <= a1 )
		{
			// use 5-alpha codebook
			for( int j = 1; j < 5; ++j )
				a[1 + j] = ( ( ( 5 - j )*a0 + j*a1 )/5 );
			a[6] = 0;
			a[7] = 255;
		}
		else
		{
			// use 7-alpha codebook
			for( int j = 1; j < 7; ++j )
				a[1 + j] = ( ( ( 7 - j )*a0 + j*a1 )/7 );
		}

		for(int y=0;y<4;y++)
		{
			for(int x=0;x<4;x++)
			{
				poscode = (code >> (16+3*(4*y+x))) & 0x07;
				out[offs+y*outputStride+x*stride+3] = a[poscode];
			}
		}
	}
}
