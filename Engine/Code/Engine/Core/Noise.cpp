#include "Noise.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include <chrono>

NoiseMap Noise::GenerateNoiseMap(int mapWidth, int mapHeight, int seed, float scale, int octaves, float persistence, float lacunarity, Vec2 offset)
{
	if (mapWidth < 1) mapWidth = 1;
	if (mapHeight < 1) mapHeight = 1;

	// Initialize noise map array
	NoiseMap noiseMap;
	noiseMap.resize(mapHeight);
	for (int height = 0; height < mapHeight; height++) 
	{
		noiseMap[height].resize(mapWidth);
	}

	RandomNumberGenerator rng;
	//rng.SetSeed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	Vec2* octaveOffsets = new Vec2[octaves];
	for (int i = 0; i < octaves; i++) 
	{
		float offsetX = rng.SRollRandomFloatInRange(-1000 * (float)seed, 1000 * (float)seed) + offset.x;
		float offsetY = rng.SRollRandomFloatInRange(-1000 * (float)seed, 1000 * (float)seed) + offset.y;
		octaveOffsets[i] = Vec2(offsetX, offsetY);
	}

	if (scale <= 0.f) scale = 0.0001f;

	float maxNoiseHeight = FLT_MIN;
	float minNoiseHeight = FLT_MAX;

	float halfWidth = mapWidth * 0.5f;
	float halfHeight = mapHeight * 0.5f;

	float factor = 1.f / scale;

	// Generate the noise map values with radial distance consideration
	for (int y = 0; y < mapHeight; y++) 
	{
		for (int x = 0; x < mapWidth; x++)
		{
			float amplitude = 1.f;
			float frequency = 1.f;
			float noiseHeight = 0.f;

			for (int i = 0; i < octaves; i++) 
			{
				float sampleX = (x - halfWidth) * factor * frequency + octaveOffsets[i].x;
				float sampleY = (y - halfHeight) * factor * frequency + octaveOffsets[i].y;

				float perlinValue = Compute2dPerlinNoise(sampleX, sampleY) * 2.f - 1.f;
				noiseHeight += perlinValue * amplitude;

				amplitude *= persistence;
				frequency *= lacunarity;
			}

			if (noiseHeight > maxNoiseHeight) maxNoiseHeight = noiseHeight;
			if (noiseHeight < minNoiseHeight) minNoiseHeight = noiseHeight;

			noiseMap[y][x] = noiseHeight;
		}
	}

	for (int y = 0; y < mapHeight; y++)
	{
		for (int x = 0; x < mapWidth; x++) 
		{
			noiseMap[y][x] = RangeMapClamped(noiseMap[y][x], minNoiseHeight, maxNoiseHeight, 0.f, 1.f);
		}
	}

	delete[] octaveOffsets;
	return noiseMap;
}
