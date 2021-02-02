#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_Coords;

uniform mat4 u_MVP;

out vec2 v_Coords;

void main()
{
	v_Coords = a_Coords;
	gl_Position = u_MVP * vec4(a_Position, 1.0f);
}