#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vNormal;
layout (location = 2) in vec2 vTexCoord;

uniform mat4 viewProjection;
uniform mat4 model;
out vec2 texCoord;

void main(){
	texCoord = vTexCoord;
    gl_Position = viewProjection * model * vPosition;
}
