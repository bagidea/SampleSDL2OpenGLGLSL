#version 400 core

out vec4 Color;

uniform vec3 objectColor;

void main()
{
	Color = vec4(objectColor, 1.0f);
}