#version 100
precision highp float;

varying vec3 v_dir;
uniform samplerCube u_tex;

void main()
{
	vec4 col = textureCube(u_tex, v_dir);
	gl_FragColor = col;
}
