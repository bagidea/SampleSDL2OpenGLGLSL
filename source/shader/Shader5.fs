#version 400 core

in vec3 fragPosition;
in vec3 normal;
in vec2 texCoord;

out vec4 Color;

uniform sampler2D outTexture1;
uniform float ambientStrength;
uniform float specularStrength;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPosition;

void main()
{
	//Ambient
	vec3 ambient = ambientStrength * lightColor * vec3(texture(outTexture1, texCoord));;

	//Diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPosition - fragPosition);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = lightColor * diff * vec3(texture(outTexture1, texCoord));

	//Specular
	vec3 viewDir = normalize(viewPosition - fragPosition);
	vec3 reflectDir = reflect(lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 64);
	vec3 specular = specularStrength * spec * lightColor * vec3(texture(outTexture1, texCoord));;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	Color = vec4(result, 1.0f);
}