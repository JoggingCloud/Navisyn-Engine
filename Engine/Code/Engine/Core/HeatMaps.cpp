#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/NavMesh.hpp"

TileHeatMap::TileHeatMap(const IntVec2& dimensions)
	: m_dimensions(dimensions)
{
	int numTiles = m_dimensions.x * m_dimensions.y;
	m_values.resize(numTiles, 0.f);
}

float TileHeatMap::GetHighestHeat() const
{
	float highestHeat = -1.f; // Initial Value. heat values are never negative

	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			IntVec2 tileCoords(x, y);
			float currentHeat = GetValue(tileCoords);

			if (currentHeat > highestHeat)
			{
				highestHeat = currentHeat;
			}
		}
	}

	return highestHeat;
}

void TileHeatMap::SetAllValues(float value)
{
	for (int index = 0; index < m_values.size(); index++)
	{
		m_values[index] = value;
	}
}

float TileHeatMap::GetValue(const IntVec2& tileCoords) const
{
	int index = tileCoords.y * m_dimensions.x + tileCoords.x;

	return m_values[index];
}

void TileHeatMap::SetValue(const IntVec2& tileCoords, float value)
{
	int index = tileCoords.y * m_dimensions.x + tileCoords.x;

	m_values[index] = value;
}

void TileHeatMap::AddValue(const IntVec2& tileCoords, float value)
{
	int index = tileCoords.x + m_dimensions.x * tileCoords.y;

	m_values[index] += value;
}

void TileHeatMap::AddVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 bounds, FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor)
{
	float minValue = valueRange.m_min;
	float maxValue = valueRange.m_max;

	// Loop through the dimensions of the map
	for (int y = 0; y < m_dimensions.y; ++y)
	{
		for (int x = 0; x < m_dimensions.x; ++x)
		{
			IntVec2 tileCoords(x, y);
			float value = GetValue(tileCoords);

			// Normalize the value to range between [0, 1]
			float normalizedValue = RangeMapClamped(value, minValue, maxValue, 0.0f, 1.0f);
			// Interpolate the color based on the normalized value
			Rgba8 color = Interpolate(lowColor, highColor, normalizedValue);

			if (value == specialValue)
			{
				color = specialColor;
			}
			
			// Need to get the bounds of the tile
			Vec2 mins = Vec2((x * bounds.GetDimensions().x / m_dimensions.x), (y * bounds.GetDimensions().y / m_dimensions.y));
			Vec2 maxs = Vec2(((x + 1) * bounds.GetDimensions().x / m_dimensions.x), ((y + 1) * bounds.GetDimensions().y / m_dimensions.y));

			AABB2 tileBounds(mins, maxs);

			// Add verts for the tile?!?!
			AddVertsForAABB2D(verts, tileBounds, color, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f));
		}
	}
}

// --------------------------------------------------------------------------------------------------------

NavMeshHeatMap::NavMeshHeatMap(NavMesh* navMesh)
	: m_navMesh(navMesh)
{
	for (int triangleID = 0; triangleID < static_cast<int>(m_navMesh->m_triangles.size()); triangleID++)
	{
		m_values[triangleID] = 0.f;
	}
}

float NavMeshHeatMap::GetHighestHeat() const
{
	float highestHeat = 0.f; // Initial Value. heat values are never negative

	for (const auto& pair : m_values)
	{
		if (pair.second > highestHeat)
		{
			highestHeat = pair.second;
		}
	}
	return highestHeat;
}

float NavMeshHeatMap::GetValue(int triangleID)
{
	auto it = m_values.find(triangleID);
	if (it != m_values.end())
	{
		return it->second;
	}
	return 0.f;
}

void NavMeshHeatMap::SetAllValues(float value)
{
	for (auto& pair : m_values)
	{
		pair.second = value;
	}
}

void NavMeshHeatMap::SetValue(int triangleID, float value)
{
	m_values[triangleID] = value;
}

void NavMeshHeatMap::AddValue(int triangleID, float value)
{
	m_values[triangleID] += value;
}

void NavMeshHeatMap::AddVertsForDebugDraw(std::vector<Vertex_PCU>& verts, FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor)
{
	float minValue = valueRange.m_min;
	float maxValue = valueRange.m_max;
	const std::vector<Vec3>& vertexPoints = m_navMesh->m_vertexes;

	for (int i = 0; i < static_cast<int>(m_navMesh->m_triangles.size()); i++)
	{
		float valueAtTriangleIndex = GetValue(i);
		float normalizedValue = RangeMapClamped(valueAtTriangleIndex, minValue, maxValue, 0.f, 1.f);

		Rgba8 color = Interpolate(lowColor, highColor, normalizedValue);

		if (valueAtTriangleIndex == specialValue) { color = specialColor; }

		const NavMeshTri& triangle = m_navMesh->m_triangles[i];
		AddVertsFor3DTriangle(verts, vertexPoints[triangle.m_vertIndexes[0]], vertexPoints[triangle.m_vertIndexes[1]], vertexPoints[triangle.m_vertIndexes[2]], color);
	}
}
