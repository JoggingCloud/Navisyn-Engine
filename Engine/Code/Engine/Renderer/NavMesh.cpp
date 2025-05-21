#include "Engine/Renderer/NavMesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Utilities/Prop.hpp"
#include <algorithm>
#include <queue>

extern Renderer* g_theRenderer;
extern RandomNumberGenerator g_rng;

NavMesh::~NavMesh()
{
	SafeDelete(m_solidVertexBuffer);
	SafeDelete(m_wireVertexBuffer);
	SafeDelete(m_bvhVertexBuffer);
	SafeDelete(m_bvhIndexBuffer);
	SafeDelete(m_heatmapVertexBuffer);

	m_heatmapVertexes.clear();
	m_bvhIndexes.clear();
	m_bvhVertexes.clear();
	m_solidVerts.clear();
	m_wireVerts.clear();
	m_triangles.clear();
}

void NavMesh::CreateBuffers()
{
	m_solidVertexBuffer = g_theRenderer->CreateVertexBuffer(m_vertexes.size());
	g_theRenderer->CopyCPUToGPU(m_solidVerts.data(), m_solidVerts.size() * sizeof(Vertex_PCU), m_solidVertexBuffer);
	
	m_wireVertexBuffer = g_theRenderer->CreateVertexBuffer(m_vertexes.size());
	g_theRenderer->CopyCPUToGPU(m_wireVerts.data(), m_wireVerts.size() * sizeof(Vertex_PCU), m_wireVertexBuffer);

	m_bvhVertexBuffer = g_theRenderer->CreateVertexBuffer(m_bvhVertexes.size());
	g_theRenderer->CopyCPUToGPU(m_bvhVertexes.data(), m_bvhVertexes.size() * sizeof(Vertex_PCU), m_bvhVertexBuffer);

	m_bvhIndexBuffer = g_theRenderer->CreateIndexBuffer(m_bvhIndexes.size());
	g_theRenderer->CopyCPUToGPU(m_bvhIndexes.data(), m_bvhIndexes.size() * sizeof(unsigned int), m_bvhIndexBuffer);

	m_heatmapVertexBuffer = g_theRenderer->CreateVertexBuffer(m_heatmapVertexes.size());
	g_theRenderer->CopyCPUToGPU(m_heatmapVertexes.data(), m_heatmapVertexes.size() * sizeof(Vertex_PCU), m_heatmapVertexBuffer);
}

void NavMesh::CreateNavMesh(std::vector<Vec3>& vertexPoints, int mapWidth, int mapHeight, std::vector<int>& vertexMapping)
{
	double timeBefore = GetCurrentTimeSeconds();

	m_triangles.clear();
	m_vertexes.clear();

	for (int y = 0; y < mapHeight - 1; y++)
	{
		for (int x = 0; x < mapWidth - 1; x++)
		{
			int BL = y * mapWidth + x;
			int BR = BL + 1;

			int TL = BL + mapWidth;
			int TR = TL + 1;

			if (vertexMapping[BL] == -1 || vertexMapping[BR] == -1 ||
				vertexMapping[TL] == -1 || vertexMapping[TR] == -1) 
			{
				continue;
			}

			// Map to valid vertex indices
			int validBL = vertexMapping[BL];
			int validBR = vertexMapping[BR];
			int validTR = vertexMapping[TR];
			int validTL = vertexMapping[TL];

			// Add vertexes to store 
			m_vertexes.emplace_back(vertexPoints[validBL]);
			m_vertexes.emplace_back(vertexPoints[validBR]);
			m_vertexes.emplace_back(vertexPoints[validTR]);

			m_vertexes.emplace_back(vertexPoints[validTR]);
			m_vertexes.emplace_back(vertexPoints[validTL]);
			m_vertexes.emplace_back(vertexPoints[validBL]);

			NavMeshTri triangleOne;
			triangleOne.m_vertIndexes[0] = static_cast<int>(m_vertexes.size()) - 6;
			triangleOne.m_vertIndexes[1] = static_cast<int>(m_vertexes.size()) - 5;
			triangleOne.m_vertIndexes[2] = static_cast<int>(m_vertexes.size()) - 4;

			triangleOne.m_neighborTriIndexes[0] = -1;
			triangleOne.m_neighborTriIndexes[1] = -1;
			triangleOne.m_neighborTriIndexes[2] = -1;

			m_triangles.emplace_back(triangleOne);
			AddVertsFor3DTriangle(m_solidVerts, vertexPoints[validBL], vertexPoints[validBR], vertexPoints[validTR], Rgba8::ELECTRIC_BLUE_LIGHT);
			AddVertsFor3DTriangle(m_wireVerts, vertexPoints[validBL], vertexPoints[validBR], vertexPoints[validTR], Rgba8::ELECTRIC_BLUE);

			NavMeshTri triangleTwo;
			triangleTwo.m_vertIndexes[0] = static_cast<int>(m_vertexes.size()) - 3;
			triangleTwo.m_vertIndexes[1] = static_cast<int>(m_vertexes.size()) - 2;
			triangleTwo.m_vertIndexes[2] = static_cast<int>(m_vertexes.size()) - 1;

			triangleTwo.m_neighborTriIndexes[0] = -1;
			triangleTwo.m_neighborTriIndexes[1] = -1;
			triangleTwo.m_neighborTriIndexes[2] = -1;

			m_triangles.emplace_back(triangleTwo);
			AddVertsFor3DTriangle(m_solidVerts, vertexPoints[validTR], vertexPoints[validTL], vertexPoints[validBL], Rgba8::ELECTRIC_BLUE_LIGHT);
			AddVertsFor3DTriangle(m_wireVerts, vertexPoints[validTR], vertexPoints[validTL], vertexPoints[validBL], Rgba8::ELECTRIC_BLUE);
		}
	}

	double timeAfter = GetCurrentTimeSeconds();
	float msElapsed = 1000.f * float(timeAfter - timeBefore);
	g_theConsole->AddLine(Rgba8::DARK_RED, Stringf("NavMesh generated with %i triangles in %.02f ms", static_cast<int>(m_triangles.size()), msElapsed));

	double timeBeforeNeighbors = GetCurrentTimeSeconds();
	ComputeNeighborsHashed();
	double timeAfterNeighbors = GetCurrentTimeSeconds();
	float msElapsedNeighbors = 1000.f * float(timeAfterNeighbors - timeBeforeNeighbors);
	g_theConsole->AddLine(Rgba8::DARK_GREEN, Stringf("ComputeNeighbors took %.02f ms", msElapsedNeighbors));

	BuildBVH();
	DrawAllBVH(m_bvhRoot, 0);

	ConstructHeatmap();
}

