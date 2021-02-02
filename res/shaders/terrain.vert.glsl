#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec2 a_ShadowCoords;
layout(location = 4) in vec2 a_TextureCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Light;

out vec4 v_Color;
out vec3 v_Normal;
out vec3 v_Position;
out vec2 v_ShadowCoords;
out vec4 v_FragLight;
out vec2 v_TextureCoords;

void main()
{
	v_Position = vec3(u_Model * vec4(a_Position, 1.0f));
	v_Color = a_Color;
	v_Normal = transpose(inverse(mat3(u_Model))) * a_Normal;
	v_ShadowCoords = a_ShadowCoords;
	v_FragLight = u_Light * vec4(v_Position, 1.0);
	v_TextureCoords = a_TextureCoords;

	gl_Position = u_Projection * u_View * u_Model * vec4(v_Position, 1.0);
}