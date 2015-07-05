#version 400 core

in vec3 fragPosition;
in vec3 normal;
in vec2 texCoord;

out vec4 Color;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	vec3 color;
	float shininess;
};

struct DirectionalLight
{
	vec3 direction;

	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
};

struct PointLight
{
	vec3 position;

	float constant;
	float linear;
	float quadratic;
	
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;	
};

struct SpotLight
{
	vec3 position;
	vec3 direction;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;
	
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
};

uniform vec3 viewPosition;

uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight pointLight[3];
uniform SpotLight spotLight;

vec3 pushDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	//Diffuse
	float diff = max(dot(normal, lightDir), 0.0f);

	//Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);

	//Combine Result
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

	return (ambient + diffuse + specular); 
}

vec3 pushPointLight(PointLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPosition);

	//Diffuse
	float diff = max(dot(normal, lightDir), 0.0f);

	//Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);

	//Attenuation
	float distance = length(light.position - fragPosition);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	//Combine Result
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular); 
}

vec3 pushSpotLight(SpotLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPosition);

	//Diffuse
	float diff = max(dot(normal, lightDir), 0.0f);

	//Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);

	//Spot Light
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = (light.cutOff - light.outerCutOff);
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);

	//Attenuation
	float distance = length(light.position - fragPosition);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	//Combine Result
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

	diffuse *= intensity;
	specular *= intensity;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

void main()
{
	vec3 result;

	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(viewPosition - fragPosition);

	result = pushDirectionalLight(directionalLight ,norm, viewDir);

	for(int i = 0; i < 3; i++)
	{
		result += pushPointLight(pointLight[i], norm, viewDir);
	}

	result += pushSpotLight(spotLight, norm, viewDir);

	Color = vec4(result, 1.0f) * vec4(material.color, 1.0f);
}