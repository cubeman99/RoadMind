#version 330 core

layout (location = 0) in vec3 a_vertPos;
layout (location = 1) in vec2 a_vertTexCoord;
layout (location = 2) in vec3 a_vertNormal;

out vec2 v_texCoord;
out vec3 v_vertNormal;

uniform mat4 u_mvp;

void main()
{
	vec3 pos = a_vertPos;
	gl_Position = u_mvp * vec4(pos, 1.0);
	v_texCoord = a_vertTexCoord;
	v_vertNormal = a_vertNormal;
}
