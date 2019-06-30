#version 430 compatibility
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#include "noise.glsl"

struct DensityPoint
{
	vec4 point;
	vec4 biome;
};

layout (packed, binding = 0) buffer PointBuffer
{
	DensityPoint points[];
};

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

uniform uvec3 u_resolution;
uniform vec3 u_size;
uniform vec3 u_offset;
uniform float u_floorPosition;

uint indexFromCoord(uint x, uint y, uint z)
{
    return (((z * (u_resolution.y + 1)) + y) * (u_resolution.x + 1)) + x;
}


void main()
{
	uvec3 id = gl_GlobalInvocationID;
	if (id.x > u_resolution.x || id.y > u_resolution.y || id.z > u_resolution.z)
		return;

	/*
	vec4 point;
	point.x = id.x;
	point.y = id.y;
	point.z = id.z;
	point.w = id.z - (float(u_resolution.z) * 0.5);
	vec3 n = vec3(float(id.x), float(id.y), float(id.z)) / 16.0;
	float feature = float(u_resolution.z) * 0.2;
	//point.w += cnoise(n) * feature;
	points[indexFromCoord(id.x, id.y, id.z)] = point;
	*/

	
	float hardFloor = 0.0;
	float hardFloorWeight = 10.0;
	float noiseScale = 1.0 / 50.0;
	int octaves = 3;
	vec3 offset = vec3(0.0);
	vec3 offsets[4];
	offsets[0] = vec3(0.0);
	offsets[1] = vec3(0.1, 1.4, -0.7);
	offsets[2] = vec3(2.11, -0.224, 0.0);
	offsets[3] = vec3(0.0);
	float noiseWeight = 1.0;
	float lacunarity = 2.0;
	float persistence = 0.5;
	float floorOffset = 2.0;
	float weightMultiplier = 1.0;
    vec3 offsetNoise = vec3(0.0);
	
	vec3 pos = u_offset + vec3(float(id.x), float(id.y), float(id.z)) * (u_size / vec3(u_resolution));
	//vec3 pos = u_offset + vec3(id) * (u_size / vec3(u_resolution));

    float noise = 0;

    float frequency = noiseScale;
    float amplitude = 70.0;
    float weight = 1;

	float temperature = (snoise(pos * vec3(1,1,0) * frequency) + 1) * 0.5;
	float moisture = (cnoise(pos * vec3(1,1,0) * frequency) + 1) * 0.5 * temperature;


	int JUNGLE = 0;
	int DESERT = 1;
	int GRASS = 2;
	int TAIGA = 3;
	int TUNDRA = 4;
	int SEVANNA = 5;


	float biome = GRASS;
	if (temperature > 0.7)
	{
		if (moisture > 0.7)
			biome = JUNGLE;
		else
			biome = SEVANNA;
	}
	if (moisture < 0.2)
		biome = DESERT;
	if (temperature < 0.4)
		biome = TAIGA;
	if (temperature < 0.2)
		biome = TUNDRA;


    for (int j = 0; j < octaves; j++) {
		vec3 samplePos = pos;
		samplePos.x += snoise(pos * frequency) * 2;
		samplePos.y += snoise(pos * frequency + frequency) * 2;
        float n = snoise((samplePos + offsetNoise) * frequency + offsets[j] + offset);
        float v = abs(n);
        v = v * v;
        //v *= weight;
        //weight = max(min(v * weightMultiplier, 1), 0);
        noise += v * amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
	
    
	vec2 params = vec2(6.0, 0.7);
	params = vec2(0);

    float finalVal = (u_floorPosition - pos.z);
	finalVal = pow(abs(finalVal), 1.4) * sign(finalVal);
	//float centerFlatten = min(1, length(pos) / 30);
	finalVal += noise * noiseWeight;// * centerFlatten;
	finalVal += mod(pos.z, params.x) * params.y;

    if (pos.z < hardFloor) {
        //finalVal += hardFloorWeight;
    }
	
	//pos = vec3(float(id.x), float(id.y), float(id.z));
	//finalVal *= 0.00001;
	//finalVal = 8 - pos.z;
	//finalVal += snoise(pos);
	uint index = indexFromCoord(id.x, id.y, id.z);
	points[index].point = vec4(pos, finalVal);
	points[index].biome = vec4(biome, 1, 2, 3);
}

