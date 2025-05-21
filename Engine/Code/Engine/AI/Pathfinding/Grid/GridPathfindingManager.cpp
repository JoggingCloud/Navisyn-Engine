#include "Engine/AI/Pathfinding/Grid/GridPathfindingManager.hpp"

void GridPathfindingManager::SetPathType(GridPathType type)
{
	m_currentPathType = type;
}

void GridPathfindingManager::SetHeuristic(std::function<int(IntVec2, IntVec2)> heuristic)
{
	m_heuristic = heuristic;
}

