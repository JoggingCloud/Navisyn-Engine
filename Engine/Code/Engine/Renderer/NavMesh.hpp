#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"

#include <vector>
#include <utility>
#include <unordered_set>

struct Vertex_PCU;
class Prop;

constexpr int MAX_BVH_DEPTH = 20; // Increase for more sub divisions (resulting in smaller boxes and better precision but more memory and traversal cost)
constexpr int MAX_TRIANGLES_PER_LEAF = 256; // Increase for more triangles per box (meaning less boxes), but slower search potentially
constexpr float NAVMESH_ZBIAS = 0.1f;

struct BVHNode
{
	Vec3 m_mins = Vec3::ZERO;
	Vec3 m_maxs = Vec3::ZERO;
	std::vector<int> m_triangleIndexes; // Store the indices of triangles
	std::vector<BVHNode*> m_childBoxes; // Child boxes of parent box

	void StretchToIncludePoint(Vec3 const& point)
	{
		if (point.x < m_mins.x) m_mins.x = point.x;
		if (point.x > m_maxs.x) m_maxs.x = point.x;
		
		if (point.y < m_mins.y) m_mins.y = point.y;
		if (point.y > m_maxs.y) m_maxs.y = point.y;
		
		if (point.z < m_mins.z) m_mins.z = point.z;
		if (point.z > m_maxs.z) m_maxs.z = point.z;
	}

	void StretchToIncludeBox(BVHNode const& other)
	{
		StretchToIncludePoint(other.m_mins);
		StretchToIncludePoint(other.m_maxs);
	}

	inline Vec3 GetCenter() const { return (m_mins + m_maxs) * 0.5f; }
	inline Vec3 GetDimensions() const { return m_maxs - m_mins; }

	bool IsPointInside(Vec3 const& point) const
	{
		return (point.x >= m_mins.x && point.x <= m_maxs.x &&
				point.y >= m_mins.y && point.y <= m_maxs.y &&
				point.z >= m_mins.z && point.z <= m_maxs.z);
	}

	bool DoesBoxOverlapBox(Vec3 const& boxMins, Vec3 const& boxMaxs) const
	{
		return !(m_maxs.x < boxMins.x || m_mins.x > boxMaxs.x ||
				 m_maxs.y < boxMins.y || m_mins.y > boxMaxs.y ||
			     m_maxs.z < boxMins.z || m_mins.z > boxMaxs.z);
	}

	bool DoBVHsOverlap(BVHNode const& nodeA, BVHNode const& nodeB)
	{
		bool overlapX = (nodeA.m_maxs.x > nodeB.m_mins.x) && (nodeA.m_mins.x < nodeB.m_maxs.x);
		bool overlapY = (nodeA.m_maxs.y > nodeB.m_mins.y) && (nodeA.m_mins.y < nodeB.m_maxs.y);
		bool overlapZ = (nodeA.m_maxs.z > nodeB.m_mins.z) && (nodeA.m_mins.z < nodeB.m_maxs.z);

		return overlapX && overlapY && overlapZ;
	}

	inline bool IsLeafNode() const { return m_childBoxes.empty(); }

	BVHNode() = default;
	~BVHNode()
	{
		for (BVHNode* child : m_childBoxes)
		{
			delete child;
		}
	}
};

struct NavMeshTri
{
	int m_vertIndexes[3];
	int m_neighborTriIndexes[3];
};

struct NavMesh
{
	NavMesh() = default;
	~NavMesh();
	
	void CreateBuffers();
	void CreateNavMesh(std::vector<Vec3>& vertexPoints, int mapWidth, int mapHeight, std::vector<int>& vertexMapping);
	void ComputeNeighborsHashed();
	void ComputeNeighborsBruteForce();

	void RemoveRandomTriangleCluster();
	void RemoveTrianglesAffectedByProps(std::vector<Prop*>& props);
	void PopulateDistanceField(NavMeshHeatMap& outDistanceField, float maxCost) const;

	Vec3 ClampPositionToNavMesh(Vec3 const& position);
	Vec3 CalculateCentroid(const NavMeshTri& triangle) const;
	Vec3 GetRandomPointInsideTriangle(int triangleID) const;
	Vec3 GetTriangleCentroid(int triangleID);

	float GetComponent(Vec3 const& vertex, int axis);
	inline int GetNumTriangles() const { return static_cast<int>(m_triangles.size()); };
	int GetClosestTriangleIndex(Vec3 const& position);
	int GetRandomNavMeshTriangleIndex() const;
	int GetTriangleIndex(const NavMeshTri* triangle) const;
	int GetContainingTriangleIndex(Vec3 const& point) const;
	int GetRecursiveTriangleIndex(BVHNode const& node, Vec3 const& point) const;
	NavMeshTri* GetNavMeshTriangle(int triangleID);

	bool FindNearestTriangle(Vec3 const& point, int& outTriangleIndex, Vec3& outProjectedPoint);
	bool ValidateNavMesh() const;

	void RebuildNavMeshVerts();
	void RenderHeatMap() const;
	void RenderBVH() const;
	void RenderNavMesh() const;

	void BuildBVH();
	void BuildBVHRecursive(BVHNode& node, std::vector<int> triangleIndices, int depth = 0);
	void CollectBVHNodesInOrder(const BVHNode* node);
	void DrawAllBVH(BVHNode* node, int depth);

	void ConstructHeatmap();

	void DebugDrawNeighbors() const;

	NavMeshHeatMap* m_heatMap = nullptr;
	BVHNode* m_bvhRoot = nullptr;

	VertexBuffer* m_solidVertexBuffer = nullptr;
	VertexBuffer* m_wireVertexBuffer = nullptr;
	VertexBuffer* m_heatmapVertexBuffer = nullptr;
	VertexBuffer* m_bvhVertexBuffer = nullptr;
	IndexBuffer* m_bvhIndexBuffer = nullptr;

	std::vector<Vec3> m_vertexes;
	std::vector<NavMeshTri> m_triangles; // List of polygons (Triangles) in the NavMesh

	std::vector<Vertex_PCU> m_solidVerts;
	std::vector<Vertex_PCU> m_wireVerts;
	std::vector<Vertex_PCU> m_heatmapVertexes;
	std::vector<Vertex_PCU> m_bvhVertexes;
	std::vector<unsigned int> m_bvhIndexes;

	int m_heatMapIndex = 0;
	float m_specialValue = -1.f;
	float m_maxDistanceFieldCost = 9999.f;
	int m_numTriangleClusters = 15;
	IntRange m_numNeighborsToRemove = IntRange(3, 9);

	// Debug BVH
	int m_currentBVHBoxIndex = 0;
	std::vector<const BVHNode*> m_debugBVHBoxes;
};
