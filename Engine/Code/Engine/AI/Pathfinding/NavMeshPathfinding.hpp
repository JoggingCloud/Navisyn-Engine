#pragma once
#include "Engine/Renderer/NavMesh.hpp"
#include "Engine/Math/LineSegment3.hpp"
#include <list>
#include <utility>
#include <vector>
#include <queue>
#include <limits>
#include <tuple>
#include <algorithm>

constexpr float ALLOWED_HEIGHT_DEVIATION = 0.125f;

struct RaycastVsGroundResult
{
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;
};

struct Node
{
	Vec3 m_position;
	int m_triangleIndex; // Index of the point this node represents in the world 
	float m_totalgCost;
	float m_fCost;
	int m_parentTriangleIndex;
	int m_openPathGen = -1; // Better for medium to large maps
	int m_closedPathGen = -1; // Better for medium to large maps
// 	bool m_isOpen; // Better for small maps
// 	bool m_isClosed; // Better for small maps

	Node() : m_position(std::numeric_limits<Vec3>::max()), m_triangleIndex(std::numeric_limits<int>::max()), m_totalgCost(0), m_fCost(0), m_parentTriangleIndex(std::numeric_limits<int>::max()), m_openPathGen(-1), m_closedPathGen(-1) {}
	Node(Vec3 position, Vec3 parentPosition, int pointIndex, float gCost, float fCost, int parentIndex, int nodeOpen, int nodeClosed)
		:m_position(position), m_triangleIndex(pointIndex), m_totalgCost(gCost), m_fCost(fCost), m_parentTriangleIndex(parentIndex), m_openPathGen(nodeOpen), m_closedPathGen(nodeClosed) {}
};

struct CompareNode
{
	bool operator()(Node* left, Node* right) const { return left->m_fCost > right->m_fCost; };
};

class NavMeshPathfinding
{
public:
	NavMeshPathfinding() = default;
	NavMeshPathfinding(NavMesh* navMesh);
	virtual ~NavMeshPathfinding() = default;

public:
	// Initializing Nodes
	void InitializeNavMeshNodes(NavMesh const& navMesh);

	// A-Star
	void ComputeAStar(Vec3 startPoint, Vec3 goalPoint, std::vector<Vec3>& outPath);
	void Funnel(Vec3 startPoint, std::vector<Vec3>& constructedPath);
	void Prune(std::vector<Vec3>& prunedPath);
	void ResamplePathToFollowNavMesh(std::vector<Vec3>& path);

	// Utility functions
	void GroundCheck(Vec3 startPosition, float endZHeight);
	bool HasLineOfSight(const Vec3& startPoint, const Vec3& endPoint) const;
	float GetAverageTriangleEdgeLength() const;
	float GetHeightOnTriangle(int triangleID, float x, float y);
	float GetHeightOnTriangle(const NavMeshTri& triangle, float x, float y);
	float GetHeightOnTriangle(Vec3 point);
	float ComputeBarycentricHeight(float x, float y, Vec3 v0, Vec3 v1, Vec3 v2);
	int GetTriangleIndexFromPoint(Vec3 point) const;
	inline float GetDistanceSquaredBetweenPoints(const Vec3& pointA, const Vec3& pointB) const { return (pointB - pointA).GetLengthSquared(); };
	inline float GetDistanceBetweenPointsExact(const Vec3& pointA, const Vec3& pointB) const { return (pointB - pointA).GetLength(); };

	Vec3 GetEdgeIntersectionPoint(int neighborTriangleID, Vec3 currentPoint, Vec3 goalPoint);
	Vec3 CalculateCentroid(NavMeshTri& triangleMesh);
	Vec3 CalculateCentroid(int triangleID);
	Vec3 GetRandomPointWithinTriangleIndex(int triangleID);
	Vec3 GetImpactPointOnTriangle(Vec3& point, int triangleID);

	RaycastVsGroundResult RaycastVsNavNesh(const Vec3& start, const Vec3& direction, float maxDistance);

	// Debug A-star
	void TestAStarPathFinding(Vec3 startPoint, Vec3 goalPoint, std::vector<Vec3>& path);

public:
	NavMesh* m_navMesh = nullptr;
	std::vector<Node> m_nodes;
	int m_pathGen = 0;
};