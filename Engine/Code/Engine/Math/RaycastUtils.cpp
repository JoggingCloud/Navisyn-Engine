#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Mat44.hpp"

RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius)
{
	RaycastResult2D result;

	// Get i Basis
	Vec2 const& iBasis = fwdNormal;

	// Calculate the j basis of the forward normal
	Vec2 jBasis = iBasis.GetRotated90Degrees();

	// Calculate vector from start position to disc Center 
	Vec2 displacementToCenter = discCenter - startPos;

	// Calculate the center projection of j basis using dot product
	float distanceToCenterDiscProjectionJ = DotProduct2D(displacementToCenter, jBasis);

	// Check if the closest point is outside the disc 
	if (distanceToCenterDiscProjectionJ > discRadius || distanceToCenterDiscProjectionJ < -discRadius)
	{
		result.m_didImpact = false;
		return result;
	}

	// Calculate the center project of i basis using dot product 
	float distanceToCenterDiscProjectionI = DotProduct2D(displacementToCenter, iBasis);

	// Check if disc is entirely before or after ray
	if (distanceToCenterDiscProjectionI < -discRadius || distanceToCenterDiscProjectionI > maxDist + discRadius)
	{
		result.m_didImpact = false;
		return result;
	}

	// Check if the length of the vector is less than the radius of the disc
	if (displacementToCenter.GetLength() < discRadius)
	{
		result.m_impactPos = startPos;
		result.m_impactDist = 0.f;
		result.m_didImpact = true;
		result.m_impactNormal = -iBasis;
		return result;
	}

	// Solving for a squared. This is Pythagorean theorem 
	float  a = sqrtf((discRadius * discRadius) - (distanceToCenterDiscProjectionJ * distanceToCenterDiscProjectionJ));

	// Calculate the impact distance
	float impactDistance = distanceToCenterDiscProjectionI - a;

	// Check if the disc is behind the ray & check if the ray is too short, there's no intersection
	if (impactDistance < 0.f || impactDistance > maxDist)
	{
		result.m_didImpact = false;
		return result;
	}

	// Calculate the impact position
	Vec2 impactPos = startPos + (iBasis * impactDistance);

 	// Normalize the impact direction
 	Vec2 impactNormal = (impactPos - discCenter).GetNormalized();

	result.m_didImpact = true;
	result.m_impactPos = impactPos;
	result.m_impactDist = impactDistance;
	result.m_impactNormal = impactNormal;

	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = maxDist;

	return result;
}

RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float raycastLength, AABB2 const& box)
{
	RaycastResult2D result;

	if (IsPointInsideAABB2D(startPos, box))
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		result.m_impactNormal = -fwdNormal;
		result.m_impactDist = 0.f;
		return result;
	}

	Vec2 endPos = startPos + fwdNormal * raycastLength;

	float oneOverRangeX = 1.f / fwdNormal.x;
	float oneOverRangeY = 1.f / fwdNormal.y;

	float tMinX = (box.m_mins.x - startPos.x) * oneOverRangeX;
	float tMaxX = (box.m_maxs.x - startPos.x) * oneOverRangeX;

	float tMinY = (box.m_mins.y - startPos.y) * oneOverRangeY;
	float tMaxY = (box.m_maxs.y - startPos.y) * oneOverRangeY;
	
	FloatRange rangeX(tMinX, tMaxX);
	FloatRange rangeY(tMinY, tMaxY);
	
	if (tMinX > tMaxX)
	{
		rangeX = FloatRange(tMaxX, tMinX);
	}

	if (tMinY > tMaxY)
	{
		rangeY = FloatRange(tMaxY, tMinY);
	}

	if (!rangeX.IsOverlapingWith(rangeY))
	{
		result.m_didImpact = false;
		return result;
	}
	
	float t = FLT_MAX;

	if (rangeY.IsOnRange(tMinX))
	{
		if (tMinX < t)
		{
			t = tMinX;
		}
	}

	if (rangeX.IsOnRange(tMinY))
	{
		if (tMinY < t)
		{
			t = tMinY;
		}

	}

	if (rangeY.IsOnRange(tMaxX))
	{
		if (tMaxX < t)
		{
			t = tMaxX;
		}
	}

	if (rangeX.IsOnRange(tMaxY))
	{
		if (tMaxY < t)
		{
			t = tMaxY;
		}
	}

	float impactDistance = static_cast<float>(fabs(t));

	Vec2 impactPos = startPos + fwdNormal * impactDistance;

	if (t < 0.f || t > raycastLength)
	{
		result.m_didImpact = false;
		return result;
	}

	Vec2 impactNormal;
	
	if (t == tMinX) 
	{
		impactNormal = Vec2(-1.f, 0.f); // Left face
	}
	else if (t == tMaxX) 
	{
		impactNormal = Vec2(1.f, 0.f); // Right face
	}
	else if (t == tMinY) 
	{
		impactNormal = Vec2(0.f, -1.f); // Bottom face
	}
	else if (t == tMaxY) 
	{
		impactNormal = Vec2(0.f, 1.f); // Top face
	}

	result.m_didImpact = true;
	result.m_impactPos = impactPos;
	result.m_impactDist = impactDistance;
	result.m_impactNormal = impactNormal;

	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = raycastLength;

	return result;
}

RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 fwdNormal, float raycastLength, Vec2 lineSegmentStart, Vec2 lineSegmentEnd)
{
	RaycastResult2D result;

	// Get i Basis
	Vec2 const& iBasis = fwdNormal;

	// Calculate the j basis of the forward normal
	Vec2 jBasis = iBasis.GetRotated90Degrees();

	Vec2 raycastToStart = lineSegmentStart - startPos;
	Vec2 raycastToEnd = lineSegmentEnd - startPos;

	float raycastToStartDisplacement = DotProduct2D(raycastToStart, jBasis);
	float raycastToEndDisplacement = DotProduct2D(raycastToEnd, jBasis);

	if (raycastToEndDisplacement >= 0 && raycastToStartDisplacement >= 0) // segment is to the left 
	{
		result.m_didImpact = false;
		return result;
	}

	if (raycastToEndDisplacement <= 0 && raycastToStartDisplacement <= 0) // segment is to the right 
	{
		result.m_didImpact = false;
		return result;
	}

	// Calculate impact distance
	float t = raycastToStartDisplacement / (raycastToStartDisplacement - raycastToEndDisplacement);

	Vec2 impactPositon = lineSegmentStart + t * (lineSegmentEnd - lineSegmentStart);
	Vec2 displacementI = impactPositon - startPos;

	float impactDistance = DotProduct2D(displacementI, fwdNormal);
	if (impactDistance >= raycastLength)
	{
		result.m_didImpact = false;
		return result;
	}

	if (impactDistance <= 0)
	{
		result.m_didImpact = false;
		return result;
	}

	Vec2 lineSegmentDistance = lineSegmentEnd - lineSegmentStart;
	Vec2 impactNormal = lineSegmentDistance.GetRotated90Degrees();
	if (raycastToStartDisplacement > 0)
	{
		impactNormal.x *= -1.f;
		impactNormal.y *= -1.f;
	}
	impactNormal.Normalize();

	result.m_didImpact = true;
	result.m_impactPos = impactPositon;
	result.m_impactDist = impactDistance;
	result.m_impactNormal = impactNormal;

	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = raycastLength;

	return result;
}

RaycastResult2D RaycastVsPlane2D(Vec2 startPos, Vec2 fwdNormal, float raycastLength, Plane2D const& plane)
{
	RaycastResult2D result;
	result.m_rayStartPos = startPos;

	float startAltitude = plane.GetAltitudeOfPoint(startPos);
	Vec2 endPos = startPos + fwdNormal * raycastLength;
	float endAltitude = plane.GetAltitudeOfPoint(endPos);

	if (startAltitude * endAltitude > 0.f)
	{
		result.m_didImpact = false;
		return result;
	}

	float t = -startAltitude / (endAltitude - startAltitude) * raycastLength;

	result.m_didImpact = true;
	result.m_impactPos = startPos + fwdNormal * t;
	result.m_impactNormal = plane.m_normal;
	result.m_impactDist = t;

	if (startAltitude < 0.f)
	{
		result.m_impactNormal = -plane.m_normal;
	}

	return result;
}

