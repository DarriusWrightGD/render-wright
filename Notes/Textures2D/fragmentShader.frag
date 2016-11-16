#version 440 core

out vec4 fColor;
in vec2 texCoord;

layout(binding=0) uniform sampler2D tex;
layout(binding=1) uniform sampler2D fbo;

void main() {
	//the negative one is a temp hack until I find a way to flip the image with soil
	vec2 flippedTexCoord = vec2(texCoord.x, 1.0-texCoord.y);
    //fColor = mix(texture(tex,flippedTexCoord),texture(fbo,flippedTexCoord), .3);
    fColor = texture(tex,flippedTexCoord);
}
