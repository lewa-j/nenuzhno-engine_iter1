#version 100
precision highp float;

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv;
attribute vec3 a_weight;
attribute vec3 a_bones;

varying vec3 v_normal;
varying vec2 v_uv;
//varying vec3 v_viewDir;
varying vec3 v_bones;

uniform mat4 u_mvpMtx;
uniform mat4 u_modelMtx;
uniform mat4 u_bonesMtx[77];
//uniform vec3 u_camPos;

void main(){
	/*int bi = int(a_bones.x);
	if(bi>76)bi=0;
	mat4 boneMtx = u_bonesMtx[bi];//*a_weight.x;*/
	mat4 boneMtx = u_bonesMtx[int(a_bones.x)]*a_weight.x+u_bonesMtx[int(a_bones.y)]*a_weight.y+u_bonesMtx[int(a_bones.z)]*a_weight.z;
	gl_Position = u_mvpMtx * boneMtx * a_position;
	gl_PointSize = 4.0;
	v_normal = normalize(mat3(u_modelMtx * boneMtx)*a_normal);
	v_uv = a_uv;
	//v_viewDir = u_camPos-(u_modelMtx*a_position).xyz;
	v_bones=a_bones;
}

