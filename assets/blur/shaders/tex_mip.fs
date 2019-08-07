#version 100
precision highp float;

varying vec2 v_uv;
uniform sampler2D u_texture;

void main()
{
	float a=0.0;
	//a=length(v_uv-0.5)*5.0;//+4.0;
	a=v_uv.x*2.0+3.5;
	vec4 col = texture2D(u_texture, v_uv,a);
	gl_FragColor = col;
}
