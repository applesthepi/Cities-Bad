#version 450 core

layout (location = 0) out vec4 o_Color;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexID;

uniform vec4 u_Color;
uniform sampler2D u_Textures[2];

void main()
{
	//o_Color = vec4(1.0, 1.0, 1.0, 1.0);
	int index = int(v_TexID);
	o_Color = texture(u_Textures[index], v_TexCoord) * v_Color;
} 