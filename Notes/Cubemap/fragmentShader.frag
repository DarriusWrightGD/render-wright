#version 440 core

out vec4 fColor;
in vec3 texCoord;
in vec3 normal;

layout(binding=0) uniform samplerCube tex;
uniform bool environmentMap;
void main() {
	if(environmentMap) {
		vec3 coords = reflect(-texCoord, normal);
		fColor= vec4(0.3,0.2,0.1,1.0) + vec4(0.97,0.83,0.79,0.0) * texture(tex,coords);
	}else {
		fColor= texture(tex,texCoord);
	}
}
