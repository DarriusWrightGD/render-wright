#version 440 core

out vec4 fColor;
uniform sampler2D pointSprite;

void main() {
	
    vec4 texColor = texture(pointSprite, gl_PointCoord);
	if(length(texColor.rgb) == 0.0) discard;

	fColor = texColor;
}
