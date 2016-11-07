#version 440 core

layout (location = 0) in vec4 vPosition;
out vec4 color;
void main(){
    gl_Position = vPosition;
	color = vec4(0.5f,0.0f,1.0f,1.0f);
}
