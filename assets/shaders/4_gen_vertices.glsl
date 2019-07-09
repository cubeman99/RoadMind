// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch01.html

#version 430 compatibility
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "marching_cubes_tables.glsl"
#include "noise.glsl"
#include "density_function.glsl"


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

layout (packed, binding = 0) buffer PointBuffer
{
	DensityPoint b_points[];
};

layout (packed, binding = 1) buffer edges_0_3_8_cells
{
	uvec4 b_z8_y8_x8_null4_edge4[];
};

layout (packed, binding = 2) buffer VertexBuffer
{
	Vertex b_vertices[];
};

layout (packed, binding = 3) buffer bb_vertex_indices
{
	uint b_vertex_indices[];
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

uint indexFromCoordDensity(uvec3 loc)
{
    return (((loc.z * (u_resolution.y + 2)) + loc.y) * (u_resolution.x + 2)) + loc.x;
}

uint indexFromCoord(uint x, uint y, uint z)
{
    return (((z * (u_resolution.y + 1)) + y) * (u_resolution.x + 1)) + x;
}

vec3 getVertexNormal(vec3 position)
{
	vec3 d = 1.0 / vec3(u_resolution);
	vec3 grad;
	grad.x = densityFunction(position + vec3( d.x,  0,  0)) -
			 densityFunction(position + vec3(-d.x,  0,  0));
	grad.y = densityFunction(position + vec3( 0,  d.y,  0)) -
			 densityFunction(position + vec3( 0, -d.y,  0));
	grad.z = densityFunction(position + vec3( 0,  0,  d.z)) -
			 densityFunction(position + vec3( 0,  0, -d.z));
	return -normalize(grad);
}

vec3 ray_dir[32] = {
    // 32 rays with a nice poisson distribution on a sphere:
    vec3( 0.286582 , 	 0.257763 ,	 -0.922729    ),
    vec3( -0.171812, 	 -0.888079, 	 0.426375   ),
    vec3( 0.440764 , 	 -0.502089, 	 -0.744066    ),
    vec3( -0.841007, 	 -0.428818, 	 -0.329882  ),
    vec3( -0.380213, 	 -0.588038, 	 -0.713898  ),
    vec3( -0.055393, 	 -0.207160, 	 -0.976738  ),
    vec3( -0.901510, 	 -0.077811, 	 0.425706   ),
    vec3( -0.974593, 	 0.123830 ,	 -0.186643  ),
    vec3( 0.208042 ,	 -0.524280, 	 0.825741     ),
    vec3( 0.258429 ,	 -0.898570, 	 -0.354663    ),
    vec3( -0.262118, 	 0.574475 ,	 -0.775418  ),
    vec3( 0.735212 ,	 0.551820 ,	 0.393646     ),
    vec3( 0.828700 ,	 -0.523923, 	 -0.196877    ),
    vec3( 0.788742 ,	 0.005727 ,	 -0.614698    ),
    vec3( -0.696885, 	 0.649338 ,	 -0.304486  ),
    vec3( -0.625313, 	 0.082413 ,	 -0.776010  ),
    vec3( 0.358696 ,	 0.928723 ,	 0.093864     ),
    vec3( 0.188264 ,	 0.628978 ,	 0.754283     ),
    vec3( -0.495193, 	 0.294596 ,	 0.817311   ),
    vec3( 0.818889 ,	 0.508670 ,	 -0.265851    ),
    vec3( 0.027189 ,	 0.057757 ,	 0.997960     ),
    vec3( -0.188421, 	 0.961802 ,	 -0.198582  ),
    vec3( 0.995439 ,	 0.019982 ,	 0.093282     ),
    vec3( -0.315254, 	 -0.925345, 	 -0.210596  ),
    vec3( 0.411992 ,	 -0.877706, 	 0.244733     ),
    vec3( 0.625857 ,	 0.080059 ,	 0.775818     ),
    vec3( -0.243839, 	 0.866185 ,	 0.436194   ),
    vec3( -0.725464, 	 -0.643645, 	 0.243768   ),
    vec3( 0.766785 ,	 -0.430702, 	 0.475959     ),
    vec3( -0.446376, 	 -0.391664, 	 0.804580   ),
    vec3( -0.761557, 	 0.562508 ,	 0.321895   ),
    vec3( 0.344460 ,	 0.753223 ,	 -0.560359    ),
};

uint rayCount = 32;
uint stepCount = 4;
float stepSize = 1.0;
uint stepLargeCount = 0;
float bigStepSize = 10.0f;

float calcAmbientOcclusion(vec3 position)
{
	float visibility = 0;
	for (uint ray = 0; ray < rayCount; ray++)
	{
	  vec3 dir = ray_dir[ray];   // From constant buffer
	  float this_ray_visibility = 1;
	  // Short-range samples from density volume:
	  for (uint step = 1; step <= stepCount; step++)   // Don't start at zero
	  {
		float d = densityFunction(position + dir * step * stepSize);
		this_ray_visibility *= clamp(d * 0.03, 0, 1);
	  }
	  // Long-range samples from density function:
	  for (uint step = 1; step <= stepLargeCount; step++)   // Don't start at zero
	  {
		float d = densityFunction(position + dir * step * bigStepSize);
		this_ray_visibility *= clamp(d * 0.03, 0, 1);
	  }
	  visibility += this_ray_visibility;
	}
	return ((visibility / float(rayCount))); // Returns occlusion
}

void main()
{
	uint x = b_z8_y8_x8_null4_edge4[gl_GlobalInvocationID.x].x;
	uint y = b_z8_y8_x8_null4_edge4[gl_GlobalInvocationID.x].y;
	uint z = b_z8_y8_x8_null4_edge4[gl_GlobalInvocationID.x].z;
	uint edgeIndex = b_z8_y8_x8_null4_edge4[gl_GlobalInvocationID.x].w;
	
	uint part = 0;
	if (edgeIndex == 8)
		part = 1;
	if (edgeIndex == 3)
		part = 2;
	b_vertex_indices[indexFromCoord(x, y, z) * 3 + part] = gl_GlobalInvocationID.x;

    // 8 corners of the current cube
    vec4 cubeCorners[8] = {
        b_points[indexFromCoordDensity(uvec3(x, y, z))].point,
        b_points[indexFromCoordDensity(uvec3(x + 1, y, z))].point,
        b_points[indexFromCoordDensity(uvec3(x + 1, y, z + 1))].point,
        b_points[indexFromCoordDensity(uvec3(x, y, z + 1))].point,
        b_points[indexFromCoordDensity(uvec3(x, y + 1, z))].point,
        b_points[indexFromCoordDensity(uvec3(x + 1, y + 1, z))].point,
        b_points[indexFromCoordDensity(uvec3(x + 1, y + 1, z + 1))].point,
        b_points[indexFromCoordDensity(uvec3(x, y + 1, z + 1))].point
    };

	// Get indices of corner points A and B for each of the three edges
    // of the cube that need to be joined to form the triangle.
    int end0 = cornerIndexAFromEdge[edgeIndex];
    int end1 = cornerIndexBFromEdge[edgeIndex];

	b_vertices[gl_GlobalInvocationID.x].first.xyz = interpolateVerts(cubeCorners[end0], cubeCorners[end1]);
	b_vertices[gl_GlobalInvocationID.x].first.w = b_points[indexFromCoordDensity(uvec3(x, y, z))].biome.x;
	b_vertices[gl_GlobalInvocationID.x].second.x = calcAmbientOcclusion(b_vertices[gl_GlobalInvocationID.x].first.xyz);
	b_vertices[gl_GlobalInvocationID.x].second.yzw = getVertexNormal(b_vertices[gl_GlobalInvocationID.x].first.xyz);
}

