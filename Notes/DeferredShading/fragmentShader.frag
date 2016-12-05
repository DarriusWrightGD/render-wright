#version 440 core


layout(binding=0) uniform sampler2D gBuffer0;
layout(binding=1) uniform sampler2D gBuffer1;
layout(binding=2) uniform sampler2D gBuffer2;

struct FragInfo {
	vec3 textureColor;
	vec3 normal;
	vec3 position;
};

FragInfo unpackBuffer(ivec2 coord) {
	vec4 data0 = texelFetch(gBuffer0, coord,0);
	vec4 data1 = texelFetch(gBuffer1, coord,0);
	vec4 data2 = texelFetch(gBuffer2, coord,0);

	FragInfo fragmentInfo;

	fragmentInfo.textureColor = data0.xyz; //vec3(unpackHalf2x16(data0.x), temp.x);
	fragmentInfo.normal = data1.xyz;//normalize(vec3(temp.x,unpackHalf2x16(data0.z)));
	fragmentInfo.position = data2.xyz;

	return fragmentInfo;
}

out vec4 fColor;



uniform vec3 lightPositions[3];
uniform vec3 lightColors[3];

uniform vec3 ambientColor  = vec3(.2,.2,.2);
uniform vec3 diffuseColor  = vec3(1,1,1);
uniform int lightCount = 3;

vec4 light(FragInfo fragInfo)
{
	vec3 color;

	for(int i =0; i < lightCount; i++){
		if(length(fragInfo.position) !=0){
			vec3 lightDirection = normalize(fragInfo.position - lightPositions[i]);
			color += fragInfo.textureColor + (lightColors[i]* (ambientColor + diffuseColor * max(dot(lightDirection, fragInfo.normal), 0.0)));
		}
	}

	return vec4(color/lightCount,1.0);
}


void main() {
	//the negative one is a temp hack until I find a way to flip the image with soil
	FragInfo fragInfo = unpackBuffer(ivec2(gl_FragCoord.xy));
    fColor = light(fragInfo);//vec4(fragInfo.textureColor,1.0);
}
