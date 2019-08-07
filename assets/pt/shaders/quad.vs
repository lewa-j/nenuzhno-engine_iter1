#version 100
precision highp float;
attribute vec4 a_position;
varying vec2 v_uv;
void main(){
	gl_Position = a_position;
	v_uv = a_position.xy*0.5+0.5;
}
