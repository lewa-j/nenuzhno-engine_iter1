#version 100
precision highp float;

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv;

varying vec3 v_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec3 v_viewDir;

uniform mat4 u_mvpMtx;
uniform mat4 u_modelMtx;
uniform vec3 u_camPos;

void main()
{
	gl_Position = u_mvpMtx * a_position;
	gl_PointSize = 4.0;
	v_normal = normalize(mat3(u_modelMtx)*a_normal);
	v_position = (u_modelMtx*a_position).xyz;
	v_uv = a_uv;
	v_viewDir = u_camPos-(u_modelMtx*a_position).xyz;
}
