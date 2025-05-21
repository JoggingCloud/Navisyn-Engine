#include "Engine/AI/Pathfinding/NavMeshPathfinding.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

extern RandomNumberGenerator g_rng;

NavMeshPathfinding::NavMeshPathfinding(NavMesh* navMesh)
	: m_navMesh(navMesh)
{
	InitializeNavMeshNodes(*navMesh);
}

void NavMeshPathfinding::InitializeNavMeshNodes(NavMesh const& navMesh)
{
	if (m_nodes.empty())
	{
		m_nodes.resize(navMesh.m_triangles.size());

		for (size_t i = 0; i < m_nodes.size(); i++)
		{
			m_nodes[i] = Node();
			m_nodes[i].m_position = Vec3::INVALID_POSITION;
			m_nodes[i].m_triangleIndex = static_cast<int>(i);
			m_nodes[i].m_totalgCost = std::numeric_limits<float>::max();
			m_nodes[i].m_fCost = std::numeric_limits<float>::max();
			m_nodes[i].m_parentTriangleIndex = -1;
			m_nodes[i].m_openPathGen = -1;
			m_nodes[i].m_closedPathGen = -1;
		}
	}
}

void NavMeshPathfinding::ComputeAStar(Vec3 startPoint, Vec3 goalPoint, std::vector<Vec3>& outPath)
{
	m_pathGen++;

	std::priority_queue<Node*, std::vector<Node*>, CompareNode> openList;

	int startNodeIndex = GetTriangleIndexFromPoint(startPoint);
	int goalNodeIndex = GetTriangleIndexFromPoint(goalPoint);

	if (startNodeIndex == -1) 
	{
		Vec3 fallbackStart;
		if (m_navMesh->FindNearestTriangle(startPoint, startNodeIndex, fallbackStart))
		{
			startPoint = fallbackStart;
		}
		else
		{
			return;
		}
	}
	else if (startNodeIndex >= m_navMesh->m_triangles.size())
	{
		return;
	}

	if (goalNodeIndex == -1)
	{
		return;
	}
	else if (goalNodeIndex >= m_navMesh->m_triangles.size())
	{
		return;
	}

	Node* startNode = &m_nodes[startNodeIndex];
	startNode->m_position = startPoint;
	startNode->m_triangleIndex = startNodeIndex;
	startNode->m_totalgCost = 0.f;
	startNode->m_fCost = GetDistanceBetweenPointsExact(startPoint, goalPoint);
	startNode->m_openPathGen = m_pathGen;
	startNode->m_closedPathGen = -1;

	openList.emplace(startNode);

	while (!openList.empty())
	{
		Node* currentNode = openList.top(); //  Get the node with the lowest fcost (highest priority/the best node to explore)
		openList.pop(); // // Remove it from the queue because it's about to explore it
		
		if (currentNode->m_closedPathGen == m_pathGen) continue; // if node has already been explored then no need to check it again 
		currentNode->m_closedPathGen = m_pathGen; // Once explored, we close that node so we don't revisit it again

		// If we found goal then compute path
		if (currentNode->m_triangleIndex == goalNodeIndex)
		{
			if (!outPath.empty()) outPath.clear();

			while (currentNode->m_triangleIndex != startNodeIndex)
			{
				outPath.emplace_back(currentNode->m_position);
				int parentIndex = currentNode->m_parentTriangleIndex;
				if (parentIndex < 0 || parentIndex >= m_nodes.size()) break;
				currentNode = &m_nodes[parentIndex];
			}
			outPath.insert(outPath.begin(), goalPoint);

			Prune(outPath);

			// Clear the open list once path is found by swapping the current open list with an empty one
			std::priority_queue<Node*, std::vector<Node*>, CompareNode> emptyList;
			std::swap(openList, emptyList);
			return;
		}

		// Global polygon lookup of current triangle index
		// Get triangleID based on current pointindex and pass the id into here
		const NavMeshTri& currentTriangle = m_navMesh->m_triangles[currentNode->m_triangleIndex]; 

		for (int neighborID : currentTriangle.m_neighborTriIndexes)
		{
			if (neighborID >= m_nodes.size() || neighborID < 0) continue; // Skip invalid neighbors
			Node* neighborNode = &m_nodes[neighborID];
			if (neighborNode->m_closedPathGen == m_pathGen) continue;

			Vec3 neighborPoint = GetEdgeIntersectionPoint(neighborID, currentNode->m_position, goalPoint);

			// Calculate costs
			float localgCost = GetDistanceBetweenPointsExact(neighborPoint, currentNode->m_position);
			float totalgCost = currentNode->m_totalgCost + localgCost;
			float hCost = GetDistanceBetweenPointsExact(neighborPoint, goalPoint);
			float fCost = totalgCost + hCost;

			if (fCost < neighborNode->m_fCost)
			{
				neighborNode->m_position = neighborPoint;
				neighborNode->m_triangleIndex = neighborID;
				neighborNode->m_totalgCost = totalgCost;
				neighborNode->m_fCost = fCost;
				neighborNode->m_parentTriangleIndex = currentNode->m_triangleIndex;

				if (neighborNode->m_openPathGen != m_pathGen)
				{
					neighborNode->m_openPathGen = m_pathGen;
					openList.emplace(neighborNode);
				}
			}
		}
	}

	outPath.clear();
}

