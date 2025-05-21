#pragma once
#include "Engine/AI/Pathfinding/Grid/GridCommon.hpp"
#include <vector>
#include <queue>

struct Node
{
	IntVec2 m_position;
	IntVec2 m_parent;
	float m_totalgCost;
	float m_fCost;
	int m_openPathGen = -1; 
	int m_closedPathGen = -1;
	int m_heapIndex = -1;

	Node() : m_position(-1, -1), m_totalgCost(0), m_fCost(0), m_parent(-1, -1), m_openPathGen(-1), m_closedPathGen(-1), m_heapIndex(-1) {}
	Node(IntVec2 position, float gCost, float fCost, IntVec2 parent, int nodeOpen, int nodeClosed)
		:m_position(position), m_totalgCost(gCost), m_fCost(fCost), m_parent(parent), m_openPathGen(nodeOpen), m_closedPathGen(nodeClosed), m_heapIndex(-1) {}
};

struct CompareNode
{
	bool operator()(Node* left, Node* right) const { return left->m_fCost > right->m_fCost; };
};

class GridAStar
{
public:
	GridAStar() = default;
	GridAStar(IntVec2 grid);
	virtual ~GridAStar() = default;

public:
	void InitializeNodeGrid(IntVec2 const& grid);
	void SetDirectionMode(DirectionMode mode) { m_directionMode = mode; }

	// A-Star
	void ComputeAStar(IntVec2 start, IntVec2 goal, std::vector<IntVec2>& outPath);

	// Debug A-Star
	void TestAStarPathFinding(IntVec2 start, IntVec2 goal, std::vector<IntVec2>& path);

public:
	int m_pathGen = 0;
	int m_grid = 0;

	DirectionMode m_directionMode = DirectionMode::Cardinal4;

	std::vector<Node> m_nodeGrid;

public:
	using IsSolidCallbackFunc = std::function<bool(IntVec2)>;
	IsSolidCallbackFunc m_isSolidCallback = nullptr;

	using CanMoveDiagonalCallbackFunc = std::function<bool(IntVec2 fromPos, IntVec2 toPos)>;
	CanMoveDiagonalCallbackFunc m_canMoveDiagonalCallback = nullptr;

	void SetIsSolidCallback(IsSolidCallbackFunc callbackFunc) { m_isSolidCallback = callbackFunc; }
	void SetCanMoveDiagonalCallback(CanMoveDiagonalCallbackFunc callbackFunc) { m_canMoveDiagonalCallback = callbackFunc; }

	inline bool IsDiagonal(IntVec2 step) { return step.x != 0 && step.y != 0; }

	class CustomHeap
	{
	public:
		void Resverve(size_t amount) { m_elements.reserve(amount); }
		void Push(Node* node);
		void DecreaseKey(Node* node);
		void Clear() { m_elements.clear(); }
		Node* Pop();
		bool Empty() const { return m_elements.empty(); }
		size_t Size() const { return m_elements.size(); }

	private:
		std::vector<Node*> m_elements;

		void PercolateUp(size_t index);
		void PercolateDown(size_t index);
	};
};