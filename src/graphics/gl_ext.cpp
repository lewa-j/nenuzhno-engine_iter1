
#include <string.h>
#include "log.h"
#include "graphics/gl_ext.h"
#include "graphics/platform_gl.h"
#include "graphics/gl_utils.h"
//#include "gl/wglew.h"

uint64_t GLExtensions::extFlags;

const char *GetComprFormatName(int f){
	if(f==0x83F0)
		return "RGB_S3TC_DXT1";
	if(f==0x83F1)
		return "RGBA_S3TC_DXT1";
	if(f==0x83F2)
		return "RGBA_S3TC_DXT3";
	if(f==0x83F3)
		return "RGBA_S3TC_DXT5";
	if(f==0x9270)
		return "R11_EAC";
	if(f==0x9271)
		return "R11_EAC";
	if(f==0x9272)
		return "RG11_EAC";
	if(f==0x9273)
		return "SIGNED_RG11_EAC";
	if(f==0x8D64)
		return "ETC1_RGB8";
	if(f==0x9274)
		return "RGB8_ETC2";
	if(f==0x9275)
		return "SRGB8_ETC2";
	if(f==0x9276)
		return "RGB8_PUNCHTHROUGH_ALPHA1_ETC2";
	if(f==0x9277)
		return "SRGB8_PUNCHTHROUGH_ALPHA1_ETC2";
	if(f==0x9278)
		return "RGBA8_ETC2_EAC";
	if(f==0x9279)
		return "SRGB8_ALPHA8_ETC2_EAC";
	if(f==0x8E8C)
		return "RGBA_BPTC_UNORM";
	if(f==0x8E8D)
		return "SRGB_ALPHA_BPTC_UNORM";
	if(f==0x8E8E)
		return "RGB_BPTC_SIGNED_FLOAT";
	if(f==0x8E8F)
		return "RGB_BPTC_UNSIGNED_FLOAT";
	if(f==0x8C00)
		return "RGB_PVRTC_4BPPV1_IMG";
	if(f==0x8C01)
		return "RGB_PVRTC_2BPPV1_IMG";
	if(f==0x8C02)
		return "RGBA_PVRTC_4BPPV1_IMG";
	if(f==0x8C03)
		return "RGBA_PVRTC_2BPPV1_IMG";
	return 0;
}