void NavMeshPathfinding::Funnel(Vec3 startPoint, std::vector<Vec3>& constructedPath)
{
	if (constructedPath.empty()) return;

	std::vector<Vec3> smoothedPath;
	smoothedPath.emplace_back(startPoint);

	Vec3 apex = startPoint;
	Vec3 leftEdge, rightEdge;

	// Step through all edges in the constructed path
	for (size_t i = 0; i < constructedPath.size() - 1; i++)
	{
		Vec3 left = constructedPath[i];
		Vec3 right = constructedPath[i + 1];

		// First edge is always the funnel opening 
		if (i == 0)
		{
			leftEdge = left;
			rightEdge = right;
			continue;
		}

		// Use CrossProduct to check if the left edge is valid 
		Vec3 aLeft = rightEdge - apex;
		Vec3 bLeft = left - apex;
		if (CrossProduct3D(aLeft, bLeft).z <= 0.f)
		{
			if (apex != rightEdge)
			{
				smoothedPath.emplace_back(rightEdge);
				apex = rightEdge;
				leftEdge = left;
				rightEdge = left;
				continue;
			}
		}

		// Use CrossProduct to check if the right edge is valid
		Vec3 aRight = leftEdge - apex;
		Vec3 bRight = right - apex;
		if (CrossProduct3D(aRight, bRight).z <= 0.f)
		{
			if (apex != leftEdge)
			{
				smoothedPath.emplace_back(leftEdge);
				apex = leftEdge;
				rightEdge = right;
				leftEdge = right;
				continue;
			}
		}

		// Update the edges and then repeat the process until we reach the end of the path
		leftEdge = left;
		rightEdge = right;
	}

	// Add the last waypoint to the smoothedPath
	smoothedPath.emplace_back(constructedPath.back());
	constructedPath = smoothedPath;
}

void NavMeshPathfinding::Prune(std::vector<Vec3>& prunedPath)
{
	if (prunedPath.size() < 3) return; // No need to prune if we have only start and end

	std::vector<Vec3> path;
	path.reserve(prunedPath.size());
	path.emplace_back(prunedPath.front()); // Start point

	int currentIndex = 0;
	while (currentIndex < prunedPath.size() - 1)
	{
		int nextIndex = currentIndex + 1;

		// Find the farthest valid point that maintains line-of-sight
		for (int i = nextIndex + 1; i < prunedPath.size(); i++)
		{
			if (!HasLineOfSight(prunedPath[currentIndex], prunedPath[i])) break;
			nextIndex = i;
		}

		path.emplace_back(prunedPath[nextIndex]);
		currentIndex = nextIndex;
	}

	for (Vec3& point : prunedPath)
	{
		int triangleIndex = GetTriangleIndexFromPoint(point);
		if (triangleIndex != -1)
		{
			point.z = GetHeightOnTriangle(triangleIndex, point.x, point.y);
		}
	}

	prunedPath = std::move(path); // Move instead of copy for efficiency
}

