#version 440 core

layout (location = 0) in vec4 vPosition;

uniform mat4 model;

layout(std140, binding=0) uniform TransformBlock
{
	mat4 view;
	mat4 projection;
} transform;

void main(){
    gl_Position = transform.projection * transform.view * model * vPosition;
}
