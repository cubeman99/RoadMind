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

layout (packed, binding = 0) buffer nonempty_cells
{
	uvec4 b_z8_y8_x8_case8[];
};

layout (packed, binding = 1) buffer PointBuffer
{
	DensityPoint b_points[];
};

layout(binding = 2, offset = 0) uniform atomic_uint b_counter;

layout (packed, binding = 3) buffer VertexBuffer
{
	Vertex vertices[];
};
layout (packed, binding = 4) buffer IndexBuffer
{
	uint indices[];
};

layout (packed, binding = 5) buffer LookupBuffer
{
	int b_triangulation[];
};

//-----------------------------------------------------------------------------
// Uniforms
//-----------------------------------------------------------------------------

uniform uvec3 u_resolution;
uniform float u_surfaceDensity = 0.0;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

vec3 interpolateVerts(vec4 v1, vec4 v2)
{
    float t = (u_surfaceDensity - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

uint indexFromCoord(uint x, uint y, uint z)
{
    return (((z * (u_resolution.y + 1)) + y) * (u_resolution.x + 1)) + x;
}

void main()
{
	//uint value = b_z8_y8_x8_case8[gl_GlobalInvocationID.x];
	//uint z = (value >> 24) & 0xFF;
	//uint y = (value >> 16) & 0xFF;
	//uint x = (value >> 8) & 0xFF;
	//uint cubeIndex = value & 0xFF;
	uint x = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].x;
	uint y = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].y;
	uint z = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].z;
	uint cubeIndex = b_z8_y8_x8_case8[gl_GlobalInvocationID.x].w;
	
    // 8 corners of the current cube
    vec4 cubeCorners[8] = {
        b_points[indexFromCoord(x, y, z)].point,
        b_points[indexFromCoord(x + 1, y, z)].point,
        b_points[indexFromCoord(x + 1, y, z + 1)].point,
        b_points[indexFromCoord(x, y, z + 1)].point,
        b_points[indexFromCoord(x, y + 1, z)].point,
        b_points[indexFromCoord(x + 1, y + 1, z)].point,
        b_points[indexFromCoord(x + 1, y + 1, z + 1)].point,
        b_points[indexFromCoord(x, y + 1, z + 1)].point
    };

    // Create triangles for current cube configuration
	cubeIndex *= 16;
    for (int i = 0; b_triangulation[cubeIndex + i] != -1; i +=3) {
    
		// Get indices of corner points A and B for each of the three edges
        // of the cube that need to be joined to form the triangle.
        int a0 = cornerIndexAFromEdge[b_triangulation[cubeIndex + i + 0]];
        int b0 = cornerIndexBFromEdge[b_triangulation[cubeIndex + i + 0]];
        int a1 = cornerIndexAFromEdge[b_triangulation[cubeIndex + i + 1]];
        int b1 = cornerIndexBFromEdge[b_triangulation[cubeIndex + i + 1]];
        int a2 = cornerIndexAFromEdge[b_triangulation[cubeIndex + i + 2]];
        int b2 = cornerIndexBFromEdge[b_triangulation[cubeIndex + i + 2]];

		uint triIndex = atomicCounterIncrement(b_counter);
		vertices[triIndex * 3 + 0].first.xyz = interpolateVerts(cubeCorners[a0], cubeCorners[b0]);
		vertices[triIndex * 3 + 1].first.xyz = interpolateVerts(cubeCorners[a1], cubeCorners[b1]);
		vertices[triIndex * 3 + 2].first.xyz = interpolateVerts(cubeCorners[a2], cubeCorners[b2]);

		vec3 a = vertices[triIndex * 3 + 0].first.xyz;
		vec3 b = vertices[triIndex * 3 + 1].first.xyz;
		vec3 c = vertices[triIndex * 3 + 2].first.xyz;
		vec3 normal = -normalize(cross(a - b, b - c));
		float biome = b_points[indexFromCoord(x, y, z)].biome.x;
		vertices[triIndex * 3 + 0].second.yzw = normal;
		vertices[triIndex * 3 + 1].second.yzw = normal;
		vertices[triIndex * 3 + 2].second.yzw = normal;
		vertices[triIndex * 3 + 0].first.w = biome;
		vertices[triIndex * 3 + 1].first.w = biome;
		vertices[triIndex * 3 + 2].first.w = biome;
		vertices[triIndex * 3 + 0].second.x = biome;
		vertices[triIndex * 3 + 1].second.x = biome;
		vertices[triIndex * 3 + 2].second.x = biome;
		vertices[triIndex * 3 + 0].second.x = biome;
		vertices[triIndex * 3 + 1].second.x = biome;
		vertices[triIndex * 3 + 2].second.x = biome;
		indices[triIndex * 3 + 0] = triIndex * 3 + 0;
		indices[triIndex * 3 + 1] = triIndex * 3 + 1;
		indices[triIndex * 3 + 2] = triIndex * 3 + 2;
    }
}

