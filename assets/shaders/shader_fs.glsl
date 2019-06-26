#version 330 core

in vec2 v_texCoord;
in vec3 v_vertNormal;

out vec4 o_color;

uniform sampler2D s_diffuse;
uniform vec4 u_color = vec4(1, 1, 1, 1);

void main()
{
	vec4 diffuse = texture2D(s_diffuse, v_texCoord);
	vec3 lightDir = normalize(vec3(0.2, 0.6, -1));
	float light = dot(v_vertNormal, -lightDir);
	light = (light + 1.0) * 0.5;
	o_color = diffuse * u_color * vec4(vec3(light), 1.0);
}