void NavMeshPathfinding::ResamplePathToFollowNavMesh(std::vector<Vec3>& path)
{
	if (path.size() < 2) return; // No need to resample if the path is too short

	std::vector<Vec3> resampledPath;
	resampledPath.reserve(path.size() * 2); // Preallocate to reduce reallocations

	resampledPath.emplace_back(path.front()); // Always keep the start point

	float stepSize = GetAverageTriangleEdgeLength() * 0.5f; // Adjust step size

	for (size_t i = 0; i < path.size() - 1; i++)
	{
		Vec3 start = path[i];
		Vec3 end = path[i + 1];

		Vec3 direction = (end - start).GetNormalized();
		float totalDistance = GetDistanceBetweenPointsExact(start, end);

		for (float dist = stepSize; dist < totalDistance; dist += stepSize)
		{
			Vec3 samplePoint = start + direction * dist;
			int triangleIndex = GetTriangleIndexFromPoint(samplePoint);

			if (triangleIndex != -1) // Ensure sample point is inside NavMesh
			{
				samplePoint.z = GetHeightOnTriangle(triangleIndex, samplePoint.x, samplePoint.y);
				resampledPath.emplace_back(samplePoint);
			}
		}

		int triangleIndex = GetTriangleIndexFromPoint(end);
		if (triangleIndex != -1)
		{
			end.z = GetHeightOnTriangle(triangleIndex, end.x, end.y);
		}
		resampledPath.emplace_back(end); // Always include the original endpoint
	}

	path = std::move(resampledPath); // Move instead of copying
}

void NavMeshPathfinding::GroundCheck(Vec3 startPosition, float endZHeight)
{
	Vec3 rayStart = startPosition + Vec3(0.f, 0.f, endZHeight);
	Vec3 rayDir = Vec3(0.f, 0.f, -1.f);
	RaycastVsGroundResult result = RaycastVsNavNesh(rayStart, rayDir, FLT_MAX);
	if (result.m_didImpact)
	{
		float actorDistanceOffGround = fabs(startPosition.z - result.m_impactPos.z);
		if (actorDistanceOffGround > ALLOWED_HEIGHT_DEVIATION)
		{
			startPosition.z = Interpolate(startPosition.z, result.m_impactPos.z, 0.25f);
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------

bool NavMeshPathfinding::HasLineOfSight(const Vec3& startPoint, const Vec3& endPoint) const
{
	Vec3 direction = (endPoint - startPoint).GetNormalized();
	float distance = GetDistanceBetweenPointsExact(startPoint, endPoint);
	float stepSize = GetAverageTriangleEdgeLength() /** 0.1f*/; // multiply by 0.1 for small step size 

	for (float t = 0; t <= distance; t += stepSize)
	{
		Vec3 pointToCheck = startPoint + (direction * t);

		bool insideNavMesh = false;
		for (size_t i = 0; i < m_navMesh->m_triangles.size(); i++)
		{
			const NavMeshTri& triangle = m_navMesh->m_triangles[i];
			const Vec3& p0 = m_navMesh->m_vertexes[triangle.m_vertIndexes[0]];
			const Vec3& p1 = m_navMesh->m_vertexes[triangle.m_vertIndexes[1]];
			const Vec3& p2 = m_navMesh->m_vertexes[triangle.m_vertIndexes[2]];
			if (IsPointInsideTriangle3D(pointToCheck, p0, p1, p2))
			{
				insideNavMesh = true;
				break;
			}
		}

		if (!insideNavMesh) return false;
	}

	return true;
}

float NavMeshPathfinding::GetAverageTriangleEdgeLength() const
{
	float totalLength = 0.f;
	int edgeCount = 0;

	if (m_navMesh && !m_navMesh->m_triangles.empty())
	{
		for (size_t i = 0; i < m_navMesh->m_triangles.size(); i++)
		{
			const NavMeshTri& triangle = m_navMesh->m_triangles[i];
			const Vec3& p0 = m_navMesh->m_vertexes[triangle.m_vertIndexes[0]];
			const Vec3& p1 = m_navMesh->m_vertexes[triangle.m_vertIndexes[1]];
			const Vec3& p2 = m_navMesh->m_vertexes[triangle.m_vertIndexes[2]];

			totalLength += GetDistanceBetweenPointsExact(p0, p1);
			totalLength += GetDistanceBetweenPointsExact(p1, p2);
			totalLength += GetDistanceBetweenPointsExact(p2, p0);
			edgeCount += 3;
		}

		return (edgeCount > 0) ? totalLength / edgeCount : 1.f; // Avoid division by zero
	}
	return 0.f;
}

float NavMeshPathfinding::GetHeightOnTriangle(int triangleID, float x, float y)
{
	NavMeshTri* triangleMesh = m_navMesh->GetNavMeshTriangle(triangleID);
	if (!triangleMesh) return 0.f; // Default ground level if something goes wrong

	Vec3 v0 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[0]];
	Vec3 v1 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[1]];
	Vec3 v2 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[2]];

	return ComputeBarycentricHeight(x, y, v0, v1, v2);
}

