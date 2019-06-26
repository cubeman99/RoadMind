#version 330 core

layout (location = 0) in vec3 a_vertPos;
layout (location = 2) in vec3 a_vertNormal;

out vec3 v_vertNormal;
out vec3 v_vertPos;

uniform mat4 u_mvp;

void main()
{
	gl_Position = u_mvp * vec4(a_vertPos, 1.0);
	v_vertNormal = a_vertNormal;
	v_vertPos = a_vertPos;
}
