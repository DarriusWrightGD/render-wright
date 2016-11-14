#version 440 core

out vec4 fColor;
in vec3 texCoord;

layout(binding=0) uniform samplerCube tex;

void main() {
	//the negative one is a temp hack until I find a way to flip the image with soil
	//vec2 flippedTexCoord = vec2(texCoord.x, 1.0-texCoord.y);
    //fColor = mix(texture(tex,flippedTexCoord),texture(fbo,flippedTexCoord), .3);
	fColor= texture(tex,texCoord);
}