void NavMesh::ComputeNeighborsHashed()
{
	std::unordered_map<Vec3, std::vector<int>> vertexPosToTriangles;

	for (int triangleIndex = 0; triangleIndex < static_cast<int>(m_triangles.size()); triangleIndex++)
	{
		for (int corner = 0; corner < 3; corner++)
		{
			Vec3 const& vert = m_vertexes[m_triangles[triangleIndex].m_vertIndexes[corner]];
			vertexPosToTriangles[vert].emplace_back(triangleIndex);
		}
	}

	for (int triangleIndex = 0; triangleIndex < static_cast<int>(m_triangles.size()); triangleIndex++)
	{
		std::unordered_set<int> possibleNeighbors;

		// Collect possible neighbors that share at least one vertex
		for (int edgeVert = 0; edgeVert < 3; edgeVert++)
		{
			const Vec3& vert = m_vertexes[m_triangles[triangleIndex].m_vertIndexes[edgeVert]];
			auto found = vertexPosToTriangles.find(vert);
			if (found != vertexPosToTriangles.end())
			{
				for (int neighborTri : found->second)
				{
					if (neighborTri != triangleIndex)
					{
						possibleNeighbors.insert(neighborTri);
					}
				}
			}
		}

		// Check if exactly 2 vertices match (meaning they share an edge)
		for (int neighborTri : possibleNeighbors)
		{
			int sharedVerts = 0;
			for (int iVert = 0; iVert < 3; iVert++)
			{
				const Vec3& vertexA = m_vertexes[m_triangles[triangleIndex].m_vertIndexes[iVert]];
				for (int jVert = 0; jVert < 3; jVert++)
				{
					const Vec3& vertexB = m_vertexes[m_triangles[neighborTri].m_vertIndexes[jVert]];
					if (vertexA == vertexB)
					{
						sharedVerts++;
					}
				}
			}

			if (sharedVerts == 2)
			{
				// Find empty edge slot (-1) and store neighbor
				for (int edge = 0; edge < 3; ++edge)
				{
					if (m_triangles[triangleIndex].m_neighborTriIndexes[edge] == -1)
					{
						m_triangles[triangleIndex].m_neighborTriIndexes[edge] = neighborTri;
						break;
					}
				}
			}
		}
	}
}

