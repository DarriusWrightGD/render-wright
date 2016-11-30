#version 440 core

out vec4 fColor;

layout(binding=0) uniform sampler2D tex;

in vec2 texCoord;

void main() {
    fColor = texture(tex,texCoord);
}