float NavMeshPathfinding::GetHeightOnTriangle(const NavMeshTri& triangle, float x, float y)
{
	Vec3 v0 = m_navMesh->m_vertexes[triangle.m_vertIndexes[0]];
	Vec3 v1 = m_navMesh->m_vertexes[triangle.m_vertIndexes[1]];
	Vec3 v2 = m_navMesh->m_vertexes[triangle.m_vertIndexes[2]];

	return ComputeBarycentricHeight(x, y, v0, v1, v2);
}

float NavMeshPathfinding::GetHeightOnTriangle(Vec3 point)
{
	int triangleIndex = GetTriangleIndexFromPoint(point);
	if (triangleIndex == -1) return point.z;

	return GetHeightOnTriangle(triangleIndex, point.x, point.y);
}

float NavMeshPathfinding::ComputeBarycentricHeight(float x, float y, Vec3 v0, Vec3 v1, Vec3 v2)
{
	// Compute edge vectors
	Vec3 edge0 = v1 - v0;
	Vec3 edge1 = v2 - v0;

	// Compute normal of the triangle plane
	Vec3 normal = CrossProduct3D(edge0, edge1).GetNormalized();

	// Create a plane from the triangle
	float distance = DotProduct3D(normal, v0);
	Plane3D trianglePlane(normal, distance);

	// Solve for Z: z = (-Ax - By - D) / C
	if (fabs(normal.z) < 1e-6f)  // Prevent division by zero in near-vertical triangles
	{
		return v0.z;  // Default to v0’s height in degenerate cases
	}

	// Compute height (z) at the given (x, y) on the triangle plane
	Vec3 pointOnXYPlane(x, y, 0.f);
	Vec3 projected = GetNearestPointOnPlane3D(pointOnXYPlane, trianglePlane);

	return projected.z;
}

int NavMeshPathfinding::GetTriangleIndexFromPoint(Vec3 point) const
{
	for (size_t i = 0; i < m_navMesh->m_triangles.size(); i++)
	{
		const NavMeshTri& triangle = m_navMesh->m_triangles[i];
		const Vec3& p0 = m_navMesh->m_vertexes[triangle.m_vertIndexes[0]];
		const Vec3& p1 = m_navMesh->m_vertexes[triangle.m_vertIndexes[1]];
		const Vec3& p2 = m_navMesh->m_vertexes[triangle.m_vertIndexes[2]];

		if (IsPointInsideTriangle3D(point, p0, p1, p2))
		{
			return static_cast<int>(i);
		}
	}

	return -1;
}

