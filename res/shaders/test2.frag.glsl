#version 450 core

out vec4 o_Color;

uniform sampler2D u_Texture;

in vec2 v_Coords;

void main()
{
	o_Color = texture(u_Texture, v_Coords);
}