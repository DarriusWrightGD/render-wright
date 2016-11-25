#version 440 core

out vec4 fColor;

struct Ray
{
	vec3 origin;
	vec3 direction;
};

struct Sphere
{
	vec3 center;
	float radius;
};

uniform float width = 800.0f;
uniform float height = 400.0f;

uniform vec3 diffuseColor = vec3(0.5,0.3,0.7);
uniform vec3 lightPosition = vec3(-0.5,0,0);
//layout (binding = 0) uniform sampler2D origin;
//layout (binding = 1) uniform sampler2D direction;

float intersectRaySphere(Ray ray,Sphere sphere, out vec3 hitPosition, out vec3 normal)
{
	vec3 rayToSphere = ray.origin - sphere.center;
	float a = dot(ray.direction, ray.direction);
	float b = 2.0 * dot(ray.direction, rayToSphere);
	float c = dot(rayToSphere,rayToSphere) - sphere.radius * sphere.radius;
	float b2 = b*b;

	float f = b2 - 4.0 * a * c;

	if(f < 0.0) return 0.0;

	float q = (b > 0.0) ? -0.5 * b + sqrt(f) : -0.5 * b - sqrt(f);

	float t0 = q/a;
	float t1 = c/q;

	float t = min(max(t0, 0.0), max(t1,0.0))*0.5;

	if(t == 0.0) return 0.0;

	hitPosition = ray.origin + t * ray.direction;
	normal = normalize(hitPosition - sphere.center);

	return t;
}


void main() {
	Ray ray;

	ray.origin = vec3(1.0);
	float u = gl_FragCoord.x /  width;
	float v = gl_FragCoord.y /  height;

	vec3 leftCorner = vec3(-2.0,-1.0,-1.0);
	vec3 horizontal = vec3(4.0,0.0,0.0);
	vec3 vertical = vec3(0.0,2.0,0.0);

	ray.direction = leftCorner + u * horizontal + v * vertical;

	Sphere sphere;
	sphere.center = vec3(0,0,-4);
	sphere.radius = 2.0f;

	vec3 hitPosition, normal;
	float t = intersectRaySphere(ray,sphere, hitPosition, normal);

	if(t != 0.0f) {
		vec3 lightDirection = normalize(hitPosition - lightPosition);
		float lightDotNormal = dot(lightDirection, normal);

		vec3 color = diffuseColor * lightDotNormal;

		fColor= vec4(color,1.0);
	}
	else {
		fColor = vec4(1,1,1,1);
	}
}
