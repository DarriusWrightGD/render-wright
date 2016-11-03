#version 440 core

out vec4 fColor;
in vec4 color;
void main() {
    fColor = color;
    //fColor = vec4(1,1,0,1);
}
