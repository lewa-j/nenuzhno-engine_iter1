
#pragma once

#include "graphics/platform_gl.h"
#define GL_SGX_BINARY_IMG 0x8C0A
#define GL_SGX_PROGRAM_BINARY_IMG 0x9130

#include <mat4x4.hpp>

class glslProg
{
public:
	glslProg();
	glslProg(const char *vert, const char *frag);
	~glslProg();

	GLint u_mvpMtx;
	GLint u_modelMtx;
	GLint u_invModelMtx;
	GLint u_cameraPos;
	GLint u_color;

	bool CreateFromFile(const char *vFileName, const char *fFileName);
	bool CreateFromFile(const char *vFileName, const char *fFileName, const char *flags);
	bool CreateFromBinary(int fmt, const void *data, int len);
	bool Use();
	GLint GetUniformLoc(const char *uname);
	GLint GetAttribLoc(const char *aname);
	bool GetBinaryData(int *fmt, char **data, int *len);
	void UniformVec4(int loc, const glm::vec4 &vec);
	void UniformMat4(int loc, const glm::mat4 &mtx);
	void UniformTex(const char *name, int unit);
	bool Save(const char *name);
private:
	GLuint id;
	GLuint CreateShader(const char *text, GLint type, const char *flags);
	bool CreateProgram(const char *vert, const char *frag, const char *flags);
	void print_log(GLuint object);
};
