#version 100
precision highp float;

varying vec2 v_uv;
uniform float u_textureWeight;
uniform sampler2D texture;
uniform sampler2D u_worldTex;
uniform float u_timeSinceStart;
uniform int u_numCubes;
const float glossiness = 0.8;

float intersectSphere(vec3 origin, vec3 ray, vec4 sphereCenter)
{
	vec3 toSphere = origin - sphereCenter.xyz;
	float a = dot(ray, ray);
	float b = 2.0 * dot(toSphere, ray);
	float c = dot(toSphere, toSphere) - sphereCenter.w*sphereCenter.w;
	float discriminant = b*b - 4.0*a*c;
	if(discriminant > 0.0){
		float t = (-b - sqrt(discriminant)) / (2.0 * a);
		if(t > 0.0) return t;
	}
	return 9999.9;
}
vec2 intersectCube(vec3 origin, vec3 ray, vec3 cubeMin, vec3 cubeMax){
   vec3 tMin = (cubeMin - origin) / ray;
   vec3 tMax = (cubeMax - origin) / ray;
   vec3 t1 = min(tMin, tMax);
   vec3 t2 = max(tMin, tMax);
   float tNear = max(max(t1.x, t1.y), t1.z);
   float tFar = min(min(t2.x, t2.y), t2.z);
   return vec2(tNear, tFar);
}
vec3 normalForCube(vec3 hit, vec3 cubeMin, vec3 cubeMax){
   if(hit.x < cubeMin.x + 0.0001) return vec3(-1.0, 0.0, 0.0);
   else if(hit.x > cubeMax.x - 0.0001) return vec3(1.0, 0.0, 0.0);
   else if(hit.y < cubeMin.y + 0.0001) return vec3(0.0, -1.0, 0.0);
   else if(hit.y > cubeMax.y - 0.0001) return vec3(0.0, 1.0, 0.0);
   else if(hit.z < cubeMin.z + 0.0001) return vec3(0.0, 0.0, -1.0);
   else return vec3(0.0, 0.0, 1.0);
}
float random(vec3 scale, float seed){
   return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) * 43758.5453 + seed);
}
vec3 cosineWeightedDirection(float seed, vec3 normal){
   float u = random(vec3(12.9898, 78.233, 151.7182), seed);
   float v = random(vec3(63.7264, 10.873, 623.6736), seed);
   float r = sqrt(u);
   float angle = 6.283185307179586 * v;
   vec3 sdir, tdir;
   if (abs(normal.x)<.5){
     sdir = cross(normal, vec3(1,0,0));
   }else{
     sdir = cross(normal, vec3(0,1,0));
   }
   tdir = cross(normal, sdir);
   return r*cos(angle)*sdir + r*sin(angle)*tdir + sqrt(1.-u)*normal;
}
vec3 uniformlyRandomDirection(float seed){
	float u = random(vec3(12.9898, 78.233, 151.7182), seed);
	float v = random(vec3(63.7264, 10.873, 623.6736), seed);
	float z = 1.0 - 2.0 * u;
	float r = sqrt(1.0 - z * z);
	float angle = 6.283185307179586 * v;
	return vec3(r * cos(angle), r * sin(angle), z);
}
vec3 uniformlyRandomVector(float seed){
	return uniformlyRandomDirection(seed) * sqrt(random(vec3(36.7539, 50.3658, 306.2759), seed));
}
float shadow(vec3 origin, vec3 ray) {
	for(int c = 0; c<u_numCubes;c++){
		vec3 cubemin = texture2D(u_worldTex,vec2(0.0,float(c)/float(u_numCubes-1))).xyz;
		vec3 cubemax = texture2D(u_worldTex,vec2(1.0,float(c)/float(u_numCubes-1))).xyz;
		vec2 tCube1 = intersectCube(origin, ray, cubemin,cubemax);
		if(tCube1.x > 0.0 && tCube1.x < 1.0 && tCube1.x < tCube1.y)
			return 0.0;
	}
	return 1.0;
}
const vec3 lightPos = vec3(-1.3,0.1,0.05);
vec3 calculateColor(vec3 origin, vec3 ray, vec3 light,float seed){
	vec3 colorMask = vec3(1.0);
	vec3 result = vec3(0.0);
	for(int bounce = 0; bounce < 4; bounce++){
		float t = 9999.9;
		vec3 normal;
		vec2 tRoom = intersectCube(origin, ray, vec3(-1.5,-1.0,-1.0), vec3(1.0));
		if(tRoom.x < tRoom.y) t = tRoom.y;
		vec3 hit = origin + ray * t;
		for(int c = 0; c < u_numCubes;c++){
			vec3 cubemin = texture2D(u_worldTex,vec2(0.0,float(c)/float(u_numCubes-1))).xyz;
			vec3 cubemax = texture2D(u_worldTex,vec2(1.0,float(c)/float(u_numCubes-1))).xyz;
			vec2 tCube1 = intersectCube(origin, ray, cubemin,cubemax);
			if(tCube1.x > 0.0 && tCube1.x < t && tCube1.x < tCube1.y)
			{
				t = tCube1.x;
				hit = origin + ray * t;
				normal = normalForCube(hit, cubemin,cubemax);
			}
		}
		float lth = intersectSphere(origin,ray,vec4(lightPos,0.1));
		if(lth<t)
			return vec3(1.0);
		vec3 surfaceColor = vec3(0.75);
		if(t == tRoom.y){
			normal = -normalForCube(hit, vec3(-1.5,-1.0,-1.0), vec3(1.0));
			if(hit.x < -1.4999) surfaceColor = vec3(1.0, 0.3, 0.1);
			else if(hit.x > 0.9999) surfaceColor = vec3(0.3, 1.0, 0.1);
			ray = cosineWeightedDirection(seed + float(bounce), normal);
			//ray = reflect(ray, normal);
			//ray = normalize(reflect(ray, normal)) + uniformlyRandomVector(seed + float(bounce)) * glossiness;
		}else if(t == 9999.9){
			break;
		}else{
			ray = normalize(reflect(ray, normal)) + uniformlyRandomVector(seed + float(bounce)) * glossiness;
			surfaceColor = vec3(0.5, 0.5, 0.9);
		}
		vec3 toLight = light - hit;
		float diffuse = max(0.0, dot(normalize(toLight), normal));
		float shadowIntensity = shadow(hit + normal * 0.0001, toLight);
		colorMask *= surfaceColor;
		//if(bounce>0)
			result += colorMask*diffuse*0.5*shadowIntensity;
		origin = hit;
	}
	return result;
}
void main(){
	vec3 dir = normalize(vec3(v_uv,-1.0));
	vec3 newLight = lightPos + uniformlyRandomVector(u_timeSinceStart - 53.0) * 0.1;
	vec3 sample1 = calculateColor(vec3(-0.2,0.15,1.5), dir, newLight,u_timeSinceStart);
	//newLight = vec3(-1.4,0.1,-0.1) + uniformlyRandomVector(u_timeSinceStart - 27.4) * 0.1;
	//sample1 = 0.5*(sample1+calculateColor(vec3(-0.2,0.15,1.5), dir, newLight,u_timeSinceStart+65.3));
	vec3 textureCol = texture2D(texture, v_uv*0.5+0.5).rgb;
	gl_FragColor = vec4(mix(sample1, textureCol, u_textureWeight), 1.0);
	
	//gl_FragColor = vec4(sample1,1.0);
}
