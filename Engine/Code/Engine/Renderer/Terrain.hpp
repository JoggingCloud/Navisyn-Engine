#pragma once
#include <vector>
#include <string>
#include "Engine/Core/Noise.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

constexpr float TERRAIN_HEIGHT = 4.f;

struct TerrainType
{
	std::string m_name;
	Rgba8       m_color;
	float       m_height;

	TerrainType() = default;
	TerrainType(std::string name, Rgba8 color, float height);
	~TerrainType() = default;
};

class Terrain
{
public:
	std::vector<TerrainType>       m_regions;
	std::vector<unsigned int>      m_indices;
	std::vector<Vertex_PCUTBN>     m_vertices;
	Noise						   m_noiseMap;
	int                            m_triangleIndex = 0;
	float						   m_terrainWidth;
	float						   m_terrainHeight;
	std::vector<float>             m_terrainHeightMap;

public:
	Terrain() = default;
	~Terrain();

	Terrain(int width, int height, int numRegions, float tilingFactor, 
	const std::vector<std::string>& regionNames, 
	const std::vector<Rgba8>& regionColors, 
	const std::vector<float>& regionHeights);

	void CreateRegions(std::string const& name, Rgba8 const& color, float height);
	void AddTriangle(int a, int b, int c);
	void GenerateTerrain(NoiseMap const& heightMap, const float& tilingFactor);

	float GetTerrainWidth() const;
	float GetTerrainHeight() const;

	inline size_t Get1DIndex(int x, int y, int width) const { return static_cast<size_t>((y * width) + x); }
};
