#version 100
precision highp float;

attribute vec4 a_position;
attribute vec2 a_uv;

varying vec2 v_uv;

uniform mat4 u_mvpMtx;

void main()
{
	gl_Position = u_mvpMtx * a_position;
	v_uv = a_uv;
	gl_PointSize = 4.0;
}
