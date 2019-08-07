#version 100
precision highp float;

varying vec2 v_uv;
uniform sampler2D u_texture;
uniform vec4 u_color;

void main()
{
	vec4 col = texture2D(u_texture, v_uv);
	gl_FragColor = col * u_color;
}
