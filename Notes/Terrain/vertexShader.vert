#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUv;

layout(binding=0) uniform sampler2D heightMap;

out vec2 uv;
uniform mat4 mvp;
uniform float maxHeight =15.0f;
uniform float minHeight = -5.0f;


void main(){
	uv = vUv;
	float height= texture(heightMap, vUv).r;
	vec4 position = vPosition;
	float difference = (maxHeight- minHeight) * height;
	position.y += difference;

    gl_Position =  mvp * position;
}
