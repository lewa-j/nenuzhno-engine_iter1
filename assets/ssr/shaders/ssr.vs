#version 100
precision highp float;
  
attribute vec4 a_position;
attribute vec3 a_normal;

uniform mat4 u_mvpMtx;

varying vec4 v_scrpos;
varying vec4 v_position;
varying vec3 v_normal;

void main()
{
	v_scrpos = u_mvpMtx * a_position;
	v_position = a_position;
	v_normal = a_normal;
	gl_Position = u_mvpMtx * a_position;
}