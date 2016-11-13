#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vVelocity;

uniform mat4 model;
uniform mat4 projection;
uniform int triangleCount;
uniform samplerBuffer geometryTbo;
uniform float timeStep = 0.02;

out float position;
out vec3 velocity;

void main(){
	position = 25.0f;
}

//bool intersect(vec3 origin, vec3 direction, vec3 v0, vec3 v1, vec3 v2, out vec3 point) {
//	vec3 u,v,normal;
//	vec3 w0, w;
//	float r, a, b;

//	u = v1 - v0;
//	v = v2 - v0;
//	normal = cross(u,v);

//	w0 = origin - v0;
//	a = -dot(normal, w0);
//	b = dot(normal, direction);

//	r = a/b;

//	if(r < 0.0 || r > 1.0 ) return false;

//	point = origin + r * direction;

//	float uu, uv, vv, wu, wv, D;

//	uu = dot (u,u);
//	uv = dot (u,v);
//	vv = dot (v,v);

//	w = point - v0;
//	wu = dot(w,u);
//	wv = dot(w,v);

//	D = uv * uv - uu * vv;

//	float s,t;
//	s = (uv * wv - vv * wu) / D;

//	if(s < 0.0 || s > 1.0) return false;

//	t = (uv * wu - uu * wv) / D;

//	if(t < 0.0 || (s + t) > 1.0) return false;

//	return true;
//}

//vec3 reflectVector(vec3 v, vec3 normal) {
//	return v - 2.0 * dot(v,normal) * normal;
//}

//void main(){
//	vec3 acceleration = vec3(0.0, -0.3, 0.0);
//	vec3 newVelocity = vVelocity + acceleration * timeStep;
//	vec4 newPosition = vPosition + vec4(newVelocity * timeStep, 0.0);

//	vec3 v0,v1,v2;
//	vec3 point;
//	int i;

//	for(i = 0; i < triangleCount; i++) { 
//		v0 = texelFetch(geometryTbo, i * 3).xyz;
//		v1 = texelFetch(geometryTbo, i * 3 + 1).xyz;
//		v2 = texelFetch(geometryTbo, i * 3 + 2).xyz;

//		if(intersect(vPosition.xyz, vPosition.xyz - newPosition.xyz, v0, v1, v2, point)) {
//			vec3 n = normalize(cross(v1 - v0, v2 - v0));
//			newPosition = vec4(point + reflectVector(newPosition.xyz - point, n), 1.0);
//			newVelocity = 0.8 * reflectVector(newVelocity,n);
//		}
//	}

//	if(newPosition.y < -40.0) {
//		newPosition = vec4(-newPosition.x * 0.3, vPosition.y + 80.0, 0.0,1.0);
//		newVelocity *= vec3(0.2,0.1,-0.3);
//	}

//	velocity = newVelocity * 0.9999;
//	position = newPosition;
//	//position = vec4(1,2,3,1);
//	//gl_Position = projection * model * vPosition;
//}
