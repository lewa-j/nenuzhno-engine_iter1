#version 100
precision highp float;

attribute vec4 a_position;

uniform mat4 u_mvpMtx;

void main()
{
	gl_Position = u_mvpMtx * a_position;
	gl_PointSize = 4.0;
}
