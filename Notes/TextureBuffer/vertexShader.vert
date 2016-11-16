#version 440 core

layout (location = 0) in vec4 vPosition;

out vec4 color;
uniform samplerBuffer colors;

void main(){
	//vertexId = gl_VertexID;
    color = texelFetch(colors,gl_VertexID);
	gl_Position = vPosition;
}
