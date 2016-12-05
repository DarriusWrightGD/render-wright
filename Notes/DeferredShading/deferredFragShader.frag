#version 440 core

layout (location =0) out vec4 color0;
layout (location =1) out vec4 color1;
layout (location =2) out vec4 color2;

in VS_OUT
{
	vec4 position;
	vec4 normal;
	vec2 texCoords;
}fs_in;

uniform sampler2D tex;

void main() {
	vec2 flippedTexCoord = vec2(fs_in.texCoords.x, 1.0-fs_in.texCoords.y);

	color0 = texture(tex,flippedTexCoord);
	color1 = fs_in.normal;
	color2 = fs_in.position;
}
