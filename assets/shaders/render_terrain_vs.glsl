#version 330 core

layout (location = 0) in vec3 a_vertPos;
layout (location = 1) in vec2 a_vertTexCoord;
layout (location = 2) in vec3 a_vertNormal;

out vec3 v_vertNormal;
out vec3 v_vertPos;
out vec3 v_vertColor;
out float v_ambientOcclusion;
out float v_fog;

uniform mat4 u_mvp;
uniform vec3 u_eyePos;
uniform float u_fogMinDistance = 0.0;
uniform float u_fogMaxDistance = 360.0;

vec3 biomeColors[6] = vec3[](
	vec3(1, 0, 0),
	vec3(1, 1, 0),
	vec3(0, 1, 0),
	vec3(0, 1, 1),
	vec3(0, 0, 1),
	vec3(1, 0, 1)
);

uniform sampler2D s_textureGrassColormap;

void main()
{
	gl_Position = u_mvp * vec4(a_vertPos, 1.0);
	v_vertNormal = a_vertNormal;
	v_vertPos = a_vertPos;

	float dist = length(a_vertPos - u_eyePos);
	v_fog = clamp((dist - u_fogMinDistance) / (u_fogMaxDistance - u_fogMinDistance), 0, 1);
	
	float temperature = a_vertTexCoord.x;
	float moisture = a_vertTexCoord.y;
	vec3 grassColor = texture2D(s_textureGrassColormap, vec2(1 - temperature, 1 - moisture)).rgb;

	//int biomeIndex = max(0, min(5, int(round(a_vertTexCoord.x))));
	//vec3 biomeColor = biomeColors[biomeIndex];
	v_vertColor = grassColor;
	v_ambientOcclusion = a_vertTexCoord.y;
}
