#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vColor;
layout (location = 2) in mat4 vMvp;

out vec4 color; 

void main(){
	color = vec4(vColor,1.0);
    gl_Position = vMvp * vPosition;
}