RaycastResult2D RaycastVsConvexHull2D(Vec2 startPos, Vec2 fwdNormal, float raycastLength, ConvexHull2D const& convexHull)
{
	// Early out 
	if (IsPointInsideConvexHull2D(startPos, convexHull))
	{
		RaycastResult2D result;
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -fwdNormal;
		result.m_impactPos = startPos;
		result.m_rayFwdNormal = fwdNormal;
		result.m_rayMaxLength = raycastLength;
		result.m_rayStartPos = startPos;
		return result;
	}

	std::vector<Plane2D> potentialEntryPlanes, potentialExitPlanes;

	for (size_t i = 0; i < convexHull.m_boundingplanes.size(); i++)
	{
		Plane2D currentPlane = convexHull.m_boundingplanes[i];

		if (currentPlane.IsPointInFrontOfPlane(startPos))
		{
			potentialEntryPlanes.emplace_back(currentPlane);
		}
		else
		{
			potentialExitPlanes.emplace_back(currentPlane);
		}
	}

	std::vector<Vec2> potentialEntryImpactPoints, potentialExitImpactPoints;

	std::vector<Plane2D> impactedPlanes;
	for (size_t i = 0; i < potentialEntryPlanes.size(); i++)
	{
		RaycastResult2D result = RaycastVsPlane2D(startPos, fwdNormal, raycastLength, potentialEntryPlanes[i]);
		if (result.m_didImpact)
		{
			potentialEntryImpactPoints.emplace_back(result.m_impactPos);
			impactedPlanes.emplace_back(potentialEntryPlanes[i]);
		}

	}

	for (size_t i = 0; i < potentialExitPlanes.size(); i++)
	{
		RaycastResult2D result = RaycastVsPlane2D(startPos, fwdNormal, raycastLength, potentialExitPlanes[i]);
		if (result.m_didImpact)
		{
			potentialExitImpactPoints.emplace_back(result.m_impactPos);
		}
	}

	float latestEntryPoint = 0.f;
	float earliestExitPoint = raycastLength;

	int latestEntryImpactPointIndex = -1;
	int earliestExitImpactPointIndex = -1;

	for (size_t i = 0; i < potentialEntryImpactPoints.size(); i++)
	{
		float currentDistance = GetDistance2D(potentialEntryImpactPoints[i], startPos);
		if (latestEntryPoint < currentDistance)
		{
			latestEntryPoint = currentDistance;
			latestEntryImpactPointIndex = static_cast<int>(i);
		}
	}

	for (size_t i = 0; i < potentialExitImpactPoints.size(); i++)
	{
		float currentDistance = GetDistance2D(potentialExitImpactPoints[i], startPos);
		if (earliestExitPoint > currentDistance)
		{
			earliestExitPoint = currentDistance;
			earliestExitImpactPointIndex = static_cast<int>(i);
		}
	}

	if (latestEntryPoint > earliestExitPoint)
	{
		RaycastResult2D result;
		result.m_didImpact = false;
		result.m_impactDist = 0.f;
		result.m_impactNormal = Vec2::ZERO;
		result.m_impactPos = startPos;
		result.m_rayFwdNormal = fwdNormal;
		result.m_rayMaxLength = raycastLength;
		result.m_rayStartPos = startPos;
		return result;
	}

	if (latestEntryImpactPointIndex != -1)
	{
		if (earliestExitImpactPointIndex != -1)
		{
			Vec2 midPoint = ( potentialEntryImpactPoints[latestEntryImpactPointIndex] + potentialExitImpactPoints[earliestExitImpactPointIndex] ) * 0.5f;
			if (IsPointInsideConvexHull2D(midPoint, convexHull))
			{
				RaycastResult2D result;
				result.m_didImpact = true;
				result.m_impactDist = GetDistance2D(potentialEntryImpactPoints[latestEntryImpactPointIndex], startPos);
				result.m_impactNormal = impactedPlanes[latestEntryImpactPointIndex].m_normal;
				result.m_impactPos = potentialEntryImpactPoints[latestEntryImpactPointIndex];
				result.m_rayFwdNormal = fwdNormal;
				result.m_rayMaxLength = raycastLength;
				result.m_rayStartPos = startPos;
				return result;
			}
		}
		else
		{
			Vec2 rayEndPoint = startPos + (fwdNormal * raycastLength);
			Vec2 midPoint = ( potentialEntryImpactPoints[latestEntryImpactPointIndex] + rayEndPoint ) * 0.5f;
			if (IsPointInsideConvexHull2D(midPoint, convexHull))
			{
				RaycastResult2D result;
				result.m_didImpact = true;
				result.m_impactDist = GetDistance2D(potentialEntryImpactPoints[latestEntryImpactPointIndex], startPos);
				result.m_impactNormal = impactedPlanes[latestEntryImpactPointIndex].m_normal;
				result.m_impactPos = potentialEntryImpactPoints[latestEntryImpactPointIndex];
				result.m_rayFwdNormal = fwdNormal;
				result.m_rayMaxLength = raycastLength;
				result.m_rayStartPos = startPos;
				return result;
			}
		}
	}
	return RaycastResult2D();
}

RaycastResult3D RaycastVsAll(Vec3 startPos, Vec3 fwdNormal, float raycastLength, AABB3 box, OBB3 orientedBox, Vec3 sphereCenter, float sphereRadius, Vec2 cylinderCenter, FloatRange cylinderZRange, float cylinderRadius)
{
	RaycastResult3D result;

	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = raycastLength;

	float unitPerStep = 0.1f;
	Vec3 position = startPos;

	for (float t = 0; t <= raycastLength; t += unitPerStep)
	{
		position = startPos + (fwdNormal * t);

		if (IsPointInsideAABB3D(position, box))
		{
			result.m_didImpact = true;
			result.m_impactPos = position;
			result.m_impactDist = t;
			result.m_impactNormal = -fwdNormal;
			return result;
		}

		if (IsPointInsideOBB3D(position, orientedBox))
		{
			result.m_didImpact = true;
			result.m_impactPos = position;
			result.m_impactDist = t;
			result.m_impactNormal = -fwdNormal;
			return result;
		}

		if (IsPointInsideSphere(position, sphereCenter, sphereRadius))
		{
			result.m_didImpact = true;
			result.m_impactPos = position;
			result.m_impactDist = t;
			result.m_impactNormal = -fwdNormal;
			return result;
		}

		if (IsPointInsideZCylinder(position, cylinderCenter, cylinderZRange, cylinderRadius))
		{
			result.m_didImpact = true;
			result.m_impactPos = position;
			result.m_impactDist = t;
			result.m_impactNormal = -fwdNormal;
			return result;
		}
	}

	return result;
}