Vec3 NavMeshPathfinding::GetEdgeIntersectionPoint(int neighborTriangleID, Vec3 currentPoint, Vec3 goalPoint)
{
	NavMeshTri* triangleMesh = m_navMesh->GetNavMeshTriangle(neighborTriangleID);
	if (!triangleMesh) return Vec3::ZERO;

	Vec3 v0 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[0]];
	Vec3 v1 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[1]];
	Vec3 v2 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[2]];

	std::vector<std::pair<Vec3, Vec3>> edges = { {v0, v1}, {v1, v2}, {v2, v0} };

	Vec3 bestIntersectionPoint = Vec3::ZERO;
	float closestDist = std::numeric_limits<float>::max();
	bool foundValidIntersection = false;

	for (const auto& edge : edges)
	{
		Vec3 intersectionPoint;
		if (DoLineSegmentIntersect3D(currentPoint, goalPoint, edge.first, edge.second, intersectionPoint))
		{
			float dist = GetDistance3D(currentPoint, intersectionPoint);
			if (dist < closestDist)
			{
				closestDist = dist;
				bestIntersectionPoint = intersectionPoint;
				bestIntersectionPoint.z = GetHeightOnTriangle(neighborTriangleID, bestIntersectionPoint.x, bestIntersectionPoint.y);
				foundValidIntersection = true;
			}
		}
	}

	if (!foundValidIntersection)
	{
		for (const auto& edge : edges)
		{
			Vec3 nearestPoint = GetNearestPointOnLineSegment3D(currentPoint, edge.first, edge.second);
			float dist = GetDistance3D(currentPoint, nearestPoint);
			if (dist < closestDist)
			{
				closestDist = dist;
				bestIntersectionPoint = nearestPoint;
				bestIntersectionPoint.z = GetHeightOnTriangle(neighborTriangleID, bestIntersectionPoint.x, bestIntersectionPoint.y);
			}
		}
	}

	return bestIntersectionPoint;
}

Vec3 NavMeshPathfinding::CalculateCentroid(NavMeshTri& triangleMesh)
{
	Vec3 v0 = m_navMesh->m_vertexes[triangleMesh.m_vertIndexes[0]];
	Vec3 v1 = m_navMesh->m_vertexes[triangleMesh.m_vertIndexes[1]];
	Vec3 v2 = m_navMesh->m_vertexes[triangleMesh.m_vertIndexes[2]];

	return (v0 + v1 + v2) / 3.f;
}

Vec3 NavMeshPathfinding::CalculateCentroid(int triangleID)
{
	NavMeshTri* triangleMesh = m_navMesh->GetNavMeshTriangle(triangleID);
	if (!triangleMesh) return Vec3::ZERO;

	Vec3 v0 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[0]];
	Vec3 v1 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[1]];
	Vec3 v2 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[2]];

	return (v0 + v1 + v2) / 3.f;
}

Vec3 NavMeshPathfinding::GetRandomPointWithinTriangleIndex(int triangleID)
{
	NavMeshTri* triangleMesh = m_navMesh->GetNavMeshTriangle(triangleID);
	if (!triangleMesh) return Vec3::ZERO;

	Vec3 v0 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[0]];
	Vec3 v1 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[1]];
	Vec3 v2 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[2]];

	// Generate random barycentric coordinates
	//g_rng.SetSeed(GetRandomSeedFromTime());
	float u = g_rng.SRollRandomFloatZeroToOne();
	float v = g_rng.SRollRandomFloatZeroToOne();

	// If u + v > 1, flip it to keep the point inside the triangle
	if (u + v > 1.0f)
	{
		u = 1.0f - u;
		v = 1.0f - v;
	}

	// Compute the random point using barycentric coordinates
	return (1.f - u - v) * v0 + u * v1 + v * v2;
}

Vec3 NavMeshPathfinding::GetImpactPointOnTriangle(Vec3& point, int triangleID)
{
	// Get any random point within the triangle bounds instead of getting the nearest point
	NavMeshTri* triangleMesh = m_navMesh->GetNavMeshTriangle(triangleID);
	if (!triangleMesh) return Vec3::ZERO;

	Vec3 v0 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[0]];
	Vec3 v1 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[1]];
	Vec3 v2 = m_navMesh->m_vertexes[triangleMesh->m_vertIndexes[2]];

	return GetNearestPointOnTriangle3D(point, v0, v1, v2);
}