void NavMesh::ComputeNeighborsBruteForce()
{
	for (size_t i = 0; i < m_triangles.size(); i++)
	{
		for (size_t j = 0; j < m_triangles.size(); j++)
		{
			if (i == j) continue; // Skip self-comparison

			int sharedVerts = 0;

			// Count how many vertices are shared between triangle i and triangle j
			for (int triangleIVerts = 0; triangleIVerts < 3; triangleIVerts++)
			{
				for (int triangleJVerts = 0; triangleJVerts < 3; triangleJVerts++)
				{
					const Vec3& vertexI = m_vertexes[m_triangles[i].m_vertIndexes[triangleIVerts]];
					const Vec3& vertexJ = m_vertexes[m_triangles[j].m_vertIndexes[triangleJVerts]];

					if (vertexI == vertexJ)
					{
						sharedVerts++;
					}
				}
			}

			// If two vertices are shared, they are neighbors
			if (sharedVerts == 2)
			{
				for (int edgeA = 0; edgeA < 3; edgeA++)
				{
					if (m_triangles[i].m_neighborTriIndexes[edgeA] == -1)
					{
						m_triangles[i].m_neighborTriIndexes[edgeA] = static_cast<int>(j);
						break; // Assign only one neighbor per edge
					}
				}
			}
		}
	}
}

void NavMesh::RemoveRandomTriangleCluster()
{
	for (int i = 0; i < m_numTriangleClusters; i++)
	{
		int randomIndex = GetRandomNavMeshTriangleIndex();
		std::unordered_set<int> trianglesToRemove;
		trianglesToRemove.insert(randomIndex); // Start with one triangle

		NavMeshTri& removedTriangle = m_triangles[randomIndex];

		// Randomly decide how many neighbors to also remove
		//g_rng.SetSeed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
		int neighborsToRemove = g_rng.SRollRandomIntInRange(m_numNeighborsToRemove.m_min, m_numNeighborsToRemove.m_max);
		int removedCount = 0;

		for (int edge = 0; edge < 3; edge++)
		{
			int neighborID = removedTriangle.m_neighborTriIndexes[edge];
			if (neighborID == -1 || trianglesToRemove.count(neighborID)) continue; // Skip invalid or already marked

			trianglesToRemove.insert(neighborID);
			removedCount++;

			if (removedCount >= neighborsToRemove) break; // Stop once we remove enough neighbors
		}

		// Remove all marked triangles
		std::vector<int> trisToRemove(trianglesToRemove.begin(), trianglesToRemove.end());
		std::sort(trisToRemove.rbegin(), trisToRemove.rend());
		for (int triangleID : trisToRemove)
		{
			if (triangleID < 0 || triangleID >= static_cast<int>(m_triangles.size())) continue;

			NavMeshTri& triangle = m_triangles[triangleID];

			// Unlink neighbors
			for (int edge = 0; edge < 3; edge++)
			{
				int neighborID = triangle.m_neighborTriIndexes[edge];
				if (neighborID == -1) continue;

				NavMeshTri& neighborTriangle = m_triangles[neighborID];

				for (int neighborEdge = 0; neighborEdge < 3; neighborEdge++)
				{
					if (neighborTriangle.m_neighborTriIndexes[neighborEdge] == triangleID)
					{
						neighborTriangle.m_neighborTriIndexes[neighborEdge] = -1;
						break;
					}
				}
			}

			int lastIndex = static_cast<int>(m_triangles.size()) - 1;
			if (triangleID != lastIndex) // Avoid replacing if it's already the last triangle
			{
				// Copy the last triangle's data to the position of the removed triangle
				m_triangles[triangleID] = m_triangles[lastIndex];

				// Update the neighbors of the last triangle to point to its new index
				NavMeshTri& movedTriangle = m_triangles[triangleID];
				for (int edge = 0; edge < 3; edge++)
				{
					int neighborID = movedTriangle.m_neighborTriIndexes[edge];
					if (neighborID == -1) continue;

					NavMeshTri& neighborTriangle = m_triangles[neighborID];
					for (int neighborEdge = 0; neighborEdge < 3; neighborEdge++)
					{
						if (neighborTriangle.m_neighborTriIndexes[neighborEdge] == lastIndex)
						{
							neighborTriangle.m_neighborTriIndexes[neighborEdge] = triangleID; // Update to new index
							break;
						}
					}
				}
			}

			// Remove the last triangle from the array
			m_triangles.pop_back();
		}
	}
}

