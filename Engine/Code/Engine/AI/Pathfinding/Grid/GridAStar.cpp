#include "Engine/AI/Pathfinding/Grid/GridAStar.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

GridAStar::GridAStar(IntVec2 grid)
{
	InitializeNodeGrid(grid);
}

void GridAStar::InitializeNodeGrid(IntVec2 const& grid)
{
	m_grid = grid.x;
	int mapDimensions = m_grid * m_grid;
	m_nodeGrid.resize(mapDimensions);
	for (int y = 0; y < grid.y; y++)
	{
		for (int x = 0; x < grid.x; x++)
		{
			int index = (y * m_grid) + x;
			m_nodeGrid[index] = Node{ IntVec2(x, y), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), IntVec2(-1, -1), -1, -1 };
		}
	}
}

void GridAStar::ComputeAStar(IntVec2 start, IntVec2 goal, std::vector<IntVec2>& outPath)
{
	m_pathGen++;
	
	CustomHeap openList;
	int mapSize = m_grid * m_grid;
	openList.Resverve(mapSize);

	int startIndex = (start.y * m_grid) + start.x;

	if (startIndex < 0 || startIndex >= static_cast<int>(m_nodeGrid.size())) return;

	Node* startNode = &m_nodeGrid[startIndex];
	startNode->m_position = start;
	startNode->m_totalgCost = 0;
	startNode->m_fCost = static_cast<float>(GetLengthSquared(start, goal));
	startNode->m_openPathGen = m_pathGen;
	startNode->m_closedPathGen = -1;

	openList.Push(startNode);

	while (!openList.Empty())
	{
		Node* currentNode = openList.Pop();

		if (currentNode->m_closedPathGen == m_pathGen) continue;
		currentNode->m_closedPathGen = m_pathGen;

		// If we found goal then compute path
		if (currentNode->m_position == goal)
		{
			if (!outPath.empty()) outPath.clear();

			while (currentNode->m_position != start)
			{
				outPath.emplace_back(currentNode->m_position);
				int parentIndex = (currentNode->m_parent.y * m_grid) + currentNode->m_parent.x;
				currentNode = &m_nodeGrid[parentIndex];
			}
			openList.Clear();
			return;
		}

		// Distance Threshold check
		int distanceSq = GetLengthSquared(currentNode->m_position, start);
		if (distanceSq > MAX_DIST_THRESHOLD * MAX_DIST_THRESHOLD)
		{
			// Generate Partial Path
			if (!outPath.empty()) outPath.clear();

			while (currentNode->m_position != start)
			{
				outPath.emplace_back(currentNode->m_position);
				int parentIndex = (currentNode->m_parent.y * m_grid) + currentNode->m_parent.x;
				currentNode = &m_nodeGrid[parentIndex];
			}
			return;
		}

		int numDirections = (m_directionMode == DirectionMode::Cardinal4) ? NUM_CARDINAL_DIRECTIONS : NUM_INTERCARDINAL_DIRECTIONS;
		for (int direction = 0; direction < numDirections; direction++)
		{
			IntVec2 stepDirection = (m_directionMode == DirectionMode::Cardinal4) ? GetStepInCardinalDirection(CardinalDir(direction)) : GetStepInCardinalAndIntercardinalDirection(IntercardinalDir(direction));
			bool isDiagonal = IsDiagonal(stepDirection);
			IntVec2 neighborCoords = currentNode->m_position + stepDirection;

			if (!m_isSolidCallback(neighborCoords))
			{
				if (isDiagonal || (!m_canMoveDiagonalCallback || m_canMoveDiagonalCallback(currentNode->m_position, neighborCoords)))
				{
					if (neighborCoords.x < 0 || neighborCoords.x >= m_grid || neighborCoords.y < 0 || neighborCoords.y >= m_grid) continue;

					int neighborIndex = (neighborCoords.y * m_grid) + neighborCoords.x;
					Node* neighborNode = &m_nodeGrid[neighborIndex];
					if (neighborNode->m_closedPathGen == m_pathGen) continue;

					int localgCost = GetLengthSquared(neighborCoords, currentNode->m_position);
					float totatgCost = currentNode->m_totalgCost + localgCost;
					int hCost = GetLengthSquared(neighborCoords, goal);
					float fCost = totatgCost + hCost;

					if (fCost < neighborNode->m_fCost)
					{
						neighborNode->m_position = neighborCoords;
						neighborNode->m_totalgCost = totatgCost;
						neighborNode->m_fCost = fCost;
						neighborNode->m_parent = currentNode->m_position;

						if (neighborNode->m_openPathGen != m_pathGen)
						{
							neighborNode->m_openPathGen = m_pathGen;
							openList.Push(neighborNode);
						}
						else
						{
							openList.DecreaseKey(neighborNode);
						}
					}
				}
			}
		}
	}

	outPath.clear();
}

