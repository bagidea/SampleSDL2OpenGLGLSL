#version 400 core

in vec2 texCoord;

out vec4 Color;

uniform sampler2D outTexture1;

void main()
{
	Color = texture(outTexture1, texCoord);
}