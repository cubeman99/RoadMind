#version 430 compatibility
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "noise.glsl"
#include "density_function.glsl"

struct DensityPoint
{
	vec4 point;
	vec4 biome;
};

layout (packed, binding = 0) buffer PointBuffer
{
	DensityPoint points[];
};


uniform uvec3 u_resolution;
uniform vec3 u_size;
uniform vec3 u_offset;

uint indexFromCoord(uint x, uint y, uint z)
{
    return (((z * (u_resolution.y + 2)) + y) * (u_resolution.x + 2)) + x;
}

void main()
{
	uvec3 id = gl_GlobalInvocationID;
	//if (id.x > u_resolution.x || id.y > u_resolution.y || id.z > u_resolution.z)
	//	return;

	vec3 pos = u_offset + vec3(float(id.x), float(id.y), float(id.z)) * (u_size / vec3(u_resolution));
	float density = densityFunction(pos);

    float noise = 0;

	uint index = indexFromCoord(id.x, id.y, id.z);
	points[index].point = vec4(pos, density);
	points[index].biome = vec4(27, 27, 27, 27);
}

