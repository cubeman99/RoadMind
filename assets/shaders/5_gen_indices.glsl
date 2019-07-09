// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch01.html

#version 430 compatibility
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "marching_cubes_tables.glsl"

//-----------------------------------------------------------------------------
// Buffers
//-----------------------------------------------------------------------------

layout (packed, binding = 0) buffer nonempty_cells
{
	uvec4 b_z8_y8_x8_case8[];
};

layout (packed, binding = 1) buffer bb_vertex_indices
{
	uint b_vertex_indices[];
};

layout (packed, binding = 2) buffer LookupBuffer
{
	int b_triangulation[];
};

layout (packed, binding = 3) buffer bb_indices
{
	uint b_indices[];
};

layout (binding = 4, offset = 0) uniform atomic_uint b_counter;

//-----------------------------------------------------------------------------
// Uniforms
//-----------------------------------------------------------------------------

uniform uvec3 u_resolution;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

uint indexFromCoord(uint x, uint y, uint z)
{
    return (((z * (u_resolution.y + 1)) + y) * (u_resolution.x + 1)) + x;
}

uint getEdgeVertexIndex(uvec3 location, uint edge)
{
	uvec3 loc = location;
	uint part = 0;
	if (edge == 0)
		part = 0;
	if (edge == 8)
		part = 1;
	if (edge == 3)
		part = 2;
	if (edge == 9)
	{
		part = 1;
		loc.x += 1;
	}
	if (edge == 10)
	{
		part = 1;
		loc.x += 1;
		loc.z += 1;
	}
	if (edge == 11)
	{
		part = 1;
		loc.z += 1;
	}
	if (edge == 4)
	{
		part = 0;
		loc.y += 1;
	}
	if (edge == 6)
	{
		part = 0;
		loc.y += 1;
		loc.z += 1;
	}
	if (edge == 2)
	{
		part = 0;
		loc.z += 1;
	}
	if (edge == 1)
	{
		part = 2;
		loc.x += 1;
	}
	if (edge == 5)
	{
		part = 2;
		loc.x += 1;
		loc.y += 1;
	}
	if (edge == 7)
	{
		part = 2;
		loc.y += 1;
	}

	return b_vertex_indices[indexFromCoord(loc.x, loc.y, loc.z) * 3 + part];
}

void main()
{
	uvec3 loc = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].xyz;
	if (loc.x >= u_resolution.x || loc.y >= u_resolution.y || loc.z >= u_resolution.y)
		return;
	uint cubeIndex = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].w * 16;

    // Create triangles for current cube configuration
    for (int i = 0; b_triangulation[cubeIndex + i] != -1; i += 3)
	{
		uint triIndex = atomicCounterIncrement(b_counter);
		b_indices[triIndex * 3 + 0] = getEdgeVertexIndex(loc, b_triangulation[cubeIndex + i + 0]);
		b_indices[triIndex * 3 + 1] = getEdgeVertexIndex(loc, b_triangulation[cubeIndex + i + 1]);
		b_indices[triIndex * 3 + 2] = getEdgeVertexIndex(loc, b_triangulation[cubeIndex + i + 2]);
    }
}

