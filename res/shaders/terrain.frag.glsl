#version 450 core

out vec4 o_Color;

in vec4 v_Color;
in vec3 v_Normal;
in vec3 v_Position;

uniform vec4 u_Color;
uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;

void main()
{
	//vec3 norm = normalize(v_Normal);
	//vec3 lightDir = normalize(u_LightDirection - v_Position);
	//
	//float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse = diff * u_LightColor;
	//
	//o_Color = u_Color * vec4(diffuse * v_Color.xyz, 1.0);

	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(u_LightDirection);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * u_LightColor;

	o_Color = u_Color * vec4(diffuse * v_Color.xyz, 1.0);
}