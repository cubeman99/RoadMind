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

layout(binding = 1, offset = 0) uniform atomic_uint b_counter;

layout (packed, binding = 2) buffer edges_0_3_8_cells
{
	uvec4 b_z8_y8_x8_null4_edge4[];
};

//-----------------------------------------------------------------------------
// Uniforms
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void main()
{
	uint x = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].x;
	uint y = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].y;
	uint z = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].z;
	uint cubeIndex = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].w;
	int neededEdges = MARCHING_CUBES_EDGE_TABLE[cubeIndex];
	uint index;

	// Edge 0 (+x)
	if ((neededEdges & 0x1) != 0)
	{
		index = atomicCounterIncrement(b_counter);
		b_z8_y8_x8_null4_edge4[index].x = x;
		b_z8_y8_x8_null4_edge4[index].y = y;
		b_z8_y8_x8_null4_edge4[index].z = z;
		b_z8_y8_x8_null4_edge4[index].w = 0;
	}
	
	// Edge 8 (+y)
	if ((neededEdges & (1 << 8)) != 0)
	{
		index = atomicCounterIncrement(b_counter);
		b_z8_y8_x8_null4_edge4[index].x = x;
		b_z8_y8_x8_null4_edge4[index].y = y;
		b_z8_y8_x8_null4_edge4[index].z = z;
		b_z8_y8_x8_null4_edge4[index].w = 8;
	}

	// Edge 3 (+z)
	if ((neededEdges & (1 << 3)) != 0)
	{
		index = atomicCounterIncrement(b_counter);
		b_z8_y8_x8_null4_edge4[index].x = x;
		b_z8_y8_x8_null4_edge4[index].y = y;
		b_z8_y8_x8_null4_edge4[index].z = z;
		b_z8_y8_x8_null4_edge4[index].w = 3;
	}
}

