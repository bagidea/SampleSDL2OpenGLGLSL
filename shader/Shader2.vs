#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vertexColor;

out vec3 outColor;
uniform float scale;

void main()
{
	gl_Position = vec4(scale*position.x, scale*position.y, scale*position.z, 1.0f);
	outColor = vertexColor;
}