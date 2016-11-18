#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vNormal;
layout (location = 2) in vec2 vTexCoord;

uniform mat4 viewProjection;
uniform mat4 model;
uniform mat3 normalMatrix;
out vec2 texCoord;
out vec3 normal;
out vec3 position;

void main(){
	texCoord = vTexCoord;
	normal =  normalize(normalMatrix * vNormal.xyz);
	position =  (model * vPosition).xyz;
    gl_Position = viewProjection * vec4(position,1.0);
}
