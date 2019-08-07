#version 100
precision highp float;
  
attribute vec4 a_position;

uniform mat4 u_mvpMtx;
uniform sampler2D u_texture;
uniform float u_size;

varying vec4 v_col;

//const float bright = 0.1;
//const float size = 21.5;

const float ndofstart = 1.0;
const float ndofdist = 2.0;
const float fdofstart = 1.0;
const float fdofdist = 3.0;

void main()
{
	float depth = abs(a_position.x*3.0);
	float fDepth = 1.5;
	float a = depth-fDepth; //focal plane
	float b = (a-fdofstart)/fdofdist; //far DoF
	float c = (-a-ndofstart)/ndofdist; //near Dof
	float size = (a>0.0)?b:c;
	size = clamp(size,0.0,1.0) *20.0+1.0;
	//float size = length(a_position.xy-0.5)*20.0+1.0;
	//float size = abs(a_position.x-u_size*0.01-0.4)*64.5+5.0;
	//float size=7.0;
	//size = u_size;//*0.5+3.0;
	
	//4/PI
	float bright = 1.273239/(size*size);

	bright = clamp(bright,0.007,1.0);
	gl_Position = vec4(a_position.xyz*2.0-1.0,1.0);
	v_col = texture2D(u_texture,a_position.xy)*bright;
	if(length(v_col)<0.01)
		gl_Position.z=-999.9;
	gl_PointSize = size;
	//gl_PointSize = a_position.x*34.5+5.0;
}
