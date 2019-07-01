#version 430 compatibility

#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#include "marching_cubes_tables.glsl"


struct Vertex {
	vec4 first; // xyz = position.xyz, w = texCoord.x
	vec4 second; // x = texCoord.y, yzw = normal.xyz
};

struct DensityPoint
{
	vec4 point;
	vec4 biome;
};

layout (packed, binding = 0) buffer VertexBuffer
{
	Vertex vertices[];
};
layout (packed, binding = 1) buffer PointBuffer
{
	DensityPoint points[];
};
layout (packed, binding = 2) buffer LookupBuffer
{
	int triangulation[];
};
layout(binding = 3, offset = 0) uniform atomic_uint ac;
layout (packed, binding = 4) buffer IndexBuffer
{
	uint indices[];
};

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

float isoLevel = 0.0;
uniform uvec3 u_resolution;

vec3 interpolateVerts(vec4 v1, vec4 v2) {
    float t = (isoLevel - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

uint indexFromCoord(uint x, uint y, uint z) {
    return z * (u_resolution.x + 1) * (u_resolution.y + 1) + y * (u_resolution.z + 1) + x;
}

void main()
{
	uvec3 id = gl_GlobalInvocationID;
	float biome = points[indexFromCoord(id.x, id.y, id.z)].biome.x;

    // 8 corners of the current cube
    vec4 cubeCorners[8] = {
        points[indexFromCoord(id.x, id.y, id.z)].point,
        points[indexFromCoord(id.x + 1, id.y, id.z)].point,
        points[indexFromCoord(id.x + 1, id.y, id.z + 1)].point,
        points[indexFromCoord(id.x, id.y, id.z + 1)].point,
        points[indexFromCoord(id.x, id.y + 1, id.z)].point,
        points[indexFromCoord(id.x + 1, id.y + 1, id.z)].point,
        points[indexFromCoord(id.x + 1, id.y + 1, id.z + 1)].point,
        points[indexFromCoord(id.x, id.y + 1, id.z + 1)].point
    };

    // Calculate unique index for each cube configuration.
    // There are 256 possible values
    // A value of 0 means cube is entirely inside surface; 255 entirely outside.
    // The value is used to look up the edge table, which indicates which edges of the cube are cut by the isosurface.
    int cubeIndex = 0;
    if (cubeCorners[0].w < isoLevel) cubeIndex |= 1;
    if (cubeCorners[1].w < isoLevel) cubeIndex |= 2;
    if (cubeCorners[2].w < isoLevel) cubeIndex |= 4;
    if (cubeCorners[3].w < isoLevel) cubeIndex |= 8;
    if (cubeCorners[4].w < isoLevel) cubeIndex |= 16;
    if (cubeCorners[5].w < isoLevel) cubeIndex |= 32;
    if (cubeCorners[6].w < isoLevel) cubeIndex |= 64;
    if (cubeCorners[7].w < isoLevel) cubeIndex |= 128;
	
    // Create triangles for current cube configuration
	cubeIndex *= 16;
    for (int i = 0; triangulation[cubeIndex + i] != -1; i +=3) {
    
		// Get indices of corner points A and B for each of the three edges
        // of the cube that need to be joined to form the triangle.
        int a0 = cornerIndexAFromEdge[triangulation[cubeIndex + i]];
        int b0 = cornerIndexBFromEdge[triangulation[cubeIndex + i]];

        int a1 = cornerIndexAFromEdge[triangulation[cubeIndex + i + 1]];
        int b1 = cornerIndexBFromEdge[triangulation[cubeIndex + i + 1]];

        int a2 = cornerIndexAFromEdge[triangulation[cubeIndex + i + 2]];
        int b2 = cornerIndexBFromEdge[triangulation[cubeIndex + i + 2]];

		uint triIndex = atomicCounterIncrement(ac);
		vertices[triIndex * 3].first.xyz = interpolateVerts(cubeCorners[a0], cubeCorners[b0]);
		vertices[triIndex * 3 + 1].first.xyz = interpolateVerts(cubeCorners[a1], cubeCorners[b1]);
		vertices[triIndex * 3 + 2].first.xyz = interpolateVerts(cubeCorners[a2], cubeCorners[b2]);

		vec3 a = vertices[triIndex * 3].first.xyz;
		vec3 b = vertices[triIndex * 3 + 1].first.xyz;
		vec3 c = vertices[triIndex * 3 + 2].first.xyz;
		vec3 normal = -normalize(cross(a - b, b - c));
		vertices[triIndex * 3].second.yzw = normal;
		vertices[triIndex * 3 + 1].second.yzw = normal;
		vertices[triIndex * 3 + 2].second.yzw = normal;
		vertices[triIndex * 3].first.w = biome;
		vertices[triIndex * 3 + 1].first.w = biome;
		vertices[triIndex * 3 + 2].first.w = biome;
		vertices[triIndex * 3].second.x = biome;
		vertices[triIndex * 3 + 1].second.x = biome;
		vertices[triIndex * 3 + 2].second.x = biome;
		vertices[triIndex * 3].second.x = biome;
		vertices[triIndex * 3 + 1].second.x = biome;
		vertices[triIndex * 3 + 2].second.x = biome;
		indices[triIndex * 3 + 0] = triIndex * 3 + 0;
		indices[triIndex * 3 + 1] = triIndex * 3 + 1;
		indices[triIndex * 3 + 2] = triIndex * 3 + 2;
    }
}

