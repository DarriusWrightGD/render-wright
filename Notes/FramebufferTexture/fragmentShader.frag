#version 440 core

out vec4 fColor;
in vec2 texCoord;

uniform sampler2D fbo;
uniform bool textureTri;


void main() {
    fColor = (textureTri) ? texture(fbo,texCoord) : vec4(0.1,0.2,1,1);
}
