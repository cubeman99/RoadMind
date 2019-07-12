
//#include "noise.glsl"

uniform float u_floorPosition;
uniform float hardFloor = 0.0;
uniform float hardFloorWeight = 10.0;
uniform float noiseScale = 1.0 / 200.0;
uniform int octaves = 3;
uniform vec3 offset = vec3(0.0);
uniform float noiseWeight = 1.0;
uniform float lacunarity = 2.0;
uniform float persistence = 0.5;
uniform float floorOffset = 2.0;
uniform float weightMultiplier = 1.0;
uniform vec3 offsetNoise = vec3(0.0);
uniform float u_amplitude = 200.0;

float densityFunction(vec3 pos)
{
	vec3 offsets[4];
	offsets[0] = vec3(0.0);
	offsets[1] = vec3(0.1, 1.4, -0.7);
	offsets[2] = vec3(2.11, -0.224, 0.0);
	offsets[3] = vec3(0.0);

    float noise = 0;

    float frequency = noiseScale;
    float amplitude = u_amplitude;
    float weight = 1.0;

	float density = u_floorPosition - pos.z;

	vec3 samplePos = pos * frequency;
	density += snoise(samplePos * 7.97) * 0.125 * amplitude;
	density += snoise(samplePos * 4.03) * 0.25 * amplitude;
	density += snoise(samplePos * 1.96) * 0.50 * amplitude;
	density += snoise(samplePos * 1.01) * 1.00 * amplitude;
	density += snoise(samplePos * 0.49) * 2.00 * amplitude;

    /*for (int j = 0; j < octaves; j++) {
		vec3 samplePos = pos;
		samplePos.x += snoise(pos * frequency) * 2;
		samplePos.y += snoise(pos * frequency + frequency) * 2;
        float n = snoise((samplePos + offsetNoise) * frequency + offsets[j] + offset);
        float v = abs(n);
        v = v * v * v;
        //weight = max(min(v * weightMultiplier, 1), 0);
        noise += v * amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }*/
	
    
	//vec2 params = vec2(6.0, 0.7);
	//params = vec2(0);

    //float finalVal = (u_floorPosition - pos.z);
	//finalVal = pow(abs(finalVal), 1.4) * sign(finalVal);
	//float centerFlatten = min(1, length(pos) / 30);
	//finalVal += noise * noiseWeight;// * centerFlatten;
	//finalVal += mod(pos.z, params.x) * params.y;

    //if (pos.z < hardFloor) {
        //finalVal += hardFloorWeight;
    //}
	
	//density = (u_floorPosition - pos.z);
	//density *= 0.0000001;
	//density += pos.z + 100;
	return density;
}


float getBiome(vec3 pos)
{
    float frequency = noiseScale;
    float amplitude = 200.0;
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
	
	return biome;
}
