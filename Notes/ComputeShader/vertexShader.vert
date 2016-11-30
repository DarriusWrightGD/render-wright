#version 440 core

layout (location = 0) in vec4 vPosition;

out vec2 texCoord;
void main(){
	texCoord = -vPosition.xy;
    gl_Position = vPosition;
}
