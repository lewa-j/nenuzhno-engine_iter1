#version 100
precision highp float;

varying vec4 v_col;
uniform sampler2D u_coreTex;

void main()
{
	//vec4 col = texture2D(u_coreTex, gl_PointCoord.xy);
	vec4 col = vec4(length(gl_PointCoord.xy-0.5)<0.5);
	gl_FragColor = col*v_col;
}
