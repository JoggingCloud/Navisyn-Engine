#pragma once
#include "Engine/AI/Pathfinding/Grid/GridCommon.hpp"
#include <vector>
#include <queue>

constexpr int MAX_CYCLES = 20;

struct Node
{
	IntVec2 m_position;
	IntVec2 m_parent;
	float m_totalgCost;
	float m_rhs; // right-hand side. This represents the estimated cost of the shortest path from a node 's' to the goal
	bool m_isOpen;
	bool m_isClosed;

	Node() : m_position(-1, -1), m_parent(-1, -1), m_totalgCost(std::numeric_limits<float>::max()), m_rhs(std::numeric_limits<float>::max()), m_isOpen(false), m_isClosed(false) {}
	Node(IntVec2 position, float gCost, float rhs, IntVec2 parent, bool isOpen, bool isClosed)
		:m_position(position), m_totalgCost(gCost), m_rhs(rhs), m_parent(parent), m_isOpen(isOpen), m_isClosed(isClosed) {}
};

using Key = std::pair<float, float>;
struct CompareNode
{
	bool operator()(const std::pair<Key, Node*>& entryA, const std::pair<Key, Node*>& entryB)
	{
		const Key& keyA = entryA.first;
		const Key& keyB = entryB.first;
		if (keyA.first != keyB.first) { return keyA.first > keyB.first; }
		return keyA.second > keyB.second;
	}
};

class GridDStarLite
{
public:
	GridDStarLite() = default;
	GridDStarLite(IntVec2 grid);
	virtual ~GridDStarLite() = default;

public:
	void InitializeNodeGrid(IntVec2 const& grid);
	void SetDirectionMode(DirectionMode mode) { m_directionMode = mode; }

	// A-Star
	void ComputeDStarLite(IntVec2 startPoint, IntVec2 goalPoint, std::vector<IntVec2>& outPath);
	void ComputeShortestPath(IntVec2 start, IntVec2 goal);
	void UpdatePredecessors(Node* node);
	void UpdateVertex(Node* node);
	void CalculateKey(Key& key, Node* node);
	void RecalculateNode(IntVec2 newStart);

	void ForEachNeighbor(Node* node, std::function<void(Node*, IntVec2)> callback);
	int GetCost(IntVec2 currentPosition, IntVec2 neighborPosition);
	Node* GetNode(IntVec2 point);

	// Debug A-Star
	void TestDStarLitePathFinding(IntVec2 start, IntVec2 goal, std::vector<IntVec2>& path);

private:
	int m_km = 0; // global value use to adjust the heuristic key whenever the start moves or the environment changes
	IntVec2 m_start; // Store because the start and goal can constantly be changed
	IntVec2 m_goal;
	std::priority_queue<std::pair<Key, Node*>, std::vector<std::pair<Key, Node*>>, CompareNode> m_openList;

public:
	int m_gridWidth = 0;
	int m_gridHeight = 0;

	DirectionMode m_directionMode = DirectionMode::Cardinal4;

	std::vector<IntVec2> m_currentStoredPathfindingPath;
	std::vector<Node> m_nodeGrid;

public:
	using IsSolidCallbackFunc = std::function<bool(IntVec2)>;
	IsSolidCallbackFunc m_isSolidCallback = nullptr;

	using CanMoveDiagonalCallbackFunc = std::function<bool(IntVec2 fromPos, IntVec2 toPos)>;
	CanMoveDiagonalCallbackFunc m_canMoveDiagonalCallback = nullptr;

	void SetIsSolidCallback(IsSolidCallbackFunc callbackFunc) { m_isSolidCallback = callbackFunc; }
	void SetCanMoveDiagonalCallback(CanMoveDiagonalCallbackFunc callbackFunc) { m_canMoveDiagonalCallback = callbackFunc; }

	inline bool IsDiagonal(IntVec2 step) { return step.x != 0 && step.y != 0; }
};