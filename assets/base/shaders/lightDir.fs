#version 100
precision highp float;

varying vec3 v_normal;
varying vec2 v_uv;
varying vec3 v_viewDir;

uniform sampler2D u_tex;
uniform vec3 u_lightDir;

const float specPower = 40.0;

vec3 calcLight(vec3 dir, vec3 norm, vec3 lcol, vec3 col){
	vec3 reflectVec = reflect(-dir, norm);
	float diffuse = max(0.0,dot(norm,dir));
	vec3 vd = normalize(v_viewDir);
	float specular = 0.5*pow(max(dot(vd, reflectVec), 0.0), specPower);

	return col*diffuse*lcol + specular*lcol;
}

void main()
{
	vec4 col = texture2D(u_tex, v_uv);

	vec3 normal = normalize(v_normal);
	gl_FragColor = vec4(0.0);
	//float diffuse = max(0.0,dot(normal,u_lightDir));
	gl_FragColor.rgb = calcLight(u_lightDir,normal,vec3(1.0),col.rgb);
	//gl_FragColor = col;
	gl_FragColor.rgb+=0.1;
}
