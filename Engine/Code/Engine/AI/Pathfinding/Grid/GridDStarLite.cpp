#include "Engine/AI/Pathfinding/Grid/GridDStarLite.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include <algorithm>

GridDStarLite::GridDStarLite(IntVec2 grid)
{
	InitializeNodeGrid(grid);
}

void GridDStarLite::InitializeNodeGrid(IntVec2 const& grid)
{
	m_gridWidth = grid.x;
	m_gridHeight = grid.y;
	int mapDimensions = m_gridWidth * m_gridHeight;
	m_nodeGrid.resize(mapDimensions);
	for (int y = 0; y < grid.y; y++)
	{
		for (int x = 0; x < grid.x; x++)
		{
			int index = (y * m_gridWidth) + x;
			m_nodeGrid[index] = Node{ IntVec2(x, y), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), IntVec2(-1, -1), false, false };
		}
	}
}

void GridDStarLite::ComputeDStarLite(IntVec2 startPoint, IntVec2 goalPoint, std::vector<IntVec2>& outPath)
{	
	m_start = startPoint;
	m_goal = goalPoint;

	outPath.clear();

	Node* goalNode = GetNode(m_goal);
	goalNode->m_rhs = 0;
	Key key;
	CalculateKey(key, goalNode);
	m_openList.emplace(key, goalNode);
	goalNode->m_isOpen = true;

	ComputeShortestPath(m_start, m_goal);

	IntVec2 current = m_start;
	while (current != m_goal)
	{
		Node* currentNode = GetNode(current);
		IntVec2 bestNeighbor = IntVec2(-1, -1);
		float minCost = std::numeric_limits<float>::max();

		ForEachNeighbor(currentNode, [&](Node* neighbor, IntVec2)
		{
			float cost = GetCost(current, neighbor->m_position) + neighbor->m_totalgCost;
			if (cost < minCost)
			{
				minCost = cost;
				bestNeighbor = neighbor->m_position;
			}
		});

		if (bestNeighbor == IntVec2(-1, -1)) break; // No path was found
		outPath.emplace_back(bestNeighbor);
		current = bestNeighbor;
	}
}

void GridDStarLite::ComputeShortestPath(IntVec2 start, IntVec2 goal)
{
	int maxSteps = MAX_CYCLES;
	while (!m_openList.empty())
	{
		maxSteps--;
		if (maxSteps <= 0) break;
		auto [topKey, current] = m_openList.top();
		m_openList.pop();

		if (current->m_isClosed || current->m_totalgCost == current->m_rhs) continue;

		Key newKey;
		CalculateKey(newKey,current);

		if (topKey < newKey)
		{
			m_openList.emplace(newKey, current);
			continue;
		}

		if (current->m_totalgCost > current->m_rhs)
		{
			current->m_totalgCost = current->m_rhs;
			UpdatePredecessors(current);
		}
		else
		{
			current->m_totalgCost = std::numeric_limits<float>::max();
			UpdateVertex(current);
			UpdatePredecessors(current);
		}
	}
}

void GridDStarLite::UpdatePredecessors(Node* node)
{
	ForEachNeighbor(node, [this](Node* neighbor, IntVec2 stepDir)
	{
		UpdateVertex(neighbor);
	});
}

void GridDStarLite::UpdateVertex(Node* node)
{
	if (node->m_position != m_goal)
	{
		float minRHS = std::numeric_limits<float>::max();
		ForEachNeighbor(node, [this, &minRHS, node](Node* neighbor, IntVec2 neighborCoords)
		{
			int cost = GetCost(node->m_position, neighborCoords);
			minRHS = std::min(minRHS, cost + neighbor->m_totalgCost);
		});
		node->m_rhs = minRHS;
	}

	if (node->m_isOpen)
	{
		node->m_isOpen = false;
		node->m_isClosed = true;
	}

	if (node->m_totalgCost != node->m_rhs)
	{
		Key key;
		CalculateKey(key, node);
		m_openList.emplace(key, node);
		node->m_isOpen = true;
	}
}

void GridDStarLite::CalculateKey(Key& key, Node* node)
{
	float min_g_rhs = std::min(node->m_totalgCost, node->m_rhs);
	int hCost = GetLengthSquared(m_start, node->m_position);
	key = { min_g_rhs + hCost + m_km, min_g_rhs };
}

void GridDStarLite::RecalculateNode(IntVec2 newStart)
{
	int heuristicChange = GetLengthSquared(m_start, newStart);
	m_km += heuristicChange;
	m_start = newStart;

	Node* startNode = GetNode(newStart);
	UpdateVertex(startNode);
}

void GridDStarLite::ForEachNeighbor(Node* node, std::function<void(Node*, IntVec2)> callback)
{
	int numDirections = (m_directionMode == DirectionMode::Cardinal4) ? NUM_CARDINAL_DIRECTIONS : NUM_INTERCARDINAL_DIRECTIONS;
	for (int direction = 0; direction < numDirections; direction++)
	{
		IntVec2 stepDirection = (m_directionMode == DirectionMode::Cardinal4) ? GetStepInCardinalDirection(CardinalDir(direction)) : GetStepInCardinalAndIntercardinalDirection(IntercardinalDir(direction));
		bool isDiagonal = IsDiagonal(stepDirection);
		IntVec2 neighborCoords = node->m_position + stepDirection;

		if (!m_isSolidCallback(neighborCoords))
		{
			if (isDiagonal || (!m_canMoveDiagonalCallback || m_canMoveDiagonalCallback(node->m_position, neighborCoords)))
			{
				if (neighborCoords.x < 0 || neighborCoords.x >= m_gridWidth || neighborCoords.y < 0 || neighborCoords.y >= m_gridHeight) continue;

				int neighborIndex = (neighborCoords.y * m_gridWidth) + neighborCoords.x;
				Node* neighbor = &m_nodeGrid[neighborIndex];

				callback(neighbor, stepDirection);
			}
		}
	}
}

int GridDStarLite::GetCost(IntVec2 currentPosition, IntVec2 neighborPosition)
{
	IntVec2 diff = currentPosition - neighborPosition;
	bool isDiagonal = (diff.x != 0 && diff.y != 0);
	return isDiagonal ? 141 : 100;
}

Node* GridDStarLite::GetNode(IntVec2 point)
{
	if (point.x < 0 || point.x >= m_gridWidth || point.y < 0 || point.y >= m_gridHeight) return nullptr;

	int index = point.y * m_gridWidth + point.x;
	return &m_nodeGrid[index];
}

void GridDStarLite::TestDStarLitePathFinding(IntVec2 start, IntVec2 goal, std::vector<IntVec2>& path)
{
	double timeBefore = GetCurrentTimeSeconds();
	ComputeDStarLite(start, goal, path);
	double timeAfter = GetCurrentTimeSeconds();

	float msElapsed = 1000.f * float(timeAfter - timeBefore);
	g_theConsole->AddLine(Rgba8::RED, Stringf("Generated a path of %i steps from (%i,%i) to (%i,%i) in %.02f ms", path.size(), start.x, start.y, goal.x, goal.y, msElapsed));

	size_t pathSizeInBytes = m_currentStoredPathfindingPath.size() * sizeof(IntVec2); // Approximate size of one path
	g_theConsole->AddLine(Rgba8::LIGHT_ORANGE, Stringf("The approximate size of one path is %i", pathSizeInBytes));
}