RaycastResult3D RaycastVsAABB3D(Vec3 startPos, Vec3 fwdNormal, float raycastLength, AABB3 box)
{
	RaycastResult3D result;

	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = raycastLength;

	Vec3 endPos = startPos + (fwdNormal * raycastLength);
	AABB3 rayBox(startPos, startPos);
	rayBox.StretchToIncludePoint(endPos);
	if (!DoAABB3DsOverlap(rayBox, box))
	{
		result.m_didImpact = false;
		return result;
	}

	if (IsPointInsideAABB3D(startPos, box))
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -fwdNormal;
		return result;
	}
	FloatRange TxRange;
	FloatRange TyRange;
	FloatRange TzRange;
	bool xFlip = false;
	bool yFlip = false;
	bool zFlip = false;

	float oneOverRangeX = 1.f / (endPos.x - startPos.x);
	TxRange.m_min = (box.m_mins.x - startPos.x) * oneOverRangeX;
	TxRange.m_max = (box.m_maxs.x - startPos.x) * oneOverRangeX;

	if (TxRange.m_min > TxRange.m_max)
	{
		float temp = TxRange.m_min;
		TxRange.m_min = TxRange.m_max;
		TxRange.m_max = temp;
		xFlip = true;
	}

	float oneOverRangeY = 1.f / (endPos.y - startPos.y);
	TyRange.m_min = (box.m_mins.y - startPos.y) * oneOverRangeY;
	TyRange.m_max = (box.m_maxs.y - startPos.y) * oneOverRangeY;

	if (TyRange.m_min > TyRange.m_max)
	{
		float temp = TyRange.m_min;
		TyRange.m_min = TyRange.m_max;
		TyRange.m_max = temp;
		yFlip = true;
	}

	float oneOverRangeZ = 1.f / (endPos.z - startPos.z);
	TzRange.m_min = (box.m_mins.z - startPos.z) * oneOverRangeZ;
	TzRange.m_max = (box.m_maxs.z - startPos.z) * oneOverRangeZ;

	if (TzRange.m_min > TzRange.m_max)
	{
		float temp = TzRange.m_min;
		TzRange.m_min = TzRange.m_max;
		TzRange.m_max = temp;
		zFlip = true;
	}

	FloatRange OverlapRange;
	bool XOverlapY = TxRange.IsOverlapingWith(TyRange);
	bool XOverlapZ = TxRange.IsOverlapingWith(TzRange);
	bool YOverlapZ = TyRange.IsOverlapingWith(TzRange);
	bool isRayHitHorizontally = false;
	bool isRaySideFace = false;

	if (XOverlapY && XOverlapZ && YOverlapZ)
	{
		if (TxRange.m_min > TyRange.m_min)
		{
			OverlapRange.m_min = TxRange.m_min;
			isRayHitHorizontally = true;
			isRaySideFace = false;
		}
		else
		{
			OverlapRange.m_min = TyRange.m_min;
			isRayHitHorizontally = true;
			isRaySideFace = true;
		}

		if (OverlapRange.m_min < TzRange.m_min)
		{
			OverlapRange.m_min = TzRange.m_min;
			isRayHitHorizontally = false;
		}

		if (TxRange.m_max > TyRange.m_max)
		{
			OverlapRange.m_max = TyRange.m_max;
		}
		else
		{
			OverlapRange.m_max = TxRange.m_max;
		}

		if (OverlapRange.m_max > TzRange.m_max)
		{
			OverlapRange.m_max = TzRange.m_max;
		}

		result.m_didImpact = true;
		result.m_impactDist = raycastLength * OverlapRange.m_min;
		result.m_impactPos = startPos + (fwdNormal * result.m_impactDist);

		if (isRayHitHorizontally)
		{
			if (isRaySideFace)
			{
				Vec3 impactToMaxZ = Vec3(result.m_impactPos.x, result.m_impactPos.y, box.m_maxs.z) - result.m_impactPos;
				Vec3 impactToMaxX = Vec3(box.m_maxs.x, result.m_impactPos.y, result.m_impactPos.z) - result.m_impactPos;
				Vec3 impactToMinX = Vec3(box.m_mins.x, result.m_impactPos.y, result.m_impactPos.z) - result.m_impactPos;
				if (yFlip)
				{
					result.m_impactNormal = CrossProduct3D(impactToMaxZ, impactToMaxX);
				}
				else
				{
					result.m_impactNormal = CrossProduct3D(impactToMaxZ, impactToMinX);
				}
			}
			else
			{
				Vec3 impactToMaxZ = Vec3(result.m_impactPos.x, box.m_mins.y, result.m_impactPos.z) - result.m_impactPos;
				Vec3 impactToMaxY = Vec3(result.m_impactPos.x, box.m_maxs.y, result.m_impactPos.z) - result.m_impactPos;
				Vec3 impactToMinY = Vec3(result.m_impactPos.x, result.m_impactPos.y, box.m_maxs.z) - result.m_impactPos;
				if (xFlip)
				{
					result.m_impactNormal = CrossProduct3D(impactToMaxZ, impactToMinY);
				}
				else
				{
					result.m_impactNormal = CrossProduct3D(impactToMaxZ, impactToMaxY);
				}
			}
		}
		else
		{
			Vec3 impactToMaxY = Vec3(result.m_impactPos.x, box.m_maxs.y, result.m_impactPos.z) - result.m_impactPos;
			Vec3 impactToMinX = Vec3(box.m_mins.x, result.m_impactPos.y, result.m_impactPos.z) - result.m_impactPos;
			Vec3 impactToMaxX = Vec3(box.m_maxs.x, result.m_impactPos.y, result.m_impactPos.z) - result.m_impactPos;
			if (zFlip)
			{
				result.m_impactNormal = CrossProduct3D(impactToMaxY, impactToMinX);
			}
			else
			{
				result.m_impactNormal = CrossProduct3D(impactToMaxY, impactToMaxX);
			}
		}
		result.m_impactNormal.Normalize();
	}
	else
	{
		result.m_didImpact = true;
		return result;
	}

	return result;
}

