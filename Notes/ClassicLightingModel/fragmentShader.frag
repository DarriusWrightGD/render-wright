#version 440 core

out vec4 fColor;
in vec2 texCoord;
in vec3 normal;
in vec3 position;

//light values
uniform vec3 ambient; 
uniform vec3 diffuse;
uniform vec4 specular;
uniform float specularStrength;

//point light
uniform float constantAttenuation;
uniform float linearAttenuation;
uniform float quadraticAttenuation;

//spot light
uniform vec3 coneDirection;
uniform float spotExponent;
uniform float spotCosCutoff;

uniform vec4 light; // this can be direction or position
uniform vec4 eye; // this can be direction or position

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
	vec3 finalColor =  color.xyz * (diffuse * max(dot(normalize(light.xyz),normalize(normal)), 0.0));
	return vec4(finalColor,color.a);
}

subroutine (LightFunc) vec4 specularLighting(vec4 color)
{

	vec3 halfVector = normalize(normalize(light.xyz) + normalize(eye.xyz));

	float lightDotNormal = max(dot(normalize(light.xyz), normalize(normal)), 0.0);
	float halfDotNormal = max(dot(halfVector, normalize(normal)),0.0);

	float specularPower = (lightDotNormal == 0.0) ? 0.0 : pow(halfDotNormal, specular.w);

	vec3 reflectedLight = specular.xyz * specularPower * specularStrength;

	return vec4(color.xyz + reflectedLight, color.a);
}

subroutine (LightFunc) vec4 directionalLighting(vec4 color)
{
	vec3 ambientColor = ambient;

	float lightDotNormal = max(dot(light.xyz,normalize(normal)), 0.0);
	vec3 diffuseColor = (diffuse * lightDotNormal);

	vec3 halfVector = normalize(light.xyz + eye.xyz);
	float halfDotNormal = max(dot(normal,halfVector),0.0);

	float specularPower = (lightDotNormal == 0) ? 0.0 : pow(halfDotNormal, specular.w);

	vec3 scatteredLight = ambientColor + diffuseColor;
	vec3 reflectedLight = specular.xyz * specularPower * specularStrength;// strength

	vec3 rgb = min(color.rgb * scatteredLight + reflectedLight, vec3(1.0));
	
	return vec4(rgb, color.a);
}

subroutine (LightFunc) vec4 pointLighting(vec4 color)
{
	vec3 lightDirection = light.xyz - position.xyz;
	float lightDistance = length(lightDirection);
	lightDirection /= lightDistance;
	//lightDistance = 1.0;
	float attenuation = 1.0 / (constantAttenuation  
		+ linearAttenuation * lightDistance 
		+ quadraticAttenuation * lightDistance * lightDistance );

	vec3 halfVector = normalize(lightDirection + eye.xyz);

	float lightDotNormal = max(0.0, dot(lightDirection, normal));
	float normalDotHalf = max(0.0, dot(normal, halfVector));

	float specularPower = (lightDotNormal == 0.0) ? 0.0 : pow(normalDotHalf, specular.w) * specularStrength;
	
	vec3 scatteredLight = ambient.xyz + (diffuse.xyz * lightDotNormal * attenuation);
	vec3 reflectedLight = specular.xyz * specularPower * attenuation;

	vec3 rgb = min(color.xyz * scatteredLight + reflectedLight,vec3(1.0));
	//vec3 rgb = min((diffuse.xyz * lightDotNormal * attenuation),vec3(1.0));
	return vec4(rgb, color.a);
}

subroutine (LightFunc) vec4 spotLighting(vec4 color) {
	vec3 lightDirection = normalize(light.xyz - position.xyz);

	float spotCos = acos(dot(-lightDirection, coneDirection));
	float cutoff = radians(clamp(spotCosCutoff, 0.0f,90.0f));

	if(spotCos < cutoff) {
		float spotFactor = pow(dot(-lightDirection, coneDirection), spotExponent);

		vec3 halfVector = normalize(lightDirection + eye.xyz);

		float lightDotNormal = max(0.0, dot(lightDirection, normal));
		float normalDotHalf = max(0.0, dot(normal, halfVector));

		float specularPower = (lightDotNormal == 0.0) ? 0.0 : pow(normalDotHalf, specular.w);
	
		vec3 scatteredLight = ambient.xyz + (diffuse.xyz * lightDotNormal * spotFactor);
		vec3 reflectedLight = specular.xyz * specularPower;

		vec3 rgb = min(color.xyz * scatteredLight + reflectedLight,vec3(1.0));

		return vec4(rgb, color.a);
	}else{
		return vec4(color.xyz + ambient, color.a);
	}
}

subroutine uniform LightFunc lighting;

void main() {
	vec2 flippedTexCoord = vec2(texCoord.x, 1.0f-texCoord.y);
	vec4 textureColor = texture(tex,flippedTexCoord);
	
    fColor = lighting(textureColor);
}
