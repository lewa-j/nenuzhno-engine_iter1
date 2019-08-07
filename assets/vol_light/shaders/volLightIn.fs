#version 100
precision highp float;

varying vec4 v_position;
varying vec3 v_viewDir;
varying vec4 v_coord;

uniform sampler2D u_lightDepth;
uniform sampler2D u_sceneDepth;
uniform mat4 u_lightMtx;
uniform mat4 u_invVPMtx;
uniform vec4 u_lightPosSize;
uniform vec3 u_cameraPos;

vec3 GetPos(vec2 uv, float depth)
{
	vec4 pos = u_invVPMtx*(vec4(uv,depth,1.0)*2.0-1.0);
	pos.xyz /= pos.w;
	return pos.xyz;
}

vec3 GetLightUV(vec3 pos){
	vec4 v = u_lightMtx * vec4(pos,1.0);
	v.xyz /= v.w;
	v.xyz = v.xyz*0.5+0.5;
	return v.xyz;
}

float GetDist(vec3 pos){
	vec3 luv = GetLightUV(pos);
	float lightDepth = texture2D(u_lightDepth,luv.xy).r;
	return luv.z-lightDepth;
}

float GetShadow(vec3 pos){
	float shadow = GetDist(pos);
	float atten = 1.0-clamp(length(u_lightPosSize.xyz-pos)/u_lightPosSize.w, 0.0, 1.0);
	return float(shadow<0.001)*atten;
}

float GetPCF(vec3 pos){
	vec3 luv = GetLightUV(pos);
	float shadow = 25.0;
	for(int x=-2; x<3; x++){
		for(int y=-2; y<2; y++){
			float lightDepth = texture2D(u_lightDepth,luv.xy+vec2(x,y)*0.002).r;
			shadow -= float((luv.z-lightDepth)<0.003);
		}
	}
	return shadow/25.0;
}

#define STEPS 128.0

float rand(float n){
	return fract(sin(n) * 43758.5453123);
}

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(float p){
	float fl = floor(p);
	float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), fc);
}
	
float noise(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
	vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

void main(){
	vec3 scr = v_coord.xyz/v_coord.w;
	scr.xyz = scr.xyz*0.5+0.5;

	float sceneDepth = texture2D(u_sceneDepth,scr.xy).r;
	
	vec3 start = u_cameraPos;
	vec3 end = GetPos(scr.xy,sceneDepth);

	float shadow = 0.0;
	vec3 step = (end-start)/STEPS;
	vec3 curPos = start;
	for(int i=0;i<int(STEPS);i++){
		curPos += step;
		shadow += GetShadow(curPos);
	}
	shadow /= STEPS;

	//if(GetShadow(end)<0.1)
	//	shadow-=0.3;
	//shadow -= GetPCF(end)*0.4;

	vec4 outc = vec4(shadow*0.5);

	/*vec4 outc = vec4(shadow*0.5)*vec4(1.0,0.0,0.0,0.0)*(abs(noise(end.xy*3.0))*0.7+0.3);
	
	if(abs(GetDist(end))<0.002){
		outc = vec4(1.0,-1.0,-1.0,0.0)*10.0;
	}*/

	//gl_FragColor = vec4((sceneDepth-scr.z)*5.0);
	//gl_FragColor = vec4(pow(scr.z,5.0));
	//gl_FragColor = vec4(start,1.0);
	//gl_FragColor = vec4(end,1.0);
	//gl_FragColor = vec4(pow(sceneDepth,5.0));
	//gl_FragColor = vec4(luv.zzz,1.0);
	gl_FragColor = outc;
	//gl_FragColor = vec4(pow(lightDepth,5.0));
	//gl_FragColor = vec4(distance(start,end)*0.1);
}