RaycastResult3D RaycastVsSphere(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Vec3 sphereCenter, float sphereRadius)
{
	RaycastResult3D result;

	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = raycastLength;

	if (IsPointInsideSphere(startPos, sphereCenter, sphereRadius))
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -fwdNormal;
		return result;
	}

	Vec3 toCenter = sphereCenter - startPos;
	float SCiLength = DotProduct3D(toCenter, fwdNormal);
	Vec3 SCi = fwdNormal * SCiLength;
	Vec3 SCjk = toCenter - SCi;
	float SCjkLengthSquared = SCjk.GetLengthSquared();
	if (SCjkLengthSquared >= sphereRadius * sphereRadius)
	{
		result.m_didImpact = false;
		return result;
	}

	result.m_didImpact = true;
	float a = sqrtf((sphereRadius * sphereRadius) - SCjkLengthSquared);
	result.m_impactDist = SCiLength - a;
	if (result.m_impactDist >= raycastLength || result.m_impactDist <= 0.f)
	{
		result.m_didImpact = false;
		return result;
	}
	result.m_impactPos = startPos + (fwdNormal * result.m_impactDist);
	result.m_impactNormal = (startPos - sphereCenter).GetNormalized();
	return result;
}

RaycastResult3D RaycastVsHemisphere(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Vec3 hemisphereCenter, Vec3 hemisphereNormal, float hemisphereRadius)
{
	RaycastResult3D result;

	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = raycastLength;

	if (IsPointInsideHemisphere(startPos, hemisphereCenter, hemisphereNormal, hemisphereRadius))
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -fwdNormal;
		return result;
	}

	Vec3 toCenter = hemisphereCenter - startPos;
	float SCiLength = DotProduct3D(toCenter, fwdNormal);
	Vec3 SCi = fwdNormal * SCiLength;
	Vec3 SCjk = toCenter - SCi;
	float SCjkLengthSquared = SCjk.GetLengthSquared();
	if (SCjkLengthSquared >= hemisphereRadius * hemisphereRadius)
	{
		result.m_didImpact = false;
		return result;
	}

	float a = sqrtf((hemisphereRadius * hemisphereRadius) - SCjkLengthSquared);
	result.m_impactDist = SCiLength - a;
	if (result.m_impactDist >= raycastLength || result.m_impactDist <= 0.f)
	{
		result.m_didImpact = false;
		return result;
	}
	Vec3 toImpact = result.m_impactPos - hemisphereCenter;
	if (DotProduct3D(toImpact, hemisphereNormal) < 0.f)
	{
		result.m_didImpact = false;
		return result;
	}

	result.m_didImpact = true;
	result.m_impactNormal = (result.m_impactPos - hemisphereCenter).GetNormalized();
	return result;
}

