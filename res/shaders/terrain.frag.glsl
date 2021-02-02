#version 450 core

out vec4 o_Color;

in vec4 v_Color;
in vec3 v_Normal;
in vec3 v_Position;
in vec2 v_ShadowCoords;
in vec4 v_FragLight;
in vec2 v_TextureCoords;

uniform sampler2D u_ShadowTexture;
uniform sampler2D u_TextureDiffuse;
uniform sampler2D u_TextureNormal;
uniform vec3 u_LightPos;
uniform vec3 u_ViewPos;
uniform vec4 u_Color;
uniform float u_HalfWidth;

float ShadowCalculation(vec3 lightDir)
{
	vec3 projCoords = v_FragLight.xyz / v_FragLight.w;
	projCoords = projCoords * 0.5 + 0.5;

	if(projCoords.z > 1.0)
        return 1.0;
	
	float closestDepth = texture(u_ShadowTexture, vec2(projCoords.x, projCoords.y)).r;
	float currentDepth = projCoords.z;

	float bias = max(0.0005 * (1.0 - dot(v_Normal, lightDir)), 0.005);
	//float shadow = currentDepth - bias > closestDepth ? 0.0 : 1.0;


	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_ShadowTexture, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(u_ShadowTexture, vec2(projCoords.x, projCoords.y) + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

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

	//vec3 color = v_Color.rgb * u_Color.rgb;
	//vec3 normal = normalize(v_Normal);
	//vec3 lightColor = vec3(1.0, 0.7451, 0.3647) * vec3(5.0, 5.0, 5.0);

	//vec3 ambient = 0.5 * color;
	
	vec3 lightDir = normalize(u_LightPos - vec3(u_HalfWidth, 0.0, u_HalfWidth));
	//float diff = max(dot(lightDir, normal), 0.0);
	//vec3 diffuse = diff * lightColor;

	//vec3 viewDir = normalize(u_ViewPos - v_Position);
	//float spec = 0.01;
	//vec3 halfwayDir = normalize(lightDir + viewDir);
	//spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	//vec3 specular = spec * lightColor;

	float shadow = ShadowCalculation(lightDir);
	vec4 tDiffuse = texture(u_TextureDiffuse, v_TextureCoords);
	vec4 tNormal = texture(u_TextureNormal, v_TextureCoords);
	//float shadowValue = shadow < 0.5 ? 0.1 : 1.0;
	
	vec3 lighting = (1.0 - (min(shadow, 0.3) * (1.0 - min(0.2, dot(vec3(0.0, 1.0, 0.0), vec3(0.0, lightDir.y, 0.0)))))) * tDiffuse.rgb;
	//o_Color = vec4(lighting, 1.0);

	//float test = dot(vec3(0.0, 1.0, 0.0), vec3(0.0, lightDir.y, 0.0));
	//o_Color = vec4(tDiffuse.r, tDiffuse.g, tDiffuse.b, 1.0);
	o_Color = vec4(v_TextureCoords.x, 0.0, v_TextureCoords.y, 1.0);
}