void GridAStar::TestAStarPathFinding(IntVec2 start, IntVec2 goal, std::vector<IntVec2>& path)
{
	double timeBefore = GetCurrentTimeSeconds();
	ComputeAStar(start, goal, path);
	double timeAfter = GetCurrentTimeSeconds();

	float msElapsed = 1000.f * float(timeAfter - timeBefore);
	g_theConsole->AddLine(Rgba8::RED, Stringf("Generated a path of %i steps from (%i,%i) to (%i,%i) in %.02f ms", path.size(), start.x, start.y, goal.x, goal.y, msElapsed));

	size_t pathSizeInBytes = path.size() * sizeof(IntVec2); // Approximate size of one path
	g_theConsole->AddLine(Rgba8::LIGHT_ORANGE, Stringf("The approximate size of one path is %i", pathSizeInBytes));
}

void GridAStar::CustomHeap::Push(Node* node)
{
	m_elements.emplace_back(node);
	node->m_heapIndex = static_cast<int>(m_elements.size()) - 1;
	PercolateUp(node->m_heapIndex);
}

void GridAStar::CustomHeap::DecreaseKey(Node* node)
{
	if (node->m_heapIndex >= 0 && node->m_heapIndex < static_cast<int>(m_elements.size()))
	{
		PercolateUp(static_cast<size_t>(node->m_heapIndex));
	}
}

Node* GridAStar::CustomHeap::Pop()
{
	if (m_elements.empty()) return nullptr;

	Node* minNode = m_elements.front();
	m_elements[0] = m_elements.back();
	m_elements.pop_back();
	PercolateDown(0);
	minNode->m_heapIndex = -1;

	return minNode;
}

void GridAStar::CustomHeap::PercolateUp(size_t index)
{
	while (index > 0)
	{
		size_t parent = (index - 1) / 2;
		if (m_elements[index]->m_fCost < m_elements[parent]->m_fCost)
		{
			std::swap(m_elements[index], m_elements[parent]);
			std::swap(m_elements[index]->m_heapIndex, m_elements[parent]->m_heapIndex);
			index = parent;
		}
		else
		{
			break;
		}
	}
}

void GridAStar::CustomHeap::PercolateDown(size_t index)
{
	size_t leftChild, rightChild, smallest;

	while (true)
	{
		leftChild = index * 2 + 1;
		rightChild = index * 2 + 2;
		smallest = index;

		if (leftChild < m_elements.size() && m_elements[leftChild]->m_fCost < m_elements[smallest]->m_fCost)
		{
			smallest = leftChild;
		}

		if (rightChild < m_elements.size() && m_elements[rightChild]->m_fCost < m_elements[smallest]->m_fCost)
		{
			smallest = rightChild;
		}

		if (smallest != index)
		{
			std::swap(m_elements[index], m_elements[smallest]);
			std::swap(m_elements[index]->m_heapIndex, m_elements[smallest]->m_heapIndex);
			index = smallest;
		}
		else
		{
			break;
		}
	}
}
