#version 400 core

in vec2 texCoord;

out vec4 Color;

uniform sampler2D outTexture1;

void main()
{
	vec4 tex = texture(outTexture1, texCoord);

	if(tex.a < 0.1f)
		discard;

	Color = tex;
}