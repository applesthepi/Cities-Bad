#version 450 core

out vec4 o_Color;

in vec4 v_Color;
in vec3 v_Normal;
in vec3 v_Position;
in vec2 v_ShadowCoords;
in vec4 v_FragLight;

uniform sampler2D u_ShadowTexture;
uniform vec3 u_LightPos;
uniform vec3 u_ViewPos;
uniform vec4 u_Color;

float ShadowCalculation(vec4 fragLight)
{
	vec3 projCoords = v_FragLight.xyz / v_FragLight.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(u_ShadowTexture, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

	return shadow;
}

void main()
{
	//vec3 norm = normalize(v_Normal);
	//vec3 lightDir = normalize(u_LightDirection - v_Position);
	//
	//float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse = diff * u_LightColor;
	//
	//o_Color = u_Color * vec4(diffuse * v_Color.xyz, 1.0);

	//vec3 norm = normalize(v_Normal);
	//vec3 lightDir = normalize(u_LightDirection);
	//
	//float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse = diff * u_LightColor;
	//
	//o_Color = u_Color * vec4(diffuse * v_Color.xyz, 1.0);

	vec3 color = v_Color.rgb * u_Color.rgb;
	vec3 normal = normalize(v_Normal);
	vec3 lightColor = vec3(1.0, 0.8, 0.5);

	vec3 ambient = 0.5 * color;
	
	vec3 lightDir = normalize(u_LightPos - v_Position);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(u_ViewPos - v_Position);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;

	float shadow = ShadowCalculation(v_FragLight);
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

	o_Color = vec4(lighting, 1.0);
}