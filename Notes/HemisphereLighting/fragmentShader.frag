#version 440 core

out vec4 fColor;
in vec3 normal;
in vec2 texCoord;
in vec3 position;

uniform sampler2D tex;

uniform vec3 lightPosition = vec3(2.0f,-2.0f,-50.0f);
uniform vec3 skyColor = vec3(.3,.3,.3);
uniform vec3 groundColor = vec3(0.1,0.0,0.3);


void main() {
	vec3 lightDirection = normalize(lightPosition - position);
	float lightDotNormal = dot(lightDirection, normal);

	float mixAmount = lightDotNormal * 0.5 + 0.5;

	vec4 textureColor = texture(tex, vec2(texCoord.x, 1.0 - texCoord.y));

	vec3 ambientColor = mix(groundColor, skyColor,mixAmount);
	vec3 finalColor = textureColor.rgb + ambientColor;
    fColor = vec4(min(finalColor, vec3(1.0)),textureColor.a);
}
