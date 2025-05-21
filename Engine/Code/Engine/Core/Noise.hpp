#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"

typedef std::vector<std::vector<float>> NoiseMap;

struct Noise
{
	Noise() = default;
	~Noise() = default;

	static NoiseMap GenerateNoiseMap(int mapWidth, int mapHeight, int seed, float scale, int octaves, float persistence, float lacunarity, Vec2 offset);
};

// ----------------------------------------------------------------------------------------
// Scale affects the size of the features in the noise pattern. A smaller scale value will make the terrain features larger (e.g., big hills and valleys), while a larger scale value will produce smaller, finer details.
// Octaves is the amount of layers of perlin noise to create more complex and detailed noise
// Persistence controls the influence of each successive octave.
// If persistence is high (close to 1), the higher octaves contribute more. If persistence is low, the higher octaves have less influence.
// Lacunarity controls the increase in frequency of the noise at each octave. It determines how much detail is added at each subsequent octave.
// Increasing the lacunarity makes the terrain more "rough" and detailed by adding more variations at smaller scales.