void NavMesh::RemoveTrianglesAffectedByProps(std::vector<Prop*>& props)
{
	if (props.empty()) return;

	std::vector<size_t> trianglesToRemove;
	float maxAgentRadius = 0.5f; // #ToDo change value if i change the maximum radius size of agents in the xml

	// Iterate through all triangle in the NavMesh
	for (size_t i = 0; i < m_triangles.size(); i++) 
	{
		NavMeshTri& triangle = m_triangles[i];

		Vec3 v0 = m_vertexes[triangle.m_vertIndexes[0]];
		Vec3 v1 = m_vertexes[triangle.m_vertIndexes[1]];
		Vec3 v2 = m_vertexes[triangle.m_vertIndexes[2]];

		// Check if the triangle is blocked by any prop
		bool isBlocked = false;
		for (const Prop* prop : props) 
		{
			if (prop->IsTriangleBlockedByAABB3D(v0, v1, v2, maxAgentRadius) || 
				prop->IsTriangleBlockedByCylinder3D(v0, v1, v2, maxAgentRadius)) 
			{
				isBlocked = true;
				break;
			}
		}

		if (isBlocked)
		{
			for (int edge = 0; edge < 3; edge++)
			{
				int neighborID = triangle.m_neighborTriIndexes[edge];
				if (neighborID == -1) continue;

				NavMeshTri& neighborTriangle = m_triangles[neighborID];

				for (int neighborEdge = 0; neighborEdge < 3; neighborEdge++)
				{
					if (neighborTriangle.m_neighborTriIndexes[neighborEdge] == static_cast<int>(i))
					{
						neighborTriangle.m_neighborTriIndexes[neighborEdge] = -1;
					}
				}
			}
			trianglesToRemove.emplace_back(i);
		}
	}

	// Remove triangles in reverse order to avoid messing up the indexing
	for (size_t i = trianglesToRemove.size(); i > 0; i--)
	{
		size_t indexToRemove = trianglesToRemove[i - 1];
		int lastIndex = static_cast<int>(m_triangles.size()) - 1;

		if (indexToRemove != lastIndex)
		{
			m_triangles[indexToRemove] = m_triangles[lastIndex];

			NavMeshTri& movedTriangle = m_triangles[indexToRemove];
			for (int edge = 0; edge < 3; edge++)
			{
				int neighborID = movedTriangle.m_neighborTriIndexes[edge];
				if (neighborID == -1) continue;

				NavMeshTri& neighborTriangle = m_triangles[neighborID];
				for (int neighborEdge = 0; neighborEdge < 3; neighborEdge++)
				{
					if (neighborTriangle.m_neighborTriIndexes[neighborEdge] == lastIndex)
					{
						neighborTriangle.m_neighborTriIndexes[neighborEdge] = static_cast<int>(indexToRemove);
					}
				}
			}
		}

		// Remove the last triangle
		m_triangles.pop_back();
	}

	// Ensure neighbors are fully validated
	for (NavMeshTri& triangle : m_triangles)
	{
		for (int& neighborIndex : triangle.m_neighborTriIndexes)
		{
			if (neighborIndex >= static_cast<int>(m_triangles.size()))
			{
				neighborIndex = -1; // Ensure no out-of-bounds neighbor reference
			}
		}
	}

	// #ToDo i don't think i need to do this
	// Rather i could just valid the nav mesh 
	RebuildNavMeshVerts();
}

void NavMesh::PopulateDistanceField(NavMeshHeatMap& outDistanceField, float maxCost) const
{
	outDistanceField.SetAllValues(maxCost); // Set all triangle indexes to max value of 9999.f

	std::queue<int> openSet;

	for (int i = 0; i < static_cast<int>(m_triangles.size()); i++)
	{
		const NavMeshTri& tri = m_triangles[i];
		for (int neighbor : tri.m_neighborTriIndexes)
		{
			if (neighbor == -1)
			{
				outDistanceField.SetValue(i, 0.f);   // Seed with 0 (danger zone)
				openSet.emplace(i);
			}
		}
	}

	while (!openSet.empty())
	{
		int currentIndex = openSet.front();
		openSet.pop();

		float currentValue = outDistanceField.GetValue(currentIndex);
		const NavMeshTri& currentTri = m_triangles[currentIndex];

		for (int neighborIndex : currentTri.m_neighborTriIndexes)
		{
			if (neighborIndex == -1) continue;

			float neighborValue = outDistanceField.GetValue(neighborIndex);
			float newValue = currentValue + 1.f; 

			if (newValue < neighborValue)
			{
				outDistanceField.SetValue(neighborIndex, newValue);
				openSet.emplace(neighborIndex);
			}
		}
	}

	// Mark unreachable triangles
	for (size_t i = 0; i < m_triangles.size(); i++)
	{
		if (outDistanceField.GetValue(static_cast<int>(i)) == maxCost)
		{
			outDistanceField.SetValue(static_cast<int>(i), m_specialValue);
		}
	}
}

