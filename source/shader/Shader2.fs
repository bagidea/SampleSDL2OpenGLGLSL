#version 400 core

in vec3 outColor;
out vec4 Color;

void main()
{
	Color = vec4(outColor, 1.0f);
}