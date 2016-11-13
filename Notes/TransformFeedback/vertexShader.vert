#version 440 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vNormal;

uniform mat4 model;
uniform mat4 projection;

out vec4 position;
out vec3 normal;

void main(){
	position = model * vPosition;
	normal = normalize((model * vec4(vNormal,0.0)).xyz);
    gl_Position = projection * position;
}