RaycastResult3D RaycastVsZCylinder(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius)
{
	RaycastResult3D result;

	// Early out for if we start inside the cylinder
	if (IsPointInsideZCylinder(startPos, centerXY, minMaxZ, radius))
	{
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -fwdNormal;
		return result;
	}

	Vec2 startPosXY = Vec2(startPos.x, startPos.y); // Project the start position onto the XY plane
	Vec2 fwdNormalXY = Vec2(fwdNormal.x, fwdNormal.y).GetNormalized(); // Project and normalize the forward normal onto the XY plane

	Vec2 displacementToCenter = centerXY - startPosXY; // Compute the displacement from start position to cylinder center
	Vec2 const& iBasis = fwdNormalXY; // The forward direction of the ray on the XY plane
	Vec2 jbasis = iBasis.GetRotated90Degrees(); // Perpendicular to the forward direction on the XY plane
	float latitude = DotProduct2D(displacementToCenter, jbasis); // Latitude from the ray to the cylinder axis

	// Checks for if latitude is two far to the left or two far to the right of the disc radius
	if (fabsf(latitude) >= radius)
	{
		result.m_didImpact = false;
		return result;
	}

	// Check the z
	FloatRange zRange;
	bool zFlip = false;
	Vec3 endPos = startPos + (fwdNormal * raycastLength);
	float endOverZRange = 1.f / (endPos.z - startPos.z);
	zRange.m_min = endOverZRange * (minMaxZ.m_min - startPos.z);
	zRange.m_max = endOverZRange * (minMaxZ.m_max - startPos.z);

	if (zRange.m_min > zRange.m_max)
	{
		float temp = zRange.m_min;
		zRange.m_min = zRange.m_max;
		zRange.m_max = temp;
		zFlip = true;
	}

	// Calculate the body of the cylinder
	float a = sqrtf((radius * radius) - (latitude * latitude));
	float SCi = DotProduct2D(displacementToCenter, iBasis);
	float impactInXYDist = SCi - a;
	float impactOutXYDist = SCi + a;

	FloatRange tRange;
	float oneOverTRange = 1.f / (Vec2(endPos.x, endPos.y) - Vec2(startPos.x, startPos.y)).GetLength();
	tRange.m_min = impactInXYDist * oneOverTRange;
	tRange.m_max = impactOutXYDist * oneOverTRange;

	if (tRange.IsOverlapingWith(zRange))
	{
		FloatRange overlapRange;
		bool isHitOnCap = false;
		
		if (zRange.m_min > tRange.m_min)
		{
			overlapRange.m_min = zRange.m_min;
			isHitOnCap = true;
		}
		else
		{
			overlapRange.m_min = tRange.m_min;
		}

		if (zRange.m_max < tRange.m_max)
		{
			overlapRange.m_max = zRange.m_max;
		}
		else
		{
			overlapRange.m_max = tRange.m_max;
		}

		result.m_didImpact = true;
		result.m_impactDist = raycastLength * overlapRange.m_min;

		if (result.m_impactDist > raycastLength || result.m_impactDist < 0.f)
		{
			result.m_didImpact = false;
			return result;
		}

		result.m_impactPos = startPos + (fwdNormal * result.m_impactDist);

		if (isHitOnCap)
		{
			if (zFlip)
			{
				result.m_impactNormal = Vec3(0.f, 0.f, 1.f);
			}
			else
			{
				result.m_impactNormal = Vec3(0.f, 0.f, -1.f);
			}
		}
		else
		{
			result.m_impactNormal = (result.m_impactPos - Vec3(centerXY.x, centerXY.y, result.m_impactPos.z)).GetNormalized();
		}
	}

	result.m_rayStartPos = startPos;
	result.m_rayFwdNormal = fwdNormal;
	result.m_rayMaxLength = raycastLength;

	return result;
}

RaycastResult3D RaycastVsZCapsule(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Capsule3 capsule, float capsuleRadius)
{
	RaycastResult3D result;
	result.m_didImpact = false;
	result.m_impactDist = raycastLength;

	Vec2 centerXY = Vec2((capsule.m_start.x + capsule.m_end.x) * 0.5f, (capsule.m_start.y + capsule.m_end.y) * 0.5f); // Get the capsule center position
	
	// Raycast vs cylinder, bottom, top sphere
	RaycastResult3D resultCylinder = RaycastVsZCylinder(startPos, fwdNormal, raycastLength, centerXY, FloatRange(capsule.m_start.z, capsule.m_end.z), capsuleRadius);

	RaycastResult3D resultBottomSphere = RaycastVsSphere(startPos, fwdNormal, raycastLength, capsule.m_start, capsuleRadius);

	RaycastResult3D resultTopSphere = RaycastVsSphere(startPos, fwdNormal, raycastLength, capsule.m_end, capsuleRadius);

	// Check if we hit the bottom hemisphere, and if it's closer than any hit so far
	if (resultBottomSphere.m_didImpact && resultBottomSphere.m_impactDist <= result.m_impactDist)
	{
		result = resultBottomSphere;
	}

	// Check if we hit the top hemisphere, and if it's closer than any hit so far
	if (resultTopSphere.m_didImpact && resultTopSphere.m_impactDist <= result.m_impactDist)
	{
		result = resultTopSphere;
	}

	// Check if we hit the cylinder
	if (resultCylinder.m_didImpact && resultCylinder.m_impactDist <= result.m_impactDist)
	{
		result = resultCylinder;
	}

	return result;
}

