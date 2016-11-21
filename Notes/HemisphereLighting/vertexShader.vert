#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 mvp;
uniform mat3 normalMatrix;
out vec3 normal;
out vec3 position;
out vec2 texCoord;

void main(){
	position = (model * vPosition).xyz;
	normal = normalMatrix * vNormal;
	texCoord = vTexCoord;
    gl_Position = mvp * vPosition;
}
