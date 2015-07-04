#version 400 core

in vec3 outColor;
in vec2 texCoord;

out vec4 Color;

uniform sampler2D outTexture1;
uniform sampler2D outTexture2;

void main()
{
	Color = mix(texture(outTexture1, texCoord), texture(outTexture2, texCoord), 0.2f) * vec4(outColor, 1.0f);
}