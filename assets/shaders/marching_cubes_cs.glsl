#version 430 compatibility

#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable


// Values from http://paulbourke.net/geometry/polygonise/

int edges[256] = int[](
    0x0,
    0x109,
    0x203,
    0x30a,
    0x406,
    0x50f,
    0x605,
    0x70c,
    0x80c,
    0x905,
    0xa0f,
    0xb06,
    0xc0a,
    0xd03,
    0xe09,
    0xf00,
    0x190,
    0x99,
    0x393,
    0x29a,
    0x596,
    0x49f,
    0x795,
    0x69c,
    0x99c,
    0x895,
    0xb9f,
    0xa96,
    0xd9a,
    0xc93,
    0xf99,
    0xe90,
    0x230,
    0x339,
    0x33,
    0x13a,
    0x636,
    0x73f,
    0x435,
    0x53c,
    0xa3c,
    0xb35,
    0x83f,
    0x936,
    0xe3a,
    0xf33,
    0xc39,
    0xd30,
    0x3a0,
    0x2a9,
    0x1a3,
    0xaa,
    0x7a6,
    0x6af,
    0x5a5,
    0x4ac,
    0xbac,
    0xaa5,
    0x9af,
    0x8a6,
    0xfaa,
    0xea3,
    0xda9,
    0xca0,
    0x460,
    0x569,
    0x663,
    0x76a,
    0x66,
    0x16f,
    0x265,
    0x36c,
    0xc6c,
    0xd65,
    0xe6f,
    0xf66,
    0x86a,
    0x963,
    0xa69,
    0xb60,
    0x5f0,
    0x4f9,
    0x7f3,
    0x6fa,
    0x1f6,
    0xff,
    0x3f5,
    0x2fc,
    0xdfc,
    0xcf5,
    0xfff,
    0xef6,
    0x9fa,
    0x8f3,
    0xbf9,
    0xaf0,
    0x650,
    0x759,
    0x453,
    0x55a,
    0x256,
    0x35f,
    0x55,
    0x15c,
    0xe5c,
    0xf55,
    0xc5f,
    0xd56,
    0xa5a,
    0xb53,
    0x859,
    0x950,
    0x7c0,
    0x6c9,
    0x5c3,
    0x4ca,
    0x3c6,
    0x2cf,
    0x1c5,
    0xcc,
    0xfcc,
    0xec5,
    0xdcf,
    0xcc6,
    0xbca,
    0xac3,
    0x9c9,
    0x8c0,
    0x8c0,
    0x9c9,
    0xac3,
    0xbca,
    0xcc6,
    0xdcf,
    0xec5,
    0xfcc,
    0xcc,
    0x1c5,
    0x2cf,
    0x3c6,
    0x4ca,
    0x5c3,
    0x6c9,
    0x7c0,
    0x950,
    0x859,
    0xb53,
    0xa5a,
    0xd56,
    0xc5f,
    0xf55,
    0xe5c,
    0x15c,
    0x55,
    0x35f,
    0x256,
    0x55a,
    0x453,
    0x759,
    0x650,
    0xaf0,
    0xbf9,
    0x8f3,
    0x9fa,
    0xef6,
    0xfff,
    0xcf5,
    0xdfc,
    0x2fc,
    0x3f5,
    0xff,
    0x1f6,
    0x6fa,
    0x7f3,
    0x4f9,
    0x5f0,
    0xb60,
    0xa69,
    0x963,
    0x86a,
    0xf66,
    0xe6f,
    0xd65,
    0xc6c,
    0x36c,
    0x265,
    0x16f,
    0x66,
    0x76a,
    0x663,
    0x569,
    0x460,
    0xca0,
    0xda9,
    0xea3,
    0xfaa,
    0x8a6,
    0x9af,
    0xaa5,
    0xbac,
    0x4ac,
    0x5a5,
    0x6af,
    0x7a6,
    0xaa,
    0x1a3,
    0x2a9,
    0x3a0,
    0xd30,
    0xc39,
    0xf33,
    0xe3a,
    0x936,
    0x83f,
    0xb35,
    0xa3c,
    0x53c,
    0x435,
    0x73f,
    0x636,
    0x13a,
    0x33,
    0x339,
    0x230,
    0xe90,
    0xf99,
    0xc93,
    0xd9a,
    0xa96,
    0xb9f,
    0x895,
    0x99c,
    0x69c,
    0x795,
    0x49f,
    0x596,
    0x29a,
    0x393,
    0x99,
    0x190,
    0xf00,
    0xe09,
    0xd03,
    0xc0a,
    0xb06,
    0xa0f,
    0x905,
    0x80c,
    0x70c,
    0x605,
    0x50f,
    0x406,
    0x30a,
    0x203,
    0x109,
    0x000
);

