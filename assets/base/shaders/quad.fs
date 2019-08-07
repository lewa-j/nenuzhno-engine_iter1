precision highp float;

varying vec2 v_uv;
uniform sampler2D u_tex;

void main()
{
	gl_FragColor.rgb = vec3(0.5);
	gl_FragColor.a = 0.2;//texture2D(u_tex,v_uv);
}
