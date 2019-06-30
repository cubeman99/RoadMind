#version 430 compatibility

#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#include "noise.glsl"


layout (local_size_x = 4, local_size_y = 4, local_size_z = 1) in;


struct Vertex {
	vec4 first; // xyz = position.xyz, w = texCoord.x
	vec4 second; // x = texCoord.y, yzw = normal.xyz
};

layout (packed, binding = 0) buffer VertexBuffer
{
	Vertex vertices[];
};


uniform uvec3 u_resolution;
uniform vec3 u_size;
uniform vec3 u_offset;
uniform float u_floorPosition;


void main()
{
	uvec3 id = gl_GlobalInvocationID;
	if (id.x >= u_resolution.x + 3 || id.y >= u_resolution.y + 3)
		return;

	float frequency =  1.0 / 200.0;

	vec3 position = (vec3(id) - vec3(1)) * (u_size / vec3(u_resolution));
	position += u_offset;
	position.z = u_floorPosition + octave_noise(vec3(position.xy, 0.0), 6, frequency, 80.0);

	uint index = (id.y * (u_resolution.x + 3)) + id.x;
	
	vertices[index].first.xyz = position;
	vertices[index].first.w = 0.0;
	vertices[index].second.x = 0.0;
	vertices[index].second.yzw = vec3(0, 0, 1);
	
	float temperature = -position.z / 60.0;
	temperature = (temperature + 1) * 0.5;
	temperature = max(0, min(1, temperature));

	float moisture = (octave_noise(position * vec3(-1,-1,0), 3, frequency, 1.0) + 1) * 0.5;
	moisture = max(0, min(1, moisture)) * temperature;
	vertices[index].first.w = temperature;
	vertices[index].second.x = moisture;

}

