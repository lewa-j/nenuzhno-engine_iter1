#version 100
precision highp float;

varying vec3 v_position;
varying vec3 v_normal;
varying vec2 v_uv;

uniform sampler2D u_texture;
uniform vec3 u_lightColor;
uniform vec3 u_lightPos;
uniform float u_lightSize;

varying vec3 v_viewDir;
const float specPower = 10.0;

vec3 calcLight(vec3 dir, vec3 norm, vec3 lcol, vec3 col){
	vec3 reflectVec = reflect(-dir, norm);
	float diffuse = max(0.0,dot(norm,dir));
	vec3 vd = normalize(v_viewDir);
	float specular = 0.3*pow(max(dot(vd, reflectVec), 0.0), specPower);

	return col*diffuse*lcol + specular*lcol;
}

vec3 calcPointLight(vec3 pos, vec3 norm, vec3 lcol, vec3 col){
	//float atten = 1.0 - pow(clamp(length(pos-v_position)/u_lightSize, 0.0, 1.0), 1.5);
	float atten = 1.0-clamp(length(pos-v_position)/u_lightSize, 0.0, 1.0);
	return calcLight(normalize(pos-v_position),norm,lcol,col)*atten;
}

void main()
{
	//if(length(u_lightPos-v_position)>u_lightSize)
	//	discard;
	vec4 col = texture2D(u_texture, v_uv);
	vec3 norm = normalize(v_normal);

	vec4 outc = vec4(0.0,0.0,0.0,1.0);
	outc.rgb += calcPointLight(u_lightPos,norm,u_lightColor,col.rgb);

	gl_FragColor = outc;
}

