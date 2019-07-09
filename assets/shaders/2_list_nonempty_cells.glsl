// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch01.html

#version 430 compatibility
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "marching_cubes_tables.glsl"

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------

struct Vertex {
	vec4 first; // xyz = position.xyz, w = texCoord.x
	vec4 second; // x = texCoord.y, yzw = normal.xyz
};

struct DensityPoint
{
	vec4 point;
	vec4 biome;
};

//-----------------------------------------------------------------------------
// Buffers
//-----------------------------------------------------------------------------

layout (packed, binding = 0) buffer asdasd
{
	uvec4 b_z8_y8_x8_case8[];
};

layout (packed, binding = 1) buffer PointBuffer
{
	DensityPoint b_points[];
};

layout (packed, binding = 2) buffer LookupBuffer
{
	int b_triangulation[];
};

layout(binding = 3, offset = 0) uniform atomic_uint b_counter;

//-----------------------------------------------------------------------------
// Uniforms
//-----------------------------------------------------------------------------

uniform float u_surfaceDensity = 0.0;
uniform uvec3 u_resolution;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

uint indexFromCoord(uint x, uint y, uint z)
{
    return (((z * (u_resolution.y + 2)) + y) * (u_resolution.x + 2)) + x;
}

void main()
{
	uvec3 id = gl_GlobalInvocationID;
	if (b_points[0].biome.x ==255)
		return;

    // 8 corners of the current cube
    vec4 cubeCorners[8] = {
        b_points[indexFromCoord(id.x, id.y, id.z)].point,
        b_points[indexFromCoord(id.x + 1, id.y, id.z)].point,
        b_points[indexFromCoord(id.x + 1, id.y, id.z + 1)].point,
        b_points[indexFromCoord(id.x, id.y, id.z + 1)].point,
        b_points[indexFromCoord(id.x, id.y + 1, id.z)].point,
        b_points[indexFromCoord(id.x + 1, id.y + 1, id.z)].point,
        b_points[indexFromCoord(id.x + 1, id.y + 1, id.z + 1)].point,
        b_points[indexFromCoord(id.x, id.y + 1, id.z + 1)].point
    };

    // Calculate unique index for each cube configuration.
    // There are 256 possible values
    // A value of 0 means cube is entirely inside surface; 255 entirely outside.
    // The value is used to look up the edge table, which indicates which edges of the cube are cut by the isosurface.
    uint cubeIndex = 0;
    if (cubeCorners[0].w < u_surfaceDensity) cubeIndex |= 1;
    if (cubeCorners[1].w < u_surfaceDensity) cubeIndex |= 2;
    if (cubeCorners[2].w < u_surfaceDensity) cubeIndex |= 4;
    if (cubeCorners[3].w < u_surfaceDensity) cubeIndex |= 8;
    if (cubeCorners[4].w < u_surfaceDensity) cubeIndex |= 16;
    if (cubeCorners[5].w < u_surfaceDensity) cubeIndex |= 32;
    if (cubeCorners[6].w < u_surfaceDensity) cubeIndex |= 64;
    if (cubeCorners[7].w < u_surfaceDensity) cubeIndex |= 128;
	
	//uint index = atomicCounterIncrement(b_counter);
	//b_z8_y8_x8_case8[index] = cubeCorners[0];
	
    // Mark if this voxel needs triangles
	if (b_triangulation[cubeIndex * 16] != -1)
	{
		uint index = atomicCounterIncrement(b_counter);
		//b_z8_y8_x8_case8[index] = (id.z << 24) + (id.y << 16) + (id.x << 8) + cubeIndex;
		b_z8_y8_x8_case8[index].xyz = id;
		b_z8_y8_x8_case8[index].w = cubeIndex;
	}
}