RaycastVsGroundResult NavMeshPathfinding::RaycastVsNavNesh(const Vec3& start, const Vec3& direction, float maxDistance)
{
	RaycastVsGroundResult result;
	result.m_impactDist = maxDistance;

	Vec3 rayDir = direction.GetNormalized();

	// Loop through the triangles
	for (size_t i = 0; i < m_navMesh->m_triangles.size(); i++)
	{
		NavMeshTri& triangle = m_navMesh->m_triangles[i];
		Vec3& v0 = m_navMesh->m_vertexes[triangle.m_vertIndexes[0]];
		Vec3& v1 = m_navMesh->m_vertexes[triangle.m_vertIndexes[1]];
		Vec3& v2 = m_navMesh->m_vertexes[triangle.m_vertIndexes[2]];

		// Create bounding volume via sphere around each triangle
		Vec3 centroid = CalculateCentroid(triangle);

		// Set the sphere radius (rough estimate using the distance from centroid to a vertex)
		float sphereRadius = GetDistance3D(centroid, v0);

		// Raycast against the sphere first to filter out potential hits
		RaycastResult3D sphereHit;
		sphereHit = RaycastVsSphere(start, rayDir, maxDistance, centroid, sphereRadius);
		if (!sphereHit.m_didImpact) continue;

		// Raycast vs triangle's plane
		Vec3 edge0 = v1 - v0;
		Vec3 edge2 = v2 - v0;
		Vec3 normal = CrossProduct3D(edge0, edge2).GetNormalized();

		// Ensure the normal is pointing upwards (if applicable)
		if (DotProduct3D(normal, Vec3(0.f, 0.f, 1.f)) < 0.f)
		{
			normal = -normal; // Flip the normal
		}

		// Calculate the distance from the plane using a vertex
		float planeDistance = DotProduct3D(normal, v0);
		Plane3D trianglePlane = Plane3D(normal, planeDistance);

		// Cast ray against the triangle's plane
		RaycastResult3D planeHit = RaycastVsPlane3D(start, direction.GetNormalized(), maxDistance, trianglePlane);

		if (IsPointInsideTriangle3D(planeHit.m_impactPos, v0, v1, v2))
		{
			// Front of plane
			if (DotProduct3D(rayDir, normal) >= 0.f) continue; // skip the backside 

			// If the point is within the sphere (due to prior sphere check), we treat it as a hit
			result.m_didImpact = true;
			result.m_impactPos = planeHit.m_impactPos;
			result.m_impactNormal = planeHit.m_impactNormal;
			result.m_impactDist = planeHit.m_impactDist;
			break; // Exit the loop once we've found an impact
		}
	}

	DebugAddWorldArrow(start, start + (direction * maxDistance), 0.8f, 0.015f, 0.01f, 0.f, Rgba8::DARK_GRAY, Rgba8::DARK_GRAY, DebugRenderMode::ALWAYS);
	DebugAddWorldArrow(start, start + (direction * result.m_impactDist), 0.8f, 0.015f, 0.01f, 0.f, Rgba8::RED, Rgba8::RED, DebugRenderMode::ALWAYS);
	DebugAddWorldPoint(result.m_impactPos, 0.05f, 16, 0.f, Rgba8::DARK_ORANGE, Rgba8::DARK_ORANGE, DebugRenderMode::ALWAYS);
	DebugAddWorldArrow(result.m_impactPos, result.m_impactPos + (result.m_impactNormal * 0.3f), 0.8f, 0.015f, 0.01f, 0.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::ALWAYS);

	return result;
}

// ---------------------------------------------------------------------------------------------------------------------

void NavMeshPathfinding::TestAStarPathFinding(Vec3 startPoint, Vec3 goalPoint, std::vector<Vec3>& path)
{
	double timeBefore = GetCurrentTimeSeconds();
	ComputeAStar(startPoint, goalPoint, path);
	double timeAfter = GetCurrentTimeSeconds();

	float msElapsed = 1000.f * float(timeAfter - timeBefore);
	g_theConsole->AddLine(Rgba8::RED, Stringf("Generated a path of %i steps from triangle %i to triangle %i in %.02f ms", path.size(), startPoint, goalPoint, msElapsed));

	size_t pathSizeInBytes = path.size() * sizeof(size_t); // Approximate size of one path
	g_theConsole->AddLine(Rgba8::DARK_ORANGE, Stringf("The approximate size of one path is %zu bytes", pathSizeInBytes));
}