Vec3 NavMesh::ClampPositionToNavMesh(Vec3 const& position)
{
	int closestTriangleIndex = GetClosestTriangleIndex(position);
	if (closestTriangleIndex == -1) { return position; }

	const NavMeshTri& triangle = m_triangles[closestTriangleIndex];
	Vec3 const& v0 = m_vertexes[triangle.m_vertIndexes[0]];
	Vec3 const& v1 = m_vertexes[triangle.m_vertIndexes[1]];
	Vec3 const& v2 = m_vertexes[triangle.m_vertIndexes[2]];

	return GetNearestPointOnTriangle3D(position, v0, v1, v2);
}

Vec3 NavMesh::CalculateCentroid(const NavMeshTri& triangle) const
{
	// Validate indices before using them
	for (int i = 0; i < 3; i++)
	{
		if (triangle.m_vertIndexes[i] < 0 || triangle.m_vertIndexes[i] >= m_vertexes.size())
		{
			return Vec3::ZERO;
		}
	}

	Vec3 v0 = m_vertexes[triangle.m_vertIndexes[0]];
	Vec3 v1 = m_vertexes[triangle.m_vertIndexes[1]];
	Vec3 v2 = m_vertexes[triangle.m_vertIndexes[2]];

	return (v0 + v1 + v2) / 3.f;
}

void NavMesh::RebuildNavMeshVerts()
{	
	m_solidVerts.clear();
	m_wireVerts.clear();

	for (NavMeshTri& tri : m_triangles)
	{
		Vec3 v0 = m_vertexes[tri.m_vertIndexes[0]];
		Vec3 v1 = m_vertexes[tri.m_vertIndexes[1]];
		Vec3 v2 = m_vertexes[tri.m_vertIndexes[2]];

		// Add updated verts
		AddVertsFor3DTriangle(m_solidVerts, v0, v1, v2, Rgba8::MORE_TRANSLUCENT);
		AddVertsFor3DTriangle(m_wireVerts, v0, v1, v2, Rgba8::ELECTRIC_BLUE);
	}

	[[maybe_unused]] bool isValidNavMesh = ValidateNavMesh();
}

void NavMesh::RenderHeatMap() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexBuffer(m_heatmapVertexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_heatmapVertexes.size()));
}

void NavMesh::RenderBVH() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerState(RasterizerMode::WIREFRAME_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexBufferIndex(m_bvhVertexBuffer, m_bvhIndexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_bvhVertexes.size()));
}

void NavMesh::RenderNavMesh() const
{
	if (m_solidVertexBuffer)
	{
		if (!m_solidVerts.empty())
		{
			g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_BACK);
			g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
			g_theRenderer->SetModelConstants();
			g_theRenderer->BindShader(nullptr);
			g_theRenderer->BindTexture(0, nullptr);
			g_theRenderer->DrawVertexBuffer(m_solidVertexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_vertexes.size()));
		}
	}
	
	if (m_wireVertexBuffer)
	{
		if (!m_wireVerts.empty())
		{
			g_theRenderer->SetModelConstants();
			g_theRenderer->SetRasterizerState(RasterizerMode::WIREFRAME_CULL_BACK);
			g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
			g_theRenderer->BindShader(nullptr);
			g_theRenderer->BindTexture(0, nullptr);
			g_theRenderer->DrawVertexBuffer(m_wireVertexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_vertexes.size()));
		}
	}
}

void NavMesh::BuildBVH()
{
	m_debugBVHBoxes.clear(); 

	m_bvhRoot = new BVHNode();

	std::vector<int> allTriangles;
	for (int i = 0; i < static_cast<int>(m_triangles.size()); i++)
	{
		allTriangles.emplace_back(i);

		const NavMeshTri& triangle = m_triangles[i];
		m_bvhRoot->StretchToIncludePoint(m_vertexes[triangle.m_vertIndexes[0]]);
		m_bvhRoot->StretchToIncludePoint(m_vertexes[triangle.m_vertIndexes[1]]);
		m_bvhRoot->StretchToIncludePoint(m_vertexes[triangle.m_vertIndexes[2]]);
	}

	BuildBVHRecursive(*m_bvhRoot, allTriangles);
	CollectBVHNodesInOrder(m_bvhRoot);
}

