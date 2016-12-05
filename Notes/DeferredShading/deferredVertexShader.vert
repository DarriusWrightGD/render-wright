#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vNormal;
layout (location = 2) in vec2 vTexCoord;

out VS_OUT
{
	vec4 position;
	vec4 normal;
	vec2 texCoords;
}vs_out;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main(){
	vs_out.position = view * model * vPosition;
	vs_out.normal = vNormal;
	vs_out.texCoords = vTexCoord; 
    gl_Position = projection * vs_out.position;
}
