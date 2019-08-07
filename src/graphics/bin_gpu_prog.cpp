
#include <fstream>
#include <string>
using namespace std;
#include "log.h"
#include "system/FileSystem.h"
#include "graphics/platform_gl.h"
#include "graphics/glsl_prog.h"
#include "graphics/gl_ext.h"
#include "graphics/gl_utils.h"

#ifdef ANDROID
#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2ext.h>
#define GL_PROGRAM_BINARY_LENGTH GL_PROGRAM_BINARY_LENGTH_OES
#define GL_NUM_PROGRAM_BINARY_FORMATS GL_NUM_PROGRAM_BINARY_FORMATS_OES
#define GL_PROGRAM_BINARY_FORMATS GL_PROGRAM_BINARY_FORMATS_OES
#else
#define glProgramBinaryOES glProgramBinary
#define glGetProgramBinaryOES glGetProgramBinary

#endif

bool glslProg::CreateFromBinary(int fmt, const void *data, int len){
	if(!(GLExtensions::extFlags&eProgBin)){
		Log("Error: Binary shader program not suported\n");
		return false;
	}
	
	int numFormats=0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS,&numFormats);
	Log("numFormats %d\n",numFormats);
	int formats[numFormats]={0};
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS,formats);
	Log("format 0: %X\n",formats[0]);
	
	if(fmt!=formats[0]){
		Log("Error: unsuported shader program format %d\n",fmt);
		return false;
	}

	if(id)
		glDeleteProgram(id);
	id = glCreateProgram();
	CheckGLError("pre glProgramBinaryOES",__FILE__,__LINE__);
	glProgramBinaryOES(id, fmt, data, len);
	
	if(CheckGLError("BinaryShaderProgram",__FILE__, __LINE__)){
		Log("bin prog %d error\n",id);
		print_log(id);
		return false;
	}

	GLint link_ok=GL_FALSE;
	glGetProgramiv(id,GL_LINK_STATUS,&link_ok);
	if(!link_ok){
		Log("bin prog %d GL_LINK_STATUS %d\n",id,link_ok);
		print_log(id);
		glDeleteProgram(id);
		id = 0;
		return false;
	}
	return true;
}

bool glslProg::Save(const char *name){
	char *progData = 0;
	int progLen = 0;
	int progFmt = 0;
	if(!GetBinaryData(&progFmt,&progData,&progLen)){
		Log("Can't get shader prog data (%s)\n",name);
		return false;
	}
	
	Log("ShaderProgram Save(%s): fmt %X, len %d\n",name,progFmt,progLen);
	
	char path[256];
	
	g_fs.GetFilePath((string("shaders/bin/")+name+".bgp").c_str(),path,true);
	ofstream out(path,ios::binary);
	if(!out){
		Log("Can't create shader prog file (%s)!\n",name);
		delete[] progData;
		return false;
	}
	
	int ident = (('1'<<24)+('P'<<16)+('G'<<8)+'B');
	out.write((char*)&ident,4);
	out.write((char*)&progFmt,4);
	out.write((char*)&progLen,4);
	out.write(progData,progLen);
	out.close();
	delete[] progData;
	return true;
}

bool glslProg::GetBinaryData(int *fmt, char **data, int *len){
	if(!(GLExtensions::extFlags&eProgBin)){
		Log("Error: Binary shader program not suported\n");
		return false;
	}
	
	glGetProgramiv(id,GL_PROGRAM_BINARY_LENGTH,len);
	Log("Program binary length %d\n",*len);
	
	if(len<=0){
		Log("Error: shader program(%d) binary length %d\n",id,*len);
		return false;
	}
	
	(*data) = new char[*len];
	//memset(data,0,*len);
	
	glGetProgramBinaryOES(id, *len, len, (GLenum*)fmt, *data);
	Log("ProgramBinary format %X\n",*fmt);
	if(CheckGLError("GetBinaryShaderProgram",__FILE__, __LINE__)){
		delete[] (*data);
		return false;
	}
	return true;
}

bool ShaderFromBinary(int type,const void *data, int len){
	//void glShaderBinary (GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
	
	int numFormats = 0;
	glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS,&numFormats);
	Log("GL_NUM_SHADER_BINARY_FORMATS %d\n",numFormats);
	if(!numFormats){
		return false;
	}
	int formats[numFormats]={0};
	glGetIntegerv(GL_SHADER_BINARY_FORMATS,formats);
	Log("format 0: %X\n",formats[0]);

	if(formats[0]==0x8C0A)
		Log("format GL_SGX_BINARY_IMG\n");          

	int fmt = formats[0];
	GLuint sid = glCreateShader(type);
	glShaderBinary(1,&sid,fmt,data,len);
	
	CheckGLError("BinaryShader",__FILE__, __LINE__);
	
	return false;
}