RaycastResult3D RaycastVsOBB3D(Vec3 startPos, Vec3 fwdNormal, float raycastLength, OBB3 box)
{
	Mat44 obbLocalToWorld;
	obbLocalToWorld.SetIJK3D(box.m_iBasisNormal, box.m_jBasisNormal, box.m_kBasisNormal);
	obbLocalToWorld.SetTranslation3D(box.m_center);

	Mat44 obbWorldToLocal = obbLocalToWorld.GetOrthonormalInverse();

	Vec3 localStartPos = obbWorldToLocal.TransformPosition3D(startPos);
	Vec3 localFwdNormal = obbWorldToLocal.TransformVectorQuantity3D(fwdNormal);

	AABB3 localABB3(-box.m_halfDimensions, box.m_halfDimensions);

	RaycastResult3D result = RaycastVsAABB3D(localStartPos, localFwdNormal, raycastLength, localABB3);

	result.m_impactPos = obbLocalToWorld.TransformPosition3D(result.m_impactPos);
	result.m_impactNormal = obbLocalToWorld.TransformVectorQuantity3D(result.m_impactNormal);
	result.m_rayStartPos = startPos;
	result.m_rayFwdNormal = fwdNormal;

	return result;
}

RaycastResult3D RaycastVsPlane3D(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Plane3D plane)
{
	RaycastResult3D result;
	result.m_rayStartPos = startPos;

	float startAltitude = plane.GetAltitudeOfPoint(startPos);
	Vec3 endPos = startPos + fwdNormal * raycastLength;
	float endAltitude = plane.GetAltitudeOfPoint(endPos);

	if (startAltitude * endAltitude > 0.f)
	{
		result.m_didImpact = false;
		return result;
	}

	float t = -startAltitude / (endAltitude - startAltitude) * raycastLength;

	result.m_didImpact = true;
	result.m_impactPos = startPos + fwdNormal * t;
	result.m_impactNormal = plane.m_normal;
	result.m_impactDist = t;

	if (startAltitude < 0.f)
	{
		result.m_impactNormal = -plane.m_normal;
	}

	return result;
}

