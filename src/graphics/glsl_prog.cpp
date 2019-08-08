
#include <cstdlib>
#include <fstream>
#include <string>
using namespace std;

#include <gtc/type_ptr.hpp>

#include "log.h"
#include "system/FileSystem.h"
#include "graphics/platform_gl.h"
#include "graphics/glsl_prog.h"
#include "graphics/gl_utils.h"

glslProg::glslProg()
{
	id = 0;
	u_mvpMtx = -1;
	u_modelMtx = -1;
	u_invModelMtx = -1;
	u_cameraPos = -1;
	u_color = -1;
}

glslProg::glslProg(const char *vert, const char *frag)
{
	id = 0;
	u_mvpMtx = -1;
	u_modelMtx = -1;
	u_invModelMtx = -1;
	u_cameraPos = -1;
	u_color = -1;
	if(!CreateProgram(vert, frag,"")){
		Log( "Shader program ctor error!\n");
	}
}

glslProg::~glslProg(){
	if(id)
		glDeleteProgram(id);
	id = 0;
}

bool glslProg::CreateFromFile(const char *vFileName, const char *fFileName)
{
	string path = "shaders/"+string(vFileName)+".vs";
	char *vs = g_fs.ReadAll(path.c_str());
	if(!vs){
		Log("Vertex shader file %s missing\n", vFileName);
		return false;
	}

	path = "shaders/"+string(fFileName)+".fs";
	char *fs = g_fs.ReadAll(path.c_str());
	if(!fs){
		Log("Fragment shader file %s missing\n", fFileName);
		delete[] vs;
		return false;
	}

	bool result = CreateProgram(vs, fs,"");

	delete[] vs;
	delete[] fs;

	if(!result)
		Log("Error in program %s/%s\n",vFileName,fFileName);
	return result;
}

bool glslProg::CreateFromFile(const char *vFileName, const char *fFileName, const char *flags)
{
	string path = "shaders/"+string(vFileName)+".vs";
	char *vs = g_fs.ReadAll(path.c_str());
	if(!vs){
		Log("Vertex shader file %s missing\n", vFileName);
		return false;
	}

	path = "shaders/"+string(fFileName)+".fs";
	char *fs = g_fs.ReadAll(path.c_str());
	if(!fs){
		Log("Fragment shader file %s missing\n", fFileName);
		delete[] vs;
		return false;
	}

	bool result = CreateProgram(vs, fs, flags);

	delete[] vs;
	delete[] fs;

	if(!result)
		Log("Error in program %s/%s (%s)\n",vFileName,fFileName,flags);
	return result;
}

void glslProg::print_log(GLuint object)
{
	GLint log_length=0;
	if(glIsShader(object)){
		glGetShaderiv(object,GL_INFO_LOG_LENGTH,&log_length);
		Log("Shader log_length %d\n",log_length);
	}else if (glIsProgram(object)){
		glGetProgramiv(object,GL_INFO_LOG_LENGTH,&log_length);
		Log("Program log_length %d\n",log_length);
	}else{
		Log("print_log: Not a shader or a program\n");
		return;
	}
	if(!log_length)
		return;
	char* log = new char[log_length];

	if(glIsShader(object))
		glGetShaderInfoLog(object,log_length,NULL,log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object,log_length,NULL,log);

	Log("%s\n",log);
	delete[] log;
}

GLuint glslProg::CreateShader(const char *src, GLint type, const char *flags)
{
	GLuint sid = glCreateShader(type);
	char ver[] = "#version 100\n";
	if(strstr(src,"#version"))
		ver[0] = 0;
	const char *strings[] = {ver,flags,src};
	glShaderSource(sid,3,strings,NULL);
	glCompileShader(sid);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(sid,GL_COMPILE_STATUS, &compile_ok);
	if(!compile_ok){
		if(type==GL_VERTEX_SHADER)
			Log("vert: ");
		else
			Log("frag: ");
		print_log(sid);
		glDeleteShader(sid);
		return 0;
	}
	return sid;
}

bool glslProg::CreateProgram(const char *vert, const char *frag, const char *flags)
{
	GLuint vs = CreateShader(vert, GL_VERTEX_SHADER, flags);
	//CheckGLError("CreateShader", __FILE__, __LINE__);
	if(!vs)
		return false;
	GLuint fs = CreateShader(frag, GL_FRAGMENT_SHADER, flags);
	if(!fs){
		glDeleteShader(vs);
		return false;
	}
	if(id)
		glDeleteProgram(id);
	id = glCreateProgram();
	//CheckGLError("glCreateProgram", __FILE__, __LINE__);

	glBindAttribLocation(id,0,"a_position");
	glBindAttribLocation(id,1,"a_normal");
	glBindAttribLocation(id,2,"a_uv");
	glBindAttribLocation(id,3,"a_uv2");//TODO check old projects
	glBindAttribLocation(id,4,"a_tangent");//TODO check old projects

	//CheckGLError("glBindAttribLocation", __FILE__, __LINE__);

	glAttachShader(id,vs);
	glAttachShader(id,fs);

	glLinkProgram(id);
	//CheckGLError("glLinkProgram", __FILE__, __LINE__);

	glDeleteShader(vs);
	glDeleteShader(fs);

	GLint link_ok=GL_FALSE;
	glGetProgramiv(id, GL_LINK_STATUS, &link_ok);
	if(!link_ok){
		Log("prog %d GL_LINK_STATUS %d\n",id,link_ok);
		print_log(id);
		glDeleteProgram(id);
		id = 0;
		return false;
	}

#if 0
	int uniformsCount = 0;
	glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformsCount);
	Log("Shader: active uniforms count %d\n", uniformsCount);
	for(int i=0; i<uniformsCount; i++)
	{
		char nameBuff[256] = {0};
		GLint size = 0;
		GLenum type = 0;
		glGetActiveUniform (id, i, sizeof(nameBuff), 0, &size, &type, nameBuff);
		Log("uniform %d: %s, size %d, type %X\n",i, nameBuff, size, type);
	}
	
	int attribsCount = 0;
	glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &attribsCount);
	Log("Shader: active attrib count %d\n", attribsCount);
	for(int i=0; i<attribsCount; i++)
	{
		char nameBuff[256] = {0};
		GLint size = 0;
		GLenum type = 0;
		glGetActiveAttrib (id, i, sizeof(nameBuff), 0, &size, &type, nameBuff);
		Log("attrib %d: %s, size %d, type %X\n",i, nameBuff, size, type);
	}
#endif
	//Log("Created Program %d\n",id);
	return true;
}

bool glslProg::Use()
{
	if(!id)
		return false;
	glUseProgram(id);
	return true;
}

GLint glslProg::GetUniformLoc(const char *uname)
{
	GLint val = glGetUniformLocation(id, uname);
	if(val < 0)
		Log("prog %d uniform %s is %d\n", id, uname, val);
	return val;
}

GLint glslProg::GetAttribLoc(const char *aname)
{
	GLint val = glGetAttribLocation(id, aname);
	Log("attribute %s is %d\n", aname, val);
	return val;
}

void glslProg::UniformVec4(int loc, const glm::vec4 &vec){
	glUniform4fv(loc,1,glm::value_ptr(vec));
}

void glslProg::UniformMat4(int loc, const glm::mat4 &mtx){
	glUniformMatrix4fv(loc,1,false,glm::value_ptr(mtx));
}

void glslProg::UniformTex(const char *name, int unit)
{
	int u_tex = GetUniformLoc(name);
	Use();
	glUniform1i(u_tex, unit);
}
