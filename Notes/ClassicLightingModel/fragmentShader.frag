#version 440 core

out vec4 fColor;
in vec2 texCoord;
in vec3 normal;

uniform vec3 ambient; //constant light
uniform vec3 diffuse;
uniform vec4 specular;

uniform vec4 lightDirection;
uniform vec4 eyeDirection;

layout(binding=0) uniform sampler2D tex;

subroutine vec4 LightFunc(vec4 color);

//this can be multiple subroutines
subroutine (LightFunc) vec4 ambientLighting(vec4 color) 
{
	vec3 finalColor =  min(color.xyz * ambient,vec3(1.0f));
	return vec4(finalColor, color.a);
}

subroutine (LightFunc) vec4 diffuseLighting(vec4 color)
{
	vec3 finalColor =  color.xyz + (diffuse * max(dot(lightDirection.xyz,normalize(normal)), 0.0));
	return vec4(finalColor,color.a);
}

subroutine (LightFunc) vec4 directionalLight(vec4 color)
{
	return vec4(1,0,0,1);
}

subroutine uniform LightFunc lighting;

void main() {
	vec2 flippedTexCoord = vec2(texCoord.x, 1.0f-texCoord.y);
	vec4 textureColor = texture(tex,flippedTexCoord);
	
    fColor = lighting(textureColor);
}
