#version 440 core

layout (location = 0) in vec4 vPosition;

void main(){
	float x = gl_InstanceID * 0.3;
    gl_Position = vec4(vPosition.x + x, vPosition.yzw);
}