void NavMesh::BuildBVHRecursive(BVHNode& node, std::vector<int> triangleIndices, int depth /*= 0*/)
{
	node.m_triangleIndexes = triangleIndices;

	for (int triangleIndex : triangleIndices)
	{
		const NavMeshTri& tri = m_triangles[triangleIndex];
		node.StretchToIncludePoint(m_vertexes[tri.m_vertIndexes[0]]);
		node.StretchToIncludePoint(m_vertexes[tri.m_vertIndexes[1]]);
		node.StretchToIncludePoint(m_vertexes[tri.m_vertIndexes[2]]);
	}

	// Base case: too few edges or too deep become a leaf node
	if (triangleIndices.size() <= MAX_TRIANGLES_PER_LEAF || depth >= MAX_BVH_DEPTH) 
	{
//		DebuggerPrintf("STOP — Leaf at depth %d with %zu triangles\n", depth, triangleIndices.size());
		return;
	}

	// Split along longest axis
	Vec3 dims = node.GetDimensions();
	int longestAxis = (dims.x >= dims.y) ? 0 : 1; // X if wider, y if wider
	std::sort(triangleIndices.begin(), triangleIndices.end(),
		[&](int a, int b)
		{
			Vec3 aCenter = CalculateCentroid(m_triangles[a]);
			Vec3 bCenter = CalculateCentroid(m_triangles[b]);
			return GetComponent(aCenter, longestAxis) < GetComponent(bCenter, longestAxis);
		});

	size_t mid = triangleIndices.size() / 2;
	std::vector<int> left(triangleIndices.begin(), triangleIndices.begin() + mid);
	std::vector<int> right(triangleIndices.begin() + mid, triangleIndices.end());

	if (left.empty() || right.empty())
	{
// 		DebuggerPrintf("Degenerate split at depth %d for %zu triangles along axis %s\n",
// 			depth, triangleIndices.size(), longestAxis == 0 ? "X" : "Y");
		return;
	}

	BVHNode* leftChild = new BVHNode();
	node.m_childBoxes.emplace_back(leftChild);
	BuildBVHRecursive(*leftChild, left, depth + 1);

	BVHNode* rightChild = new BVHNode();
	node.m_childBoxes.emplace_back(rightChild);
	BuildBVHRecursive(*rightChild, right, depth + 1);
}

void NavMesh::CollectBVHNodesInOrder(const BVHNode* node)
{
	if (!node) return;
	m_debugBVHBoxes.emplace_back(node);

	for (const BVHNode* child : node->m_childBoxes)
	{
		CollectBVHNodesInOrder(child);
	}
}

void NavMesh::DrawAllBVH(BVHNode* node, int depth)
{
	if (!node) return;

	static int nodeCount = 0;
	nodeCount++;

	// Color code: Green for leaf, Red for internal
	Rgba8 color = node->IsLeafNode() ? Rgba8::GREEN : Rgba8::RED;

	AddVertsForAABB3D(m_bvhVertexes, m_bvhIndexes, AABB3(node->m_mins, node->m_maxs), color);

	// Recurse if not leaf
	for (BVHNode* child : node->m_childBoxes)
	{
		DrawAllBVH(child, depth + 1);
	}

	if (!m_debugBVHBoxes.empty() && m_currentBVHBoxIndex < (int)m_debugBVHBoxes.size())
	{
		const BVHNode* current = m_debugBVHBoxes[m_currentBVHBoxIndex];
		AddVertsForAABB3D(m_bvhVertexes, m_bvhIndexes, AABB3(current->m_mins, current->m_maxs), Rgba8::MAGENTA);
	}

}

void NavMesh::ConstructHeatmap()
{
	m_heatMap = new NavMeshHeatMap(this);
	m_heatmapVertexes.resize(m_triangles.size() * 3);
	PopulateDistanceField(*m_heatMap, m_maxDistanceFieldCost);
	m_heatMap->AddVertsForDebugDraw(m_heatmapVertexes, FloatRange(0.f, m_heatMap->GetHighestHeat()), Rgba8::RED, Rgba8::BANANA_YELLOW, -2.f, Rgba8::PURPLE);
}

void NavMesh::DebugDrawNeighbors() const
{
	for (size_t i = 0; i < m_triangles.size(); i++)
	{
		const NavMeshTri& tri = m_triangles[i];

		// Get triangle center
		Vec3 triCenter = (
			m_vertexes[tri.m_vertIndexes[0]] +
			m_vertexes[tri.m_vertIndexes[1]] +
			m_vertexes[tri.m_vertIndexes[2]]) / 3.f;

		for (int neighborID : tri.m_neighborTriIndexes)
		{
			if (neighborID == -1) continue;

			const NavMeshTri& neighborTri = m_triangles[neighborID];
			Vec3 neighborCenter = (
				m_vertexes[neighborTri.m_vertIndexes[0]] +
				m_vertexes[neighborTri.m_vertIndexes[1]] +
				m_vertexes[neighborTri.m_vertIndexes[2]]) / 3.f;

			// Optional: only draw one direction of the link to avoid duplicate lines
			if (i < static_cast<size_t>(neighborID))
			{
				DebugAddWorldArrow(triCenter, neighborCenter, 0.8f, 0.1f, 0.05f, 0.f, Rgba8::BANANA_YELLOW, Rgba8::BANANA_YELLOW, DebugRenderMode::ALWAYS);
			}
		}
	}
}

