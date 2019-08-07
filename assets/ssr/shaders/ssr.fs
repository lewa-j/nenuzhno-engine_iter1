#version 100
precision highp float;

varying vec4 v_scrpos;
varying vec4 v_position;
varying vec3 v_normal;
uniform sampler2D u_tex;
uniform sampler2D u_depthTex;
uniform mat4 u_mvpMtx;
uniform mat4 u_invMVPMtx;

uniform vec3 u_eyePos;// = vec3(0.0, 0.6, 1.0);

vec3 GetPos(vec2 uv, float depth)
{
	vec4 pos = u_invMVPMtx*(vec4(uv,depth,1.0)*2.0-1.0);
	pos.xyz /= pos.w;
	return pos.xyz;
}

vec3 GetUV(vec3 apos)
{
	vec4 pVP = u_mvpMtx*vec4(apos,1.0);
	pVP.xy = vec2(0.5,0.5)+vec2(0.5,0.5)*pVP.xy / pVP.w;
	return vec3(pVP.xy,pVP.z/pVP.w);
}

vec4 u_camera = vec4(0.1,2.0,0.5,1.0);//near far fov aspect
float GetDepth(vec2 uv)
{
	float depth = texture2D(u_depthTex,uv).r;
	depth = 2.0 * u_camera.x * u_camera.y / 
		(u_camera.y + u_camera.x - 
		(depth*2.0-1.0)*(u_camera.y-u_camera.x));
	return depth;
}

const float rayStepSize = 0.1;
vec2 v_uv;
vec3 raytrace(in vec3 reflectionVector, in float startDepth)
{
	vec3 color = vec3(0.0);
	float stepSize = rayStepSize;

	float size = length(reflectionVector.xy);
	reflectionVector = normalize(reflectionVector/size);
	reflectionVector = reflectionVector * stepSize;

	//Current sampling position is at current fragment
	vec2 sampledPosition = v_uv;
	//Current depth at current fragment
	float currentDepth = startDepth;
	//The sampled depth at the current sampling position
	//float sampledDepth = linearizeDepth(texture2D(u_depthTexture, sampledPosition).x);
	float sampledDepth = GetDepth(sampledPosition);

	// Raytrace as long as in texture space of depth buffer (between 0 and 1)
	while(sampledPosition.x <= 1.0 && sampledPosition.x >= 0.0 && sampledPosition.y <= 1.0 && sampledPosition.y >= 0.0)
	{
		//Update sampling position by adding reflection vector's xy and y components
		sampledPosition = sampledPosition + reflectionVector.xy;
		//Updating depth values
		currentDepth = currentDepth + reflectionVector.z * startDepth;
		//float sampledDepth = linearizeDepth( texture2D(u_depthTexture, sampledPosition).x);
		float sampledDepth = GetDepth(sampledPosition);

		//If current depth is greater than sampled depth of depth buffer, intersection is found
		if(currentDepth > sampledDepth)
		{
			//Delta is for stop the raytracing after the first intersection is found
			//Not using delta will create "repeating artifacts"
			float delta = (currentDepth - sampledDepth);
			if(delta < 0.003)
			{
				color = texture2D(u_tex, v_uv).rgb;
				break;
			}
		}
	}

	return color;
}

#if 0
vec4 ssr()
{
	vec3 reflectedColor = vec3(0.0);

	//vec3 normal = normalize( texture2D(u_normalTexture, v_uv).xyz*2.0-1.0 );
	vec3 normal = v_normal;
	
	//Depth at current fragment
	//float currDepth = linearizeDepth( texture2D(u_depthTex, v_uv).x );
	float currDepth = GetDepth(v_uv);

	vec3 pos = GetPos(v_uv,currDepth);
	//Eye position, camera is at (0, 0, 0), we look along negative z, add near plane to correct parallax
	vec3 eyePosition = normalize(pos-u_eyePos);// vec3(0.0, 0.0, 0.01) );
	vec3 reflectionVector = reflect(eyePosition, normal);
	reflectionVector= GetUV(reflectionVector);
	//Call raytrace to get reflected color
	reflectedColor = raytrace(reflectionVector, currDepth);

	float nl = max(-dot(normal, eyePosition),0.0);
	float fresnel = pow(1.0-nl,0.82);

	return vec4(reflectedColor, 1.0);
}

#else
vec4 ssr()
{
	//todo: normal buffer
	vec3 normal = v_normal;
	
	//vec3 pos = GetPos(uv, depth);
	vec3 pos = v_position.xyz;
	vec3 viewDir = normalize(pos-u_eyePos);
	vec3 reflectDir = normalize(reflect(viewDir, normal));
	
	vec3 currentRay = vec3(0.0);
	vec3 nuv = vec3(0.0);
	float L = 0.06;//???
	
	for(int i = 0; i<10; i++){
		currentRay = pos+reflectDir*L;
		
		nuv = GetUV(currentRay);
		float nd = texture2D(u_depthTex, nuv.xy).x;
		vec3 newPos = GetPos(nuv.xy, nd);//2???
		L = length(pos-newPos);
	}
	if(L>0.4)
		return vec4(0.0);
	vec4 cnuv = vec4(0.0);
	cnuv = texture2D(u_tex, nuv.xy);
	/*for(float x = -0.1; x<=0.1; x+=0.025){
		cnuv += texture2D(u_tex, nuv.xy+vec2(x*L,0.0))*0.1;
	}*/
	
	float nl = max(-dot(normal, viewDir),0.0);
	float fresnel = pow(1.0-nl,0.82);
	
	//return vec4(normalize(currentRay),1.0);
	//return vec4(vec3(pow(nuv.z,0.4)),1.0);
	//return vec4(L*4.0);
	return cnuv*fresnel*(1.0-L*2.0);
//	return vec4(L*4.0);
}
#endif

void main()
{
	vec2 uv = v_scrpos.xy/v_scrpos.w*0.5+0.5;
	v_uv = uv;
	vec4 col = texture2D(u_tex, uv);
	//float depth = texture2D(u_depthTex, uv).x;
	float depth = gl_FragCoord.z;
	vec4 reflect = vec4(0.0);
	if(depth<(texture2D(u_depthTex, uv).x+0.002))
	{
		reflect = ssr();
	}
	
	gl_FragColor = col+reflect;
	//gl_FragColor = reflect;
	
	//gl_FragColor = col*pow(depth, 10.0);
	//gl_FragColor = vec4(reflectDir, 1.0);
}
