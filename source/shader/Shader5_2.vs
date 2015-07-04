#version 400 core

layout (location = 0) in vec3 Position;

uniform mat4 gProjection;
uniform mat4 gView;
uniform mat4 gModel;

void main()
{
	gl_Position = gProjection * gView * gModel * vec4(Position, 1.0f);
}