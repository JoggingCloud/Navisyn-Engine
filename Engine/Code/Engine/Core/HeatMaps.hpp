#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <map>
#include <vector>

struct NavMesh;

class TileHeatMap 
{
public:
	TileHeatMap(const IntVec2& dimensions);
	TileHeatMap() = default;
	~TileHeatMap() = default;

	float GetHighestHeat() const;
	float GetValue(const IntVec2& tileCoords) const;
	
	void SetAllValues(float value);
	void SetValue(const IntVec2& tileCoords, float value);
	void AddValue(const IntVec2& tileCoords, float value);

	void AddVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 bounds, FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor);

private:
	IntVec2 m_dimensions;
	std::vector<float> m_values;
};

class NavMeshHeatMap
{
public:
	NavMeshHeatMap(NavMesh* navMesh);
	NavMeshHeatMap() = default;
	~NavMeshHeatMap() = default;

	float GetHighestHeat() const;
	float GetValue(int triangleID);

	void SetAllValues(float value);
	void SetValue(int triangleID, float value);
	void AddValue(int triangleID, float value);

	void AddVertsForDebugDraw(std::vector<Vertex_PCU>& verts, FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor);

private:
	NavMesh* m_navMesh = nullptr;
	std::unordered_map<int, float> m_values; // Triangle ID to Distance
};
