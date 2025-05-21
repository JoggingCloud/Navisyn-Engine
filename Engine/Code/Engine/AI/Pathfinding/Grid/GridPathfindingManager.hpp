#pragma once
#include "Engine/AI/Pathfinding/Grid/GridCommon.hpp"

enum class GridPathType
{
	Dijkstra,
	AStar,
	DStarLite,
	BFS,
	FlowField,
	DistanceField
};

class IGridPathfinder
{
public:
	virtual void ComputePath(IntVec2 start, IntVec2 goal, std::vector<IntVec2>& outPath) = 0;
	virtual void SetHeuristic(std::function<int(IntVec2, IntVec2)> heuristic) {}
	virtual ~IGridPathfinder() = default;
};

class GridPathfindingManager
{
public:
	GridPathfindingManager() = default;
	~GridPathfindingManager() = default;

	void SetPathType(GridPathType type);
	void SetHeuristic(std::function<int(IntVec2, IntVec2)> heuristic);

	//void ComputePath(IntVec2 start, IntVec2 goal, std::vector<IntVec2>& outPath);

private:
	GridPathType m_currentPathType = GridPathType::AStar;
	IGridPathfinder* m_current = nullptr;
	
	std::unordered_map<GridPathType, IGridPathfinder*> m_pathAlgorithms;
	std::function<int(IntVec2, IntVec2)> m_heuristic = nullptr;
};
