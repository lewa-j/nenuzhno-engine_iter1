#version 100
precision highp float;

varying vec3 v_normal;
varying vec2 v_uv;
varying vec3 v_bones;

uniform sampler2D u_texture;
uniform vec4 u_color;

void main()
{
	vec4 col = texture2D(u_texture, v_uv);
	col *= max(0.2,dot(v_normal,normalize(vec3(1.0))));
	gl_FragColor = col * u_color;
	//gl_FragColor = mix(gl_FragColor,v_bones.xxxx*0.1,0.99);
}
