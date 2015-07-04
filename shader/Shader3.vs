#version 400 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Color;
layout (location = 2) in vec2 TexCoord;

out vec3 outColor;
out vec2 texCoord;

uniform float scale;

void main()
{
	gl_Position = vec4(scale * Position.x, scale * Position.y, scale * Position.z, 1.0f);
	outColor = Color;
	texCoord = vec2(TexCoord.x, 1.0f - TexCoord.y);
}