Vec3 NavMesh::GetRandomPointInsideTriangle(int triangleID) const
{
	if (m_triangles.empty()) return Vec3::ZERO;

	// Get triangle's vertex indices
	const NavMeshTri& triangle = m_triangles[triangleID];
	const Vec3& p0 = m_vertexes[triangle.m_vertIndexes[0]];
	const Vec3& p1 = m_vertexes[triangle.m_vertIndexes[1]];
	const Vec3& p2 = m_vertexes[triangle.m_vertIndexes[2]];

	// Random barycentric coordinates
	//g_rng.SetSeed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	float r1 = g_rng.SRollRandomFloatInRange(0.f, 1.f);
	float r2 = g_rng.SRollRandomFloatInRange(0.f, 1.f);

	// Ensure the point is inside the triangle
	if (r1 + r2 > 1.f)
	{
		r1 = 1.f - r1;
		r2 = 1.f - r2;
	}

	// Compute the random point using barycentric interpolation
	Vec3 randomPoint = p0 + r1 * (p1 - p0) + r2 * (p2 - p0); 

	// Validate the point is within the triangle 
	if (IsPointInsideTriangle3D(randomPoint, p0, p1, p2))
	{
		return randomPoint;
	}

	return Vec3::ZERO;
}

Vec3 NavMesh::GetTriangleCentroid(int triangleID)
{
	if (m_triangles.empty()) return Vec3::ZERO;

	const NavMeshTri& triangle = m_triangles[triangleID];

	return CalculateCentroid(triangle);
}

float NavMesh::GetComponent(Vec3 const& vertex, int axis)
{
	switch (axis)
	{
	case 0: return vertex.x;
	case 1: return vertex.y;
	default: GUARANTEE_RECOVERABLE(false, "Invalid axis in GetComponent"); return 0.f;
	}
}

int NavMesh::GetClosestTriangleIndex(Vec3 const& position)
{
	float closestDistanceSq  = FLT_MAX;
	int closestIndex = -1;

	for (size_t i = 0; i < m_triangles.size(); i++)
	{
		const NavMeshTri& tri = m_triangles[i];
		Vec3 const& v0 = m_vertexes[tri.m_vertIndexes[0]];
		Vec3 const& v1 = m_vertexes[tri.m_vertIndexes[1]];
		Vec3 const& v2 = m_vertexes[tri.m_vertIndexes[2]];

		Vec3 closestPoint = GetNearestPointOnTriangle3D(position, v0, v1, v2);
		float distanceSq = GetDistanceSquared3D(position, closestPoint);

		if (distanceSq < closestDistanceSq)
		{
			closestDistanceSq = distanceSq;
			closestIndex = static_cast<int>(i);
		}
	}
	return closestIndex;
}

int NavMesh::GetRandomNavMeshTriangleIndex() const
{
	if (m_triangles.empty()) return -1;

	//g_rng.SetSeed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	int randomTriangleIndex = g_rng.SRollRandomIntInRange(0, static_cast<int>(m_triangles.size() - 1));

	return randomTriangleIndex;
}

int NavMesh::GetTriangleIndex(const NavMeshTri* triangle) const
{
	for (size_t i = 0; i < m_triangles.size(); i++)
	{
		if (&m_triangles[i] == triangle)
		{
			return static_cast<int>(i);
		}
	}
	return -1; 
}

int NavMesh::GetContainingTriangleIndex(Vec3 const& point) const
{
	if (!m_bvhRoot) return -1;
	return GetRecursiveTriangleIndex(*m_bvhRoot, point);
}

int NavMesh::GetRecursiveTriangleIndex(BVHNode const& node, Vec3 const& point) const
{
	if (!node.IsPointInside(point)) return -1;

	if (node.IsLeafNode())
	{
		for (int triangleIndex : node.m_triangleIndexes)
		{
			const NavMeshTri& triangle = m_triangles[triangleIndex];
			Vec3 const& v0 = m_vertexes[triangle.m_vertIndexes[0]];
			Vec3 const& v1 = m_vertexes[triangle.m_vertIndexes[1]];
			Vec3 const& v2 = m_vertexes[triangle.m_vertIndexes[2]];

			if (IsPointInsideTriangle3D(point, v0, v1, v2)) return triangleIndex;
		}
	}

	// Otherwise recurse
	for (const BVHNode* child : node.m_childBoxes)
	{
		int result = GetRecursiveTriangleIndex(*child, point);
		if (result != -1) return result;
	}

	return -1;
}

NavMeshTri* NavMesh::GetNavMeshTriangle(int triangleID)
{
	if (triangleID < 0 || triangleID >= static_cast<int>(m_triangles.size())) { return nullptr; }
	return &m_triangles[triangleID];
}

bool NavMesh::FindNearestTriangle(Vec3 const& point, int& outTriangleIndex, Vec3& outProjectedPoint)
{
	float bestDistSq = FLT_MAX;
	bool found = false;

	for (int i = 0; i < m_triangles.size(); i++)
	{
		const NavMeshTri& tri = m_triangles[i];
		const Vec3& v0 = m_vertexes[tri.m_vertIndexes[0]];
		const Vec3& v1 = m_vertexes[tri.m_vertIndexes[1]];
		const Vec3& v2 = m_vertexes[tri.m_vertIndexes[2]];

		Vec3 projected = GetNearestPointOnTriangle3D(point, v0, v1, v2);
		float distSq = GetDistanceSquared3D(point, projected);

		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			outProjectedPoint = projected;
			outTriangleIndex = i;
			found = true;
		}
	}

	return found;
}

bool NavMesh::ValidateNavMesh() const
{
	for (size_t i = 0; i < m_triangles.size(); i++) 
	{
		const NavMeshTri& tri = m_triangles[i];

		// Check if all neighbor indices are valid
		for (int neighborIndex : tri.m_neighborTriIndexes) 
		{
			if (neighborIndex < -1 || neighborIndex >= static_cast<int>(m_triangles.size())) 
			{
				DebuggerPrintf("Invalid neighbor index in Triangle %zu: %d (Max valid: %d)\n", i, neighborIndex, static_cast<int>(m_triangles.size()) - 1);
				ERROR_AND_DIE(Stringf("Error: Triangle %zu has an invalid neighbor index %d.\n", i, neighborIndex));
			}
		}

		// Verify connectivity with neighbors
		for (int edge = 0; edge < 3; edge++) 
		{
			int neighborID = tri.m_neighborTriIndexes[edge];
			if (neighborID == -1) continue; // No neighbor on this edge

			const NavMeshTri& neighborTri = m_triangles[neighborID];
			bool found = false;

			for (int neighborEdge = 0; neighborEdge < 3; ++neighborEdge)
			{
				if (neighborTri.m_neighborTriIndexes[neighborEdge] == static_cast<int>(i)) 
				{
					found = true;

					// Verify shared vertices
					int sharedVerts = 0;
					Vec3 sharedVertA, sharedVertB;

					for (int triangleIVert = 0; triangleIVert < 3; triangleIVert++)
					{
						for (int triangleJVert = 0; triangleJVert < 3; triangleJVert++)
						{
							const Vec3& vertexI = m_vertexes[tri.m_vertIndexes[triangleIVert]];
							const Vec3& vertexJ = m_vertexes[neighborTri.m_vertIndexes[triangleJVert]];

							if (vertexI == vertexJ)
							{
								sharedVerts++;
								if (sharedVerts == 1)
								{
									sharedVertA = vertexI;
								}
								else if (sharedVerts == 2)
								{
									sharedVertB = vertexI;
								}
							}
						}
					}

					if (sharedVerts != 2) 
					{
						ERROR_AND_DIE(Stringf("Error: Triangle %zu and neighbor %d share %d vertices (expected 2).\n", i, neighborID, sharedVerts));
					}

					bool isvalidEdge = false;
					for (int triangleVert = 0; triangleVert < 3; triangleVert++)
					{
						const Vec3& vertA = m_vertexes[tri.m_vertIndexes[triangleVert]];
						const Vec3& vertB = m_vertexes[tri.m_vertIndexes[(triangleVert + 1) % 3]];

						if ((vertA == sharedVertA && vertB == sharedVertB) ||
							(vertA == sharedVertB && vertB == sharedVertA))
						{
							isvalidEdge = true;
							break;
						}
					}

					if (!isvalidEdge)
					{
						ERROR_AND_DIE(Stringf("Error: Triangle %zu and neighbor %d do not share a valid edge.\n", i, neighborID));
					}

					break;
				}
			}

			if (!found) 
			{
				ERROR_AND_DIE(Stringf("Error: Triangle %zu points to neighbor %d, but the reverse connection is missing.\n", i, neighborID));
			}
		}
	}
	return true;
}
