#version 400 core

in vec3 fragPosition;
in vec3 normal;
in vec2 texCoord;

out vec4 Color;

uniform sampler2D outTexture1;
uniform sampler2D outSpecular;
uniform float ambientStrength;
uniform float specularStrength;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 viewPosition;
uniform float shininess;

void main()
{
	//Ambient
	vec3 ambient = ambientStrength * lightColor * vec3(texture(outTexture1, texCoord));;

	//Diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-lightDirection);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = lightColor * diff * vec3(texture(outTexture1, texCoord));

	//Specular
	vec3 viewDir = normalize(viewPosition - fragPosition);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
	vec3 specular = specularStrength * spec * lightColor * vec3(texture(outSpecular, texCoord));

	vec3 result = (ambient + diffuse + specular) * objectColor;
	Color = vec4(result, 1.0f);
}