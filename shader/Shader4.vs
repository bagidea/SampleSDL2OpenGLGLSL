#version 400 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

out vec2 texCoord;

uniform mat4 gWorld;

void main()
{
	gl_Position = gWorld * vec4(Position, 1.0f);
	texCoord = vec2(TexCoord.x, 1.0f - TexCoord.y);
}