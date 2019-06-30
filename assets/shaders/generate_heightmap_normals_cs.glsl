#version 430 compatibility

#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

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

/*
Point get_position(uint x, uint y, uint z)
{
	float frequency =  1.0 / 200.0;

	vec3 pos = vec3(float(x), float(y), float(z)) * (u_size / vec3(u_resolution));
	pos += u_offset;
	pos.z = u_floorPosition + octave_noise(pos, 6, frequency, 80.0);


	float temperature = 1 - max(0, min(1, (snoise(pos) + 1) * 0.5));
	temperature = -pos.z / 30.0;
	temperature = (temperature + 1) * 0.5;
	temperature = max(0, min(1, temperature));

	//float temperature = (octave_noise(pos * vec3(-1,-1,0), 3, frequency, 1.0) + 1) * 0.5;
	float moisture = (octave_noise(pos * vec3(-1,-1,0), 3, frequency, 1.0) + 1) * 0.5;
	moisture = max(0, min(1, moisture)) * temperature;

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
	biome = 0;

	Point point;
	point.position.xyz = pos;
	point.biome.x = temperature;
	point.biome.y = moisture;

	return point;
}
*/

uint get_index(int x, int y)
{
	return ((y + 1) * (int(u_resolution.x) + 3)) + (x + 1);
}

vec3 get_normal(vec3 a, vec3 b, vec3 c)
{
	return normalize(cross(a - b, b - c));
}

void main()
{
	uvec3 id = gl_GlobalInvocationID;
	if (id.x >= u_resolution.x + 1 || id.y >= u_resolution.y + 1)
		return;

	vec3 normalSum = vec3(0.0);
	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			vec3 p0 = vertices[get_index(int(id.x) + x - 1, int(id.y) + y - 1)].first.xyz;
			vec3 p1 = vertices[get_index(int(id.x) + x, int(id.y) + y - 1)].first.xyz;
			vec3 p2 = vertices[get_index(int(id.x) + x - 1, int(id.y) + y)].first.xyz;
			vec3 p3 = vertices[get_index(int(id.x) + x, int(id.y) + y)].first.xyz;
			normalSum += get_normal(p0, p1, p3);
			normalSum += get_normal(p0, p3, p2);
		}
	}

	vec3 normal = normalize(normalSum / 8.0);
	//normal = normalize(vec3(float(id.x) / float(u_resolution.x), float(id.y) / float(u_resolution.y), 1.0));
	vertices[get_index(int(id.x), int(id.y))].second.yzw = normal;
}

