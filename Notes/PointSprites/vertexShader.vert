#version 440 core

layout (location = 0) in vec4 vPosition;

uniform mat4 proj;
void main(){
	gl_PointSize = -vPosition.z/15.0f;
    gl_Position = proj * vPosition;
}