void GLExtensions::Init()
{
	extFlags = 0;

	const char *glVer = (const char *)glGetString(GL_VERSION);
	if(strstr(glVer,"ES"))
		extFlags |= eGLES;
	if(strstr(glVer,"Core"))
		extFlags |= eCore;

	int numExts = 0;
	const char *extString = 0;
	const char *t = 0;
	char ext[256]={0};

	if(extFlags&eCore){
#ifndef ANDROID
		//gl 3.0 way
		glGetIntegerv(GL_NUM_EXTENSIONS,&numExts);
#endif
	}else{
		extString = (const char *)glGetString(GL_EXTENSIONS);
		t = extString;
		
		const char *end = 0;
		while(*t){
			end = strstr(t," ");
			if(end){
				strncpy(ext,t,end-t);
				ext[end-t]=0;
			}
			else
				break;
			t=end+1;
			numExts++;
		}
		t = extString;
	}
	Log("Found %d extensions\n",numExts);

	for(int i=0; i<numExts; i++){
		const char *ex = 0;

		if(extFlags&eCore){
#ifndef ANDROID
			ex = (const char *)glGetStringi(GL_EXTENSIONS,i);
#endif
		}else{
			const char *end = strstr(t," ");
			strncpy(ext,t,end-t);
			ext[end-t] = 0;
			t=end+1;
			ex = ext;
		}
		//Log("ext %d: %s\n",i,ex);

		if(!strcmp(ex,"GL_ARB_depth_texture")
				||!strcmp(ex,"GL_SGIX_depth_texture")
				||!strcmp(ex,"GL_OES_depth_texture"))
			extFlags |= eDepth_texture;
		else
		if(!strcmp(ex,"GL_ARB_vertex_array_object")
				||!strcmp(ex,"GL_OES_vertex_array_object")){
			extFlags |= eVertex_array_object;
		}else
		if(!strcmp(ex,"GL_ARB_texture_non_power_of_two")
				||!strcmp(ex,"GL_OES_texture_npot"))
			extFlags |= eTexture_npot;
		else
		if(!strcmp(ex,"GL_EXT_texture_compression_s3tc")
				||!strcmp(ex,"GL_NV_texture_compression_s3tc")
				||!strcmp(ex,"GL_S3_s3tc"))
			extFlags |= eTexture_compression_s3tc;
		else
		if(!strcmp(ex,"GL_EXT_packed_depth_stencil")
				||!strcmp(ex,"GL_NV_packed_depth_stencil")
				||!strcmp(ex,"GL_OES_packed_depth_stencil"))
			extFlags |= ePacked_depth_stencil;
		else
		if(!strcmp(ex,"GL_EXT_texture_filter_anisotropic"))
			extFlags |= eTexture_filter_anisotropic;
		else
		if(!strcmp(ex,"GL_OES_compressed_ETC1_RGB8_texture"))
			extFlags |= eCompressed_ETC1_RGB8_texture;
		else
		if(!strcmp(ex,"GL_OES_depth24"))
			extFlags |= eDepth24;
		else
		if(!strcmp(ex,"GL_OES_depth32"))
			extFlags |= eDepth32;
		else
		if(!strcmp(ex,"GL_OES_element_index_uint"))
			extFlags |= eElement_index_uint;
		else
		if(!strcmp(ex,"GL_OES_rgb8_rgba8"))
			extFlags |= eFBO_rgb8_rgba8;
		else
		if(!strcmp(ex,"GL_OES_rgb8_rgba8")
				||strcmp(ex,"GL_EXT_bgra"))
			extFlags |= eTexture_format_BGRA8888;
		else
		if(!strcmp(ex,"GL_OES_get_program_binary")
				||strcmp(ex,"GL_ARB_get_program_binary"))
			extFlags |= eProgBin;
	}

	if(extFlags & eVertex_array_object){
		InitVAO();
	}
	if(extFlags & eProgBin){
		InitBinProg();
	}

	//GL_ARB_depth_texture GL_SGIX_depth_texture GL_OES_depth_texture
	//GL_OES_depth_texture_cube_map
	//GL_ARB_ES2_compatibility
	//GL_ARB_half_float_pixel GL_ARB_half_float_vertex GL_NV_half_float GL_EXT_color_buffer_half_float GL_OES_texture_half_float GL_OES_texture_half_float_linear GL_OES_vertex_half_float
	//GL_ARB_point_parameters GL_EXT_point_parameters
	//GL_ARB_point_sprite GL_NV_point_sprite
	//GL_ARB_seamless_cube_map
	//GL_ARB_texture_float GL_ATI_texture_float
	//GL_ARB_texture_non_power_of_two GL_OES_texture_npot
	//GL_ARB_vertex_array_object GL_OES_vertex_array_object
	//GL_S3_s3tc GL_EXT_texture_compression_dxt1 GL_EXT_texture_compression_s3tc GL_NV_texture_compression_s3tc GL_NV_texture_compression_s3tc_update
	//GL_EXT_packed_depth_stencil GL_NV_packed_depth_stencil GL_OES_packed_depth_stencil
	//GL_EXT_texture3D
	//GL_EXT_texture_filter_anisotropic
	//GL_ARB_depth_buffer_float GL_NV_depth_buffer_float
	//GL_SGIS_generate_mipmap
	//GL_OES_compressed_ETC1_RGB8_texture
	//GL_OES_depth24
	//GL_OES_depth32
	//GL_OES_element_index_uint
	//GL_OES_fbo_render_mipmap
	//GL_OES_rgb8_rgba8
	//GL_EXT_texture_format_BGRA8888 GL_NV_bgr GL_EXT_bgra

	//const char *wglExtString = wglGetExtensionsStringARB(wglGetCurrentDC());
	//Log("wgl extensions %s\n",wglExtString);

	CheckGLError("pre glGetInt", __FILE__, __LINE__);

	int max_uniform_vecs=0;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS,&max_uniform_vecs);
	Log("GL_MAX_VERTEX_UNIFORM_VECTORS %d\n",max_uniform_vecs);

	int max_tex_units=0;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&max_tex_units);
	Log("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS %d\n",max_tex_units);
	CheckGLError("glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)", __FILE__, __LINE__);
	
	int num_compr_formats=0;
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS,&num_compr_formats);
	Log("GL_NUM_COMPRESSED_TEXTURE_FORMATS %d\n",num_compr_formats);
	if(num_compr_formats){
		int compr_formats[num_compr_formats]={0};
		glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS,compr_formats);
		for(int i=0;i<num_compr_formats;i++){
			Log("compressed format %d: %X (%s)\n",i,compr_formats[i],GetComprFormatName(compr_formats[i]));
		}
	}

//debug
#if 1
	Log("extFlags %llX\n",extFlags);

	Log("%s %s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER));
	Log("%s\n",glGetString(GL_VERSION));
	Log("%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
}

#ifdef ANDROID
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays;
PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays;
PFNGLISVERTEXARRAYOESPROC glIsVertexArray;
PFNGLGETPROGRAMBINARYOESPROC glGetProgramBinaryOES;
PFNGLPROGRAMBINARYOESPROC glProgramBinaryOES;


#include <EGL/egl.h>
void GLExtensions::InitVAO()
{
	Log("InitVAO\n");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
	Log("glBindVertexArray %p\n",glBindVertexArray);
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
	glIsVertexArray = (PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress("glIsVertexArrayOES");
}

void GLExtensions::InitBinProg()
{
	glGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC) eglGetProcAddress("glGetProgramBinaryOES");
	glProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC) eglGetProcAddress("glProgramBinaryOES");
}
#else
void GLExtensions::InitVAO(){Log("InitVAO null\n");}
void GLExtensions::InitBinProg(){Log("InitBinProg null\n");}
#endif