BoxCastResult3D BoxCastVsAABB3D(Vec3 startPos, Vec3 fwdNormal, float raycastLength, AABB3 const& box, Vec3 const& boxHalfExtents)
{
	BoxCastResult3D result;

	result.m_boxFwdNormal = fwdNormal;
	result.m_boxStartPos = startPos;
	result.m_maxBoxLength = raycastLength;
	result.m_boxHalfExtents = boxHalfExtents;

	Vec3 endPos = startPos + (fwdNormal * raycastLength);

	// Expand the AABB by the half extents of the box
	AABB3 expandedBox = box;
	expandedBox.StretchToIncludePoint(box.m_mins - boxHalfExtents);
	expandedBox.StretchToIncludePoint(box.m_maxs + boxHalfExtents);

	// Check if the starting or ending position is inside the expanded box
	if (expandedBox.IsPointInside(startPos) || expandedBox.IsPointInside(endPos))
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = startPos;
		result.m_impactNormal = -fwdNormal;
		return result;
	}

	FloatRange TxRange;
	FloatRange TyRange;
	FloatRange TzRange;
	bool xFlip = false;
	bool yFlip = false;
	bool zFlip = false;

	float oneOverRangeX = 1.f / (endPos.x - startPos.x);
	TxRange.m_min = (expandedBox.m_mins.x - startPos.x) * oneOverRangeX;
	TxRange.m_max = (expandedBox.m_maxs.x - startPos.x) * oneOverRangeX;

	if (TxRange.m_min > TxRange.m_max)
	{
		float temp = TxRange.m_min;
		TxRange.m_min = TxRange.m_max;
		TxRange.m_max = temp;
		xFlip = true;
	}

	float oneOverRangeY = 1.f / (endPos.y - startPos.y);
	TyRange.m_min = (expandedBox.m_mins.y - startPos.y) * oneOverRangeY;
	TyRange.m_max = (expandedBox.m_maxs.y - startPos.y) * oneOverRangeY;

	if (TyRange.m_min > TyRange.m_max)
	{
		float temp = TyRange.m_min;
		TyRange.m_min = TyRange.m_max;
		TyRange.m_max = temp;
		yFlip = true;
	}

	float oneOverRangeZ = 1.f / (endPos.z - startPos.z);
	TzRange.m_min = (expandedBox.m_mins.z - startPos.z) * oneOverRangeZ;
	TzRange.m_max = (expandedBox.m_maxs.z - startPos.z) * oneOverRangeZ;

	if (TzRange.m_min > TzRange.m_max)
	{
		float temp = TzRange.m_min;
		TzRange.m_min = TzRange.m_max;
		TzRange.m_max = temp;
		zFlip = true;
	}

	FloatRange OverlapRange;
	bool XOverlapY = TxRange.IsOverlapingWith(TyRange);
	bool XOverlapZ = TxRange.IsOverlapingWith(TzRange);
	bool YOverlapZ = TyRange.IsOverlapingWith(TzRange);
	bool isRayHitHorizontally = false;
	bool isRaySideFace = false;

	if (XOverlapY && XOverlapZ && YOverlapZ)
	{
		if (TxRange.m_min > TyRange.m_min)
		{
			OverlapRange.m_min = TxRange.m_min;
			isRayHitHorizontally = true;
			isRaySideFace = false;
		}
		else
		{
			OverlapRange.m_min = TyRange.m_min;
			isRayHitHorizontally = true;
			isRaySideFace = true;
		}

		if (OverlapRange.m_min < TzRange.m_min)
		{
			OverlapRange.m_min = TzRange.m_min;
			isRayHitHorizontally = false;
		}

		if (TxRange.m_max > TyRange.m_max)
		{
			OverlapRange.m_max = TyRange.m_max;
		}
		else
		{
			OverlapRange.m_max = TxRange.m_max;
		}

		if (OverlapRange.m_max > TzRange.m_max)
		{
			OverlapRange.m_max = TzRange.m_max;
		}

		result.m_didImpact = true;
		result.m_impactDist = raycastLength * OverlapRange.m_min;
		result.m_impactPos = startPos + (fwdNormal * result.m_impactDist);

		if (isRayHitHorizontally)
		{
			if (isRaySideFace)
			{
				Vec3 impactToMaxZ = Vec3(result.m_impactPos.x, result.m_impactPos.y, expandedBox.m_maxs.z) - result.m_impactPos;
				Vec3 impactToMaxX = Vec3(expandedBox.m_maxs.x, result.m_impactPos.y, result.m_impactPos.z) - result.m_impactPos;
				Vec3 impactToMinX = Vec3(expandedBox.m_mins.x, result.m_impactPos.y, result.m_impactPos.z) - result.m_impactPos;
				if (yFlip)
				{
					result.m_impactNormal = CrossProduct3D(impactToMaxZ, impactToMaxX);
				}
				else
				{
					result.m_impactNormal = CrossProduct3D(impactToMaxZ, impactToMinX);
				}
			}
			else
			{
				Vec3 impactToMaxZ = Vec3(result.m_impactPos.x, expandedBox.m_mins.y, result.m_impactPos.z) - result.m_impactPos;
				Vec3 impactToMaxY = Vec3(result.m_impactPos.x, expandedBox.m_maxs.y, result.m_impactPos.z) - result.m_impactPos;
				Vec3 impactToMinY = Vec3(result.m_impactPos.x, result.m_impactPos.y, expandedBox.m_maxs.z) - result.m_impactPos;
				if (xFlip)
				{
					result.m_impactNormal = CrossProduct3D(impactToMaxZ, impactToMinY);
				}
				else
				{
					result.m_impactNormal = CrossProduct3D(impactToMaxZ, impactToMaxY);
				}
			}
		}
		else
		{
			Vec3 impactToMaxY = Vec3(result.m_impactPos.x, expandedBox.m_maxs.y, result.m_impactPos.z) - result.m_impactPos;
			Vec3 impactToMinX = Vec3(expandedBox.m_mins.x, result.m_impactPos.y, result.m_impactPos.z) - result.m_impactPos;
			Vec3 impactToMaxX = Vec3(expandedBox.m_maxs.x, result.m_impactPos.y, result.m_impactPos.z) - result.m_impactPos;
			if (zFlip)
			{
				result.m_impactNormal = CrossProduct3D(impactToMaxY, impactToMinX);
			}
			else
			{
				result.m_impactNormal = CrossProduct3D(impactToMaxY, impactToMaxX);
			}
		}
		result.m_impactNormal.Normalize();
	}
	else
	{
		result.m_didImpact = true;
		return result;
	}

	return result;
}