int cornerIndexAFromEdge[12] = int[](
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    0,
    1,
    2,
    3
);

int cornerIndexBFromEdge[12] = int[](
    1,
    2,
    3,
    0,
    5,
    6,
    7,
    4,
    4,
    5,
    6,
    7
);


struct Vertex {
    vec3 position;
	float pad1;
    vec2 texCoord;
	vec2 pad2;
    vec3 normal;
	float pad3;
};


layout (packed, binding = 0) buffer VertexBuffer
{
	Vertex vertices[];
};
layout (packed, binding = 1) buffer PointBuffer
{
	vec4 points[];
};
layout (packed, binding = 2) buffer LookupBuffer
{
	int triangulation[];
};
layout(binding = 3, offset = 0) uniform atomic_uint ac;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

float isoLevel = 0.0;
uniform uint u_width = 16;
uniform uint u_height = 16;
//uniform uint u_depth = 16;

vec3 interpolateVerts(vec4 v1, vec4 v2) {
    float t = (isoLevel - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

uint indexFromCoord(uint x, uint y, uint z) {
    return z * (u_width + 1) * (u_height + 1) + y * (u_width + 1) + x;
}

void main()
{
	uvec3 id = gl_GlobalInvocationID;

	/*
	uint triIndex = atomicCounterIncrement(ac);
	vertices[triIndex * 3].position = vec3(
		float(triIndex * 3),
		float(triIndex * 3 + 1),
		float(triIndex * 3 + 2));
	vertices[triIndex * 3 + 1].position = vec3(
		float(triIndex * 3 + 3),
		float(triIndex * 3 + 4),
		float(triIndex * 3 + 5));
	vertices[triIndex * 3 + 2].position = vec3(
		float(triIndex * 3 + 6),
		float(triIndex * 3 + 7),
		float(triIndex * 3 + 8));
	//vertices[triIndex * 3].texCoord = vec2(float(triIndex * 4), float(triIndex * 5));
	//vertices[triIndex * 3].normal = vec3(float(triIndex * 6), float(triIndex * 7), float(triIndex * 8));
	*/

	
    // 8 corners of the current cube
    vec4 cubeCorners[8] = {
        points[indexFromCoord(id.x, id.y, id.z)],
        points[indexFromCoord(id.x + 1, id.y, id.z)],
        points[indexFromCoord(id.x + 1, id.y, id.z + 1)],
        points[indexFromCoord(id.x, id.y, id.z + 1)],
        points[indexFromCoord(id.x, id.y + 1, id.z)],
        points[indexFromCoord(id.x + 1, id.y + 1, id.z)],
        points[indexFromCoord(id.x + 1, id.y + 1, id.z + 1)],
        points[indexFromCoord(id.x, id.y + 1, id.z + 1)]
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
		vertices[triIndex * 3].position = interpolateVerts(cubeCorners[a0], cubeCorners[b0]);
		vertices[triIndex * 3 + 1].position = interpolateVerts(cubeCorners[a1], cubeCorners[b1]);
		vertices[triIndex * 3 + 2].position = interpolateVerts(cubeCorners[a2], cubeCorners[b2]);

		vec3 a = vertices[triIndex * 3].position;
		vec3 b = vertices[triIndex * 3 + 1].position;
		vec3 c = vertices[triIndex * 3 + 2].position;
		vec3 normal = normalize(cross(a - b, b - c));
		vertices[triIndex * 3].normal = normal;
		vertices[triIndex * 3 + 1].normal = normal;
		vertices[triIndex * 3 + 2].normal = normal;
		
		vertices[triIndex * 3].texCoord = a.xy;
		vertices[triIndex * 3 + 1].texCoord = b.xy;
		vertices[triIndex * 3 + 2].texCoord = c.xy;
    }
}

