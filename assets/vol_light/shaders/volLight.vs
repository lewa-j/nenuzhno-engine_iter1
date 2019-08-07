#version 100
precision highp float;

attribute vec4 a_position;

varying vec4 v_position;
varying vec3 v_viewDir;
varying vec4 v_coord;
varying vec4 v_cameraPos;

uniform mat4 u_mvpMtx;
uniform mat4 u_modelMtx;
uniform vec3 u_cameraPos;

void main(){
	gl_Position = u_mvpMtx * a_position;
	v_coord = gl_Position;
	v_position = (u_modelMtx*a_position);
	v_viewDir = u_cameraPos - v_position.xyz;
}
