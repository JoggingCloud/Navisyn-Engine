#include "Engine/Renderer/Terrain.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <chrono>

Terrain::Terrain(int width, int height, int numRegions, float tilingFactor, const std::vector<std::string>& regionNames, const std::vector<Rgba8>& regionColors, const std::vector<float>& regionHeights)
{
	for (int i = 0; i < numRegions; i++)
	{
		CreateRegions(regionNames[i], regionColors[i], regionHeights[i]);
	}

	NoiseMap noiseMap = Noise::GenerateNoiseMap(width, height, 4, 20.f, 15, 0.2f, 3.f, Vec2(0.f, 0.f)); // 20x20 map size 

	m_terrainWidth = static_cast<float>(noiseMap[0].size());
	m_terrainHeight = static_cast<float>(noiseMap.size());

	m_terrainHeightMap.resize(width * height);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			m_terrainHeightMap[Get1DIndex(i, j, width)] = noiseMap[i][j];
		}
	}

	GenerateTerrain(noiseMap, tilingFactor);
}

Terrain::~Terrain()
{
	m_indices.clear();
	m_vertices.clear();
}

void Terrain::CreateRegions(std::string const& name, Rgba8 const& color, float height)
{			
	m_regions.emplace_back(name, color, height);
}

void Terrain::AddTriangle(int a, int b, int c)
{
	m_indices[m_triangleIndex] = a;
	m_indices[m_triangleIndex + 1] = b;
	m_indices[m_triangleIndex + 2] = c;
	m_triangleIndex += 3;
}

void Terrain::GenerateTerrain(NoiseMap const& heightMap, const float& tilingFactor)
{
	int mapWidth = (int)heightMap[0].size();
	int mapHeight = (int)heightMap.size();
	int totalVertices = mapWidth * mapHeight;
	m_vertices.resize(totalVertices);
	m_indices.resize((mapWidth - 1) * (mapHeight - 1) * 6);

	float xOffset = static_cast<float>(mapWidth) / 2.f;
	float yOffset = static_cast<float>(mapHeight) / 2.f;

	Vec2 uvCoord = Vec2::ZERO;
	int vertexIndex = 0;

	for (int y = 0; y < mapHeight; y++)
	{
		for (int x = 0; x < mapWidth; x++)
		{
			// Adjust numeric values based on how big i want the map to be (Smaller for small map)
			Vec3 vertex = Vec3((float)x - xOffset, (float)y - yOffset, (m_terrainHeightMap[Get1DIndex(x, y, mapWidth)] * TERRAIN_HEIGHT) - TERRAIN_HEIGHT * 0.5f); 

			uvCoord.x = RangeMap((float)x, 0.f, (float)mapWidth, 0.f, tilingFactor);
			uvCoord.y = RangeMap((float)y, 0.f, (float)mapHeight, 0.f, tilingFactor);

			Rgba8 vertexColor = Rgba8::WHITE;
			for (int i = 0; i < m_regions.size(); i++)
			{
				if (m_regions[i].m_height >= m_terrainHeightMap[Get1DIndex(x, y, mapWidth)])
				{
					if (i == 0)
					{
						// Lowest region, assign its exact color since there's nothing to interpolate with
						vertexColor = m_regions[i].m_color;
					}
					else
					{
						// Interpolate color between the current region and the previous one
						float t = (m_terrainHeightMap[Get1DIndex(x, y, mapWidth)] - m_regions[i - 1].m_height) / (m_regions[i].m_height - m_regions[i - 1].m_height);
						vertexColor = Interpolate(m_regions[i - 1].m_color, m_regions[i].m_color, t);
					}
					break;
				}
			}

			m_vertices[vertexIndex] = Vertex_PCUTBN(vertex, vertexColor, uvCoord, Vec3::ZERO, Vec3::ZERO, Vec3::ZERO);

			if (x < mapWidth - 1 && y < mapHeight - 1)
			{
				AddTriangle(vertexIndex, vertexIndex + 1, vertexIndex + mapWidth + 1);
				AddTriangle(vertexIndex, vertexIndex + mapWidth + 1, vertexIndex + mapWidth);
			}

			vertexIndex++;
		}
	}

	CalculateTangentSpaceBasisVectors(m_vertices, m_indices);
}

float Terrain::GetTerrainWidth() const
{
	return m_terrainWidth;
}

float Terrain::GetTerrainHeight() const
{
	return m_terrainHeight;
}

TerrainType::TerrainType(std::string name, Rgba8 color, float height)
	: m_name(name), m_color(color), m_height(height)
{
}
