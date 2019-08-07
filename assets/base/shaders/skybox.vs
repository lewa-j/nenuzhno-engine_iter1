#version 100
attribute vec4 a_position;
varying vec3 v_dir;
uniform mat4 u_mvpMtx;

void main()
{
	gl_Position = u_mvpMtx * a_position;
	v_dir = a_position.xyz;
}
