#version 450 core

out vec4 o_Color;

void main()
{
	o_Color = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}