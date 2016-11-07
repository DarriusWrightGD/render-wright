#version 440 core

out vec4 fColor;
sample in vec4 color;

//sample information can be optained from in the shader through the parameters
//gl_SamplePosition and gl_SampleID

void main() {
    fColor = color;
}
