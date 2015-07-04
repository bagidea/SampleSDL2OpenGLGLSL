#version 400 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;

out vec3 fragPosition;
out vec3 normal;
out vec2 texCoord;

uniform mat4 gProjection;
uniform mat4 gView;
uniform mat4 gModel;

void main()
{
	gl_Position = gProjection * gView * gModel * vec4(Position, 1.0f);
	fragPosition = vec3(gModel * vec4(Position, 1.0f));
	normal = mat3(transpose(inverse(gModel))) * Normal;
	texCoord = vec2(TexCoord.x, 1.0f - TexCoord.y);
}