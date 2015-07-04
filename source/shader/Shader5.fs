#version 400 core

in vec3 fragPosition;
in vec3 normal;
in vec2 texCoord;

out vec4 Color;

uniform sampler2D outTexture1;
uniform float ambientStrength;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;

void main()
{
	//Ambient
	vec3 ambient = ambientStrength * lightColor;

	//Diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPosition - fragPosition);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	vec3 result = (ambient + diffuse) * objectColor;

	Color = texture(outTexture1, texCoord) * vec4(result, 1.0f);
}