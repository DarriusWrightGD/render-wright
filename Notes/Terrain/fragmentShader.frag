#version 440 core

out vec4 fColor;
in vec2 uv;
layout(binding=0) uniform sampler2D tex;
void main() {
	float height = texture(tex,uv).r;

	if(height > .6) {
		fColor = vec4(1);
	} else if (height > .4) {
		fColor = vec4(0.0,0.7,0.2,1.0);
	} else {
		fColor = vec4(0.0,0.2,0.8,1.0);
	}
}
