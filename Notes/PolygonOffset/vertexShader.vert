#version 440 core

layout (location = 0) in vec4 vPosition;

uniform mat4 mvp;
uniform vec4 color;

out vec4 oColor;

void main(){
    gl_Position = mvp * vPosition;
	oColor = color;
}
