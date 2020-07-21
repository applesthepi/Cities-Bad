#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

out vec4 v_Color;
out vec3 v_Normal;
out vec3 v_Position;

void main()
{
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0f);
	v_Position = vec3(u_Model * vec4(a_Position, 1.0f));
	v_Color = a_Color;
	v_Normal = a_Normal;
}