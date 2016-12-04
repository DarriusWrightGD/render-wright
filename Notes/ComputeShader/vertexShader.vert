#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec2 vUv;

out vec2 texCoord;
void main(){
	texCoord = vUv;
    gl_Position = vPosition;
}
