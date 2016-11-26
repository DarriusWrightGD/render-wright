#version 440 core

out vec4 fColor;

uniform sampler2D heightMap;

in TES_OUT
{
  vec2 tc;
} fs_in;

void main() {

	float height = texture(heightMap,fs_in.tc).r;

	if(height > .6) {
		fColor = vec4(1);
	} else if (height > .4) {
		fColor = vec4(0.0,0.7,0.2,1.0);
	} else {
		fColor = vec4(0.0,0.2,0.8,1.0);
	}
}
