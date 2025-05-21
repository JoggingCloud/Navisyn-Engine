#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <math.h>
#include <cmath>
#include <algorithm>

Vec3 GetClamped(Vec3& value, Vec3& minValue, Vec3& maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}
	else if (value > maxValue)
	{
		return maxValue;
	}
	else
	{
		return value;
	}
}

float GetClamped(float value, float minValue, float maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}
	else if (value > maxValue)
	{
		return maxValue;
	}
	else
	{
		return value;
	}
}

Vec3 GetClamped(Vec3& value, float minValue, float maxValue)
{
	value.x = std::clamp(value.x, minValue, maxValue);
	value.y = std::clamp(value.y, minValue, maxValue);
	value.z = std::clamp(value.z, minValue, maxValue);

	return value;
}

float GetClampedZeroToOne(float value)
{
	if (value < 0.0f) 
	{
		return 0.0f;
	}
	else if (value > 1.0f) 
	{
		return 1.0f;
	}
	else 
	{
		return value;
	}
}

float NormalizeByte(unsigned char byte)
{
	if (byte <= 0)
	{
		return 0.f;
	}

	if (byte >= 255)
	{
		return 1.f;
	}
	return static_cast<float>(byte) / 255.f;
}

unsigned char DenormalizeByte(float f)
{
	if (f <= 0.f)
	{
		return 0;
	}

	if (f >= 1.f)
	{
		return 255;
	}

	return static_cast<unsigned char>(f * 256.f);
}

float NormalizeRgbaByte(unsigned char value)
{
	return static_cast<float>(value) / 255.0f;
}

unsigned char DenormalizeRgbaByte(float value)
{
	return static_cast<unsigned char>(RoundDownToInt(value * 255.f));
}

Vec3 Interpolate(Vec3 start, Vec3 end, float t)
{
	return start + t * (end - start);
}

float Interpolate(float start, float end, float t)
{
	return start + t * (end - start);
}

Rgba8 Interpolate(Rgba8 start, Rgba8 end, float fractionOfEnd)
{
	float r = Interpolate(NormalizeRgbaByte(start.r), NormalizeRgbaByte(end.r), fractionOfEnd);
	float g = Interpolate(NormalizeRgbaByte(start.g), NormalizeRgbaByte(end.g), fractionOfEnd);
	float b = Interpolate(NormalizeRgbaByte(start.b), NormalizeRgbaByte(end.b), fractionOfEnd);
	float a = Interpolate(NormalizeRgbaByte(start.a), NormalizeRgbaByte(end.a), fractionOfEnd);
	return Rgba8(DenormalizeRgbaByte(r), DenormalizeRgbaByte(g), DenormalizeRgbaByte(b), DenormalizeRgbaByte(a));
}

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	// Based on the range, Calculate by subtracting value by range Start and then dividing by the distance
	// Value can be any number 
	return (value - rangeStart) / (rangeEnd - rangeStart);
}

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	// Calculate the fraction within the input range.
	float fraction = (inValue - inStart) / (inEnd - inStart);

	// Map the fraction to the output range.
	float outValue = outStart + fraction * (outEnd - outStart);

	return outValue;
}

float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float fraction = (inValue - inStart) / (inEnd - inStart);
	float newfraction = GetClamped(fraction, 0, 1);

	return Interpolate(outStart,outEnd,newfraction);
}

int RoundDownToInt(float value)
{
	return static_cast<int>(floorf(value)); // Converting value from a float to a int and using floorf to round the float down
}

// Angle Utilies
float ConvertDegreesToRadians(float degrees)
{
	return (degrees * (pi / 180));
}
float ConvertRadiansToDegrees(float radians)
{
	return (radians * (180 / pi));
}
float CosDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return cosf(radians);
}
float SinDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return sinf(radians);
}

float TanDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);

	float sine = sinf(radians);
	float cosine = cosf(radians);

	return sine / cosine;
}

float Atan2Degrees(float y, float x)
{
	float radians = atan2f(y,x);
	return ConvertRadiansToDegrees(radians);
}

float GetShortestAngularDispDegrees(float startDegrees, float endDegrees)
{
	// Calculate the angle difference between the start and end degree
	float angularDifference = endDegrees - startDegrees;

	// Check that the difference is within the range -180,180
	while (angularDifference > 180.0f) 
	{
		angularDifference -= 360.0f;
	}
	while (angularDifference <= -180.0f) 
	{
		angularDifference += 360.0f;
	}

	return angularDifference;
}

float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	// Calculate distance between goal and current
	float angleDifference = GetShortestAngularDispDegrees(currentDegrees, goalDegrees);

	// Calculate the change in degrees
	float newAngle = currentDegrees;

	if (angleDifference > 0.f)
	{
		newAngle += GetClamped(angleDifference, 0, maxDeltaDegrees);
	}
	else if (angleDifference < 0.f)
	{
		newAngle += GetClamped(angleDifference, -maxDeltaDegrees, 0);
	}

	// Clamp angle difference to max delta degrees

	return newAngle;
}

float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	// Get the magnitude (length) of the two vectors
	float magnitudeA = GetMagnitude2D(a);
	float magnitudeB = GetMagnitude2D(b);

	// Get cos angle between the vectors 
	float cosTheta = DotProduct2D(a, b) / (magnitudeA * magnitudeB);

	// Find the angle in radians, use arcos 
	float angleRadians = acosf(cosTheta);

	// Convert to degrees
	float angleDegrees = ConvertRadiansToDegrees(angleRadians);

	return angleDegrees;
}

float GetAngleDegreesBetweenVectors3D(Vec3 const& a, Vec3 const& b)
{
	// Get the magnitude (length) of the two vectors
	float magnitudeA = GetMagnitude3D(a);
	float magnitudeB = GetMagnitude3D(b);

	// Get cos angle between the vectors 
	float cosTheta = DotProduct3D(a, b) / (magnitudeA * magnitudeB);

	// Find the angle in radians, use arcos 
	float angleRadians = acosf(cosTheta);

	// Convert to degrees
	float angleDegrees = ConvertRadiansToDegrees(angleRadians);

	return angleDegrees;
}

float DotProduct2D(Vec2 const& a, Vec2 const b)
{
	return (a.x * b.x) + (a.y * b.y);
}

float DotProduct3D(Vec3 const& a, Vec3 const b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float DotProduct4D(Vec4 const& a, Vec4 const b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

float CrossProduct2D(Vec2 const& a, Vec2 const b)
{
	return (a.x * b.y) - (a.y * b.x);
}

Vec3 CrossProduct3D(Vec3 const& a, Vec3 const b)
{
	return Vec3((a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x));
}

Vec3 ZCrossProduct3D(Vec3 const& a, Vec3 const b)
{
	return Vec3(0.f, 0.f, (a.x * b.y) - (a.y * b.x));
}

float GetDistance2D(IntVec2 const& positionA, IntVec2 const& positionB)
{
	return sqrtf(((float)positionA.x - (float)positionB.x) * ((float)positionA.x - (float)positionB.x) + ((float)positionA.y - (float)positionB.y) * ((float)positionA.y - (float)positionB.y));
}

int GetDistanceSquared2D(IntVec2 const& positionA, IntVec2 const& positionB)
{
	return ((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y));
}

// 2D Distance Utilities 
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	return sqrtf((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y));
}
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	return ((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y));
}

int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return abs(pointB.x - pointA.x) + abs(pointB.y - pointA.y);
}

int GetHexTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	int dx = pointA.x - pointB.x;
	int dy = pointA.y - pointB.y;
	int dz = (pointA.x + pointA.y) - (pointB.x + pointB.y);

	return (abs(dx) + abs(dy) + abs(dz)) / 2;
}

float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 distToProjectOnto = vectorToProjectOnto.GetNormalized();

	return DotProduct2D(vectorToProject, distToProjectOnto);
}

Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	float veclength = GetProjectedLength2D(vectorToProject, vectorToProjectOnto);

	Vec2 normal = vectorToProjectOnto.GetNormalized();

	return normal * veclength;
}

float GetMagnitude2D(Vec2 const& vector)
{
	return sqrtf((vector.x * vector.x) + (vector.y * vector.y));
}

float GetMagnitudeSquared2D(Vec2 const& vector)
{
	return ((vector.x * vector.x) + (vector.y * vector.y));
}

// 3D Distance Utilities
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return sqrtf((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y) + (positionA.z - positionB.z ) * (positionA.z - positionB.z));
}
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return ((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y) + (positionA.z - positionB.z ) * (positionA.z - positionB.z));
}
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return sqrtf((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y));
}
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return ((positionA.x - positionB.x) * (positionA.x - positionB.x) + (positionA.y - positionB.y) * (positionA.y - positionB.y));
}

float GetProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	Vec3 distToProjectOnto = vectorToProjectOnto.GetNormalized();

	return DotProduct3D(vectorToProject, distToProjectOnto);
}

Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	float veclength = GetProjectedLength3D(vectorToProject, vectorToProjectOnto);

	if (vectorToProjectOnto.GetLengthSquared() < EPSILON)
	{
		return Vec3::ZERO;
	}

	Vec3 normal = vectorToProjectOnto.GetNormalized();

	return normal * veclength;
}

float GetMagnitude3D(Vec3 const& vector)
{
	return sqrtf((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
}

float GetMagnitudeSquared3D(Vec3 const& vector)
{
	return ((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
}

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	// Calculate the distance between the point and the center of the disc
	float distance = GetDistance2D(discCenter, point);

	// Check if the distance is less than or equal to the disc's radius
	return distance <= discRadius;
}

bool IsPointInsideSphere(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius)
{
	float distance = GetDistance3D(sphereCenter, point);

	return distance <= sphereRadius;
}

bool IsPointInsideHemisphere(Vec3 const& point, Vec3 const& hemisphereCenter, Vec3 const& hemisphereNormal, float hemisphereRadius)
{
	float distance = GetDistance3D(hemisphereCenter, point);

	if (distance > hemisphereRadius)
	{
		return false;
	}

	Vec3 toPoint = point - hemisphereCenter;
	float dotProduct = DotProduct3D(toPoint, hemisphereNormal);

	return dotProduct >= 0.f;
}

// Adjust function
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	// Check if the point is inside the disc (circle) defined by sectorTip and sectorRadius
	if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius)) 
	{
		return false;
	}

	// Calculate the vector from the sector tip to the point
	Vec2 displacement = point - sectorTip;

	// Calculate the angle (in degrees) between the sector's forward direction and the point
	float angleDegrees = GetAngleDegreesBetweenVectors2D(displacement, Vec2{ 1.0f, 0.0f });

	// Normalize the angle to be in the range [0, 360)
	while (angleDegrees < 0)
	{
		angleDegrees += 360;
	}

	// Adjust the angle to be relative to the sector's orientation
	angleDegrees -= sectorForwardDegrees;

	// Normalize the adjusted angle to be in the range [-180, 180)
	while (angleDegrees < -180) 
	{
		angleDegrees += 360;
	}
	while (angleDegrees >= 180) 
	{
		angleDegrees -= 360;
	}

	// Calculate the start and end angles of the sector
	float halfAperture = sectorApertureDegrees * 0.5f;
	float startAngle = -halfAperture;
	float endAngle = halfAperture;

	// Check if the adjusted angle is within the sector's angular range
	if (angleDegrees >= startAngle && angleDegrees <= endAngle) 
	{
		return true;
	}

	return false;
}

bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	return box.IsPointInside(point);
}

bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box)
{
	return box.IsPointInside(point);
}

bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule)
{
	Vec2 startToCenter = point - capsule.m_start;
	Vec2 endToCenter = point - capsule.m_end;
	Vec2 length = (capsule.m_end - capsule.m_start).GetNormalized();
	float radiusSquared = capsule.m_radius * capsule.m_radius;

	if (DotProduct2D(startToCenter, length) <= 0.0f) 
	{
		if (startToCenter.GetLengthSquared() <= radiusSquared)
		{
			return true;
		}
	}

	if (DotProduct2D(endToCenter, -length) <= 0.0f)
	{
		if (endToCenter.GetLengthSquared() <= radiusSquared)
		{
			return true;
		}
	}

	float centerRange = DotProduct2D(startToCenter, length);
	Vec2 lengthTangent = centerRange * length;

	Vec2 lengthNormal = startToCenter - lengthTangent;

	if (lengthNormal.GetLengthSquared() < radiusSquared)
	{
		return true;
	}

	return false;
}

bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	// Calculate the squared length of the bone
	OBB2 square;
	square.m_center = (boneStart + boneEnd) * 0.5f;
	square.m_halfDimensions = Vec2((boneEnd - boneStart).GetLength() * 0.5f, radius);
	square.m_iBasisNormal = (boneEnd - boneStart).GetNormalized();

	if (IsPointInsideOBB2D(point, square))
	{
		return true;
	}

	if (IsPointInsideDisc2D(point, boneStart, radius))
	{
		return true;
	}

	if (IsPointInsideDisc2D(point, boneEnd, radius))
	{
		return true;
	}

	return false;
}

bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox)
{
	Vec2 jBasisNorm = orientedBox.m_iBasisNormal.GetRotated90Degrees();
	Vec2 distanceFromCenterToPoint = point - orientedBox.m_center;
	float pointI = DotProduct2D(orientedBox.m_iBasisNormal, distanceFromCenterToPoint);
	float pointJ = DotProduct2D(jBasisNorm, distanceFromCenterToPoint);

	if (pointI >= orientedBox.m_halfDimensions.x)
	{
		return false;
	}
	if (pointI <= -orientedBox.m_halfDimensions.x)
	{
		return false;
	}
	if (pointJ >= orientedBox.m_halfDimensions.y)
	{
		return false;
	}
	if (pointJ <= -orientedBox.m_halfDimensions.y)
	{
		return false;
	}

	return true;
}

bool IsPointInsideOBB3D(Vec3 const& point, OBB3 const& orientedBox)
{
	Vec3 distanceFromCenterToPoint = point - orientedBox.m_center;

	float pointI = DotProduct3D(orientedBox.m_iBasisNormal, distanceFromCenterToPoint);
	float pointJ = DotProduct3D(orientedBox.m_jBasisNormal, distanceFromCenterToPoint);
	float pointK = DotProduct3D(orientedBox.m_kBasisNormal, distanceFromCenterToPoint);

	if (pointI >= orientedBox.m_halfDimensions.x || pointI <= -orientedBox.m_halfDimensions.x)
	{
		return false;
	}
	if (pointJ >= orientedBox.m_halfDimensions.y || pointJ <= -orientedBox.m_halfDimensions.y)
	{
		return false;
	}
	if (pointK >= orientedBox.m_halfDimensions.z || pointK <= -orientedBox.m_halfDimensions.z)
	{
		return false;
	}

	return true;
}

bool IsPointInsideZCylinder(Vec3 const& point, Vec2 const& cylinderCenterXY, FloatRange cylinderZRange, float radius)
{
	if (point.z > cylinderZRange.m_max || point.z < cylinderZRange.m_min)
	{
		return false;
	}
	Vec2 pointXY = Vec2(point.x, point.y);
	Vec2 toPoint = pointXY - cylinderCenterXY;
	float distanceSquared = toPoint.GetLengthSquared();
	if (distanceSquared > radius * radius)
	{
		return false;
	}
	return true;
}

bool IsPointInsideCylinder(Vec3 const& point, Vec3 const& startPos, Vec3 const& endPos, float radius)
{
	// Compute the vector along the cylinder axis
	Vec3 cylinderAxis = (endPos - startPos).GetNormalized();
	float cylinderHeight = GetDistance3D(startPos, endPos);

	// Project the point onto the cylinder axis
	Vec3 pointToStart = point - startPos;
	float projection = DotProduct3D(pointToStart, cylinderAxis);

	// Check if the point is within the cylinder height
	if (projection < 0 || projection > cylinderHeight) 
	{
		return false;
	}

	// Compute the closest point on the cylinder axis
	Vec3 closestPointOnAxis = startPos + (cylinderAxis * projection);

	// Check the radial distance
	float radialDistanceSquared = GetDistanceSquared3D(point, closestPointOnAxis);
	return radialDistanceSquared <= (radius * radius);
}

bool IsPointInsideBottomZCapsule(Vec3 const& point, Vec3 const& capsuleStart, float capsuleRadius)
{
	float distaceToCenterSq = (point - capsuleStart).GetLengthSquared();

	return distaceToCenterSq <= (capsuleRadius * capsuleRadius);
}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	// Check if the point is inside the disc (circle) defined by sectorTip and sectorRadius
	if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius)) 
	{
		return false;
	}

	// Calculate the vector from the sector tip to the point
	Vec2 displacementSectorTipToPoint = point - sectorTip;

	// Calculate the angle (in degrees) between the displacement vector and the sector's forward normal
	float angleDegrees = GetAngleDegreesBetweenVectors2D(displacementSectorTipToPoint, sectorForwardNormal);

	// Normalize the angle to be in the range [0, 360)
	while (angleDegrees < 0) 
	{
		angleDegrees += 360;
	}

	// Check if the absolute value of the angle is less than half of the sector's aperture
	if (abs(angleDegrees) < sectorApertureDegrees * 0.5f) 
	{
		return true;
	}

	return false;
}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	    // Calculate the vector from the sector tip to the point
    Vec2 toPoint = point - sectorTip;

    // Calculate the angle (in degrees) of the vector with respect to the sector's forward direction
    float angleToVector = toPoint.GetOrientationDegrees();

    // Calculate the difference between the sector's forward angle and the angle to the vector
    float angleDifference = GetShortestAngularDispDegrees(sectorForwardDegrees, angleToVector);

    // Check if the point is within the specified radius
    bool isInsideRadius = toPoint.GetLengthSquared() <= (sectorRadius * sectorRadius);

    // Check if the angle difference is within the sector's aperture angle
    bool isInsideAperture = (angleDifference <= sectorApertureDegrees * 0.5f) && (angleDifference >= -sectorApertureDegrees * 0.5f);

    return isInsideRadius && isInsideAperture;
}

bool IsPointInsideTriangle3D(Vec3 const& point, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2)
{
	// Barycentric coordinates method for checking if a point is in a triangle
	Vec3 edge0 = vertex1 - vertex0; // From vertex 0 to vertex 1
	Vec3 edge1 = vertex2 - vertex0; // From vertex 0 to vertex 2
	Vec3 pointToVertex0 = point - vertex0; // Vector from vertex 0 to the point

	// Dot products of edge vectors and point vector
	float edge0DotEdge0 = DotProduct3D(edge0, edge0); // Determine how much the point is proj onto edge0
	float edge0DotEdge1 = DotProduct3D(edge0, edge1); // Determine how much edge0 and edge1 are aligned
	float edge0DotPoint = DotProduct3D(edge0, pointToVertex0); // Determine how much the point moves along edge0 direction

	float edge1DotEdge1 = DotProduct3D(edge1, edge1); // Determine how much the point is proj onto edge1
	float edge1DotPoint = DotProduct3D(edge1, pointToVertex0); // Determine how much the point moves along edge1 direction

	// Compute the denominator of the barycentric coordinates
	float denominator = 1.f / (edge0DotEdge0 * edge1DotEdge1 - edge0DotEdge1 * edge0DotEdge1);
	 
	// Compute the barycentric coordinates (u, v)
	float u = (edge1DotEdge1 * edge0DotPoint - edge0DotEdge1 * edge1DotPoint) * denominator;
	float v = (edge0DotEdge0 * edge1DotPoint - edge0DotEdge1 * edge0DotPoint) * denominator;

	 return (u >= 0.f) && (v >= 0.f) && (u + v <= 1.f);
}

bool IsPointInsideTriangle3D(Vec3 const& point, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2, float& outU, float& outV)
{
	// Barycentric coordinates method for checking if a point is in a triangle
	Vec3 edge0 = vertex1 - vertex0; // From vertex 0 to vertex 1
	Vec3 edge1 = vertex2 - vertex0; // From vertex 0 to vertex 2
	Vec3 pointToVertex0 = point - vertex0; // Vector from vertex 0 to the point

	// Dot products of edge vectors and point vector
	float edge0DotEdge0 = DotProduct3D(edge0, edge0); // Determine how much the point is proj onto edge0
	float edge0DotEdge1 = DotProduct3D(edge0, edge1); // Determine how much edge0 and edge1 are aligned
	float edge0DotPoint = DotProduct3D(edge0, pointToVertex0); // Determine how much the point moves along edge0 direction

	float edge1DotEdge1 = DotProduct3D(edge1, edge1); // Determine how much the point is proj onto edge1
	float edge1DotPoint = DotProduct3D(edge1, pointToVertex0); // Determine how much the point moves along edge1 direction

	// Compute the denominator of the barycentric coordinates
	float denominator = 1.f / (edge0DotEdge0 * edge1DotEdge1 - edge0DotEdge1 * edge0DotEdge1);

	// Compute the barycentric coordinates (u, v)
	outU = (edge1DotEdge1 * edge0DotPoint - edge0DotEdge1 * edge1DotPoint) * denominator;
	outV = (edge0DotEdge0 * edge1DotPoint - edge0DotEdge1 * edge0DotPoint) * denominator;

	return (outU >= 0.f) && (outV >= 0.f) && (outU + outV <= 1.f);
}

bool IsPointInsideHexagon3D(Vec3 const& point3D, Vec3 const& hexCenter3D, float hexRadius)
{
	// Calculate displacement of the point relative to the hexagon's center
	Vec2 point = Vec2(point3D.x, point3D.y);
	Vec2 center = Vec2(hexCenter3D.x, hexCenter3D.y);
	float radius = hexRadius;

	Vec2 displacement = point - center;

	for (float theta = 0.f; theta < 360.f; theta += 60.f)
	{
		Vec2 hexPoint = center + Vec2(radius * CosDegrees(theta), radius * SinDegrees(theta));
		Vec2 nextHexPoint = center + Vec2(radius * CosDegrees(theta + 60.f), radius * SinDegrees(theta + 60.f));

		Vec2 edge = nextHexPoint - hexPoint;
		Vec2 toPoint = point - hexPoint;

		float cross = CrossProduct2D(edge, toPoint);
		if (cross < 0)
		{
			return false;
		}
	}

	return true;
}

bool IsPointInsideConvexHull2D(Vec2 const& point, ConvexHull2D const& convexhull)
{
	for (const Plane2D& plane : convexhull.m_boundingplanes)
	{
		float distance = plane.GetAltitudeOfPoint(point);

		if (distance > 0.f)
		{
			return false;
		}
	}

	return true;
}

// Geometric Query Utilities
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	GetDistanceSquared2D(centerA, centerB);
	if ((radiusA + radiusB) >= sqrtf((centerB.x - centerA.x) * (centerB.x - centerA.x) + (centerB.y - centerA.y) * (centerB.y - centerA.y)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float distanceSquared = GetDistanceSquared3D(centerA, centerB);

	float radiiSumSquared = (radiusA + radiusB) * (radiusA + radiusB);

	return distanceSquared <= radiiSumSquared;
}

bool DoAABB2DsOverlap(AABB2 const& boxA, AABB2 const& boxB)
{
	bool overlapX = (boxA.m_maxs.x > boxB.m_mins.x) && (boxA.m_mins.x < boxB.m_maxs.x);
	bool overlapY = (boxA.m_maxs.y > boxB.m_mins.y) && (boxA.m_mins.y < boxB.m_maxs.y);

	return overlapX && overlapY;
}

bool DoAABB3DsOverlap(AABB3 const& boxA, AABB3 const& boxB)
{
	bool overlapX = (boxA.m_maxs.x > boxB.m_mins.x) && (boxA.m_mins.x < boxB.m_maxs.x);
	bool overlapY = (boxA.m_maxs.y > boxB.m_mins.y) && (boxA.m_mins.y < boxB.m_maxs.y);
	bool overlapZ = (boxA.m_maxs.z > boxB.m_mins.z) && (boxA.m_mins.z < boxB.m_maxs.z);

	return overlapX && overlapY && overlapZ;
}

bool DoSphereAndAABB3DOverlap(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& box)
{
	Vec3 closestPoint = box.GetNearestPoint(sphereCenter);

	if (!IsPointInsideSphere(closestPoint, sphereCenter, sphereRadius))
	{
		return false;
	}

	return true;
}

bool DoOBB3DAndPlaneOverlap(OBB3 const& obb, Plane3D const& plane)
{
	// Calculate the projection radius of the OBB on the plane normal
	float radius = obb.m_halfDimensions.x * fabs(DotProduct3D(obb.m_iBasisNormal, plane.m_normal)) + obb.m_halfDimensions.y * fabs(DotProduct3D(obb.m_jBasisNormal, plane.m_normal)) + obb.m_halfDimensions.z * fabs(DotProduct3D(obb.m_kBasisNormal, plane.m_normal));

	// Calculate the distance from the OBB center to the plane
	float distance = DotProduct3D(plane.m_normal, obb.m_center) - plane.m_distance;

	// Check if the plane intersects the OBB
	return fabs(distance) <= radius;
}

bool DoSphereAndPlaneOverlap(Vec3 const& sphereCenter, float sphereRadius, Plane3D const& plane)
{
	// Calculate the distance from the sphere center to the plane 
	float distance = plane.GetAltitudeOfPoint(sphereCenter);

	// Check if the plane intersects the sphere
	return fabs(distance) <= sphereRadius;
}

bool DoPlaneAndZCapsuleBottomOverlap(Vec3 const& capsuleStart, float capsuleRadius, Plane3D const& plane)
{
	float bottomHemisphereDistToPlane = plane.GetAltitudeOfPoint(capsuleStart);

	if (fabs(bottomHemisphereDistToPlane) <= capsuleRadius) return true;

	return false;
}

bool DoLineSegmentIntersect3D(Vec3 const& lineAStart, Vec3 const& lineAEnd, Vec3 const& lineBStart, Vec3 const& lineBEnd, Vec3& outIntersection)
{
	// Step 1: Calculate the direction vectors for both lines
	Vec3 directionA = lineAEnd - lineAStart;
	Vec3 directionB = lineBEnd - lineBStart;

	// Step 2: Calculate the vector from the start of line B to the start of line A
	Vec3 startDifference = lineAStart - lineBStart;

	// Step 3: Calculate dot products needed for the formula
	float directionA_dot_directionA = DotProduct3D(directionA, directionA);  // Length squared of direction A
	float directionA_dot_directionB = DotProduct3D(directionA, directionB);  // How much direction A and B align
	float directionB_dot_directionB = DotProduct3D(directionB, directionB);  // Length squared of direction B
	float directionA_dot_startDifference = DotProduct3D(directionA, startDifference);  // Projection of start difference onto A
	float directionB_dot_startDifference = DotProduct3D(directionB, startDifference);  // Projection of start difference onto B

	// Step 4: Calculate the denominator to detect if lines are parallel
	float denominator = directionA_dot_directionA * directionB_dot_directionB - directionA_dot_directionB * directionA_dot_directionB;
	if (fabs(denominator) < 1e-6f)
	{
		// Lines are parallel or almost parallel — no intersection
		return false;
	}

	// Step 5: Solve for the parameters s and t along each line
	float s = (directionA_dot_directionB * directionB_dot_startDifference - directionB_dot_directionB * directionA_dot_startDifference) / denominator;
	float t = (directionA_dot_directionA * directionB_dot_startDifference - directionA_dot_directionB * directionA_dot_startDifference) / denominator;

	// Step 6: Check if s and t are within [0, 1] meaning the intersection is within the segments
	if (s >= 0.f && s <= 1.f && t >= 0.f && t <= 1.f)
	{
		// Step 7: Calculate the intersection point
		outIntersection = lineAStart + s * directionA;
		return true;
	}

	// No intersection within the line segments
	return false;
}

bool DoZCylindersOverlap3D(Vec2 cylinder1CenterXY, float cylinder1Radius, FloatRange cylinder1MinMaxZ, Vec2 cylinder2CenterXY, float cylinder2Radius, FloatRange cylinder2MinMaxZ)
{
	Vec3 referencePosition(cylinder2CenterXY.x, cylinder2CenterXY.y, (cylinder2MinMaxZ.m_min + cylinder2MinMaxZ.m_max) * 0.5f);
	Vec3 nearestPoint = GetNearestPointOnZCylinder(referencePosition, cylinder1CenterXY, cylinder1MinMaxZ, cylinder1Radius);

	if (!IsPointInsideZCylinder(nearestPoint, cylinder2CenterXY, cylinder2MinMaxZ, cylinder2Radius))
	{
		return false;
	}
	return true;
}

bool DoZCylinderAndAABB3DOverlap(Vec2 cylinderCenterXY, float cylinderRadius, FloatRange cylinderMinMaxZ, AABB3 box)
{
	if (box.m_maxs.z <= cylinderMinMaxZ.m_min || box.m_mins.z >= cylinderMinMaxZ.m_max)
	{
		return false;
	}
	AABB2 topView = AABB2(Vec2(box.m_mins.x, box.m_mins.y), Vec2(box.m_maxs.x, box.m_maxs.y));
	Vec2 point = GetNearestPointOnAABB2D(cylinderCenterXY, topView);
	if (!IsPointInsideDisc2D(point, cylinderCenterXY, cylinderRadius))
	{
		return false;
	}
	return true;
}

bool DoZCylinderAndSphereOverlap(Vec2 cylinderCenterXY, float cylinderRadius, FloatRange cylinderMinMaxZ, Vec3 sphereCenter, float sphereRadius)
{
	if (sphereCenter.z + sphereRadius <= cylinderMinMaxZ.m_min || sphereCenter.z - sphereRadius >= cylinderMinMaxZ.m_max)
	{
		return false;
	}
	Vec3 nearestPointZ = GetNearestPointOnZCylinder(sphereCenter, cylinderCenterXY, cylinderMinMaxZ, cylinderRadius);
	if (!IsPointInsideSphere(nearestPointZ, sphereCenter, sphereRadius))
	{
		return false;
	}
	return true;
}

Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius)
{
	// Calculate the vector from the disc center to the reference position
	float dx = referencePosition.x - discCenter.x;
	float dy = referencePosition.y - discCenter.y;

	// Calculate the distance from the disc center to the reference position
	float distance = sqrtf((dx * dx) + (dy * dy));

	// If the reference position is inside the disc, it's already the nearest point
	if (distance <= discRadius)
	{
		return referencePosition;
	}

	// Calculate the normalized vector from the disc center to the reference position
	float normalizedX = dx / distance;
	float normalizedY = dy / distance;

	// Calculate the nearest point on the disc along the line from the center
	float nearestPointX = discCenter.x + normalizedX * discRadius;
	float nearestPointY = discCenter.y + normalizedY * discRadius;

	return Vec2(nearestPointX, nearestPointY);
}

Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePosition, AABB2& box)
{
	return box.GetNearestPoint(referencePosition);
}

Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, LineSegment2 const& infiniteLine)
{
	Vec2 infiniteLineDistance = infiniteLine.m_end - infiniteLine.m_start;
	Vec2 startDistPoint = referencePosition - infiniteLine.m_start;

	if (DotProduct2D(startDistPoint, infiniteLineDistance) <= 0)
	{
		return infiniteLine.m_start;
	}

	Vec2 endDistPoint = referencePosition - infiniteLine.m_end;

	if (DotProduct2D(endDistPoint, infiniteLineDistance) >= 0)
	{
		return infiniteLine.m_end;
	}

	Vec2 displacement = GetProjectedOnto2D(startDistPoint, infiniteLineDistance);

	return infiniteLine.m_start + displacement;
}

Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, Vec2 const& lineSegStart, Vec2 const& anotherPointOnLine)
{
	Vec2 lineDistance = anotherPointOnLine - lineSegStart;
	Vec2 referenceToStart = referencePosition - lineSegStart;

	if (DotProduct2D(referenceToStart, lineDistance) <= 0)
	{
		return lineSegStart;
	}

	Vec2 endDistPoint = referencePosition - anotherPointOnLine;

	if (DotProduct2D(endDistPoint, lineDistance) >= 0)
	{
		return anotherPointOnLine;
	}

	Vec2 displacement = GetProjectedOnto2D(referenceToStart, lineDistance);

	return lineSegStart + displacement;
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePosition, LineSegment2 const& lineSegment)
{
	Vec2 segmentDistance = lineSegment.m_end - lineSegment.m_start;
	Vec2 startDistPoint = referencePosition - lineSegment.m_start;

	if (DotProduct2D(startDistPoint, segmentDistance) <= 0)
	{
		return lineSegment.m_start;
	}

	Vec2 endDistPoint = referencePosition - lineSegment.m_end;

	if (DotProduct2D(endDistPoint, segmentDistance) >= 0)
	{
		return lineSegment.m_end;
	}

	Vec2 displacement = GetProjectedOnto2D(startDistPoint, segmentDistance);

	return lineSegment.m_start + displacement;
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePosition, Vec2 const& lineSegStart, Vec2 const& lineSegEnd)
{
	Vec2 segmentDistance = lineSegEnd - lineSegStart;
	Vec2 startDistPoint = referencePosition - lineSegStart;

	if (DotProduct2D(startDistPoint, segmentDistance) <= 0)
	{
		return lineSegStart;
	}

	Vec2 endDistPoint = referencePosition - lineSegEnd;

	if (DotProduct2D(endDistPoint, segmentDistance) >= 0)
	{
		return lineSegEnd;
	}

	Vec2 displacement = GetProjectedOnto2D(startDistPoint, segmentDistance);

	return lineSegStart + displacement;
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePosition, Capsule2 const& capsule)
{
	Vec2 boneDirection = capsule.m_end - capsule.m_start;

	Vec2 posToReference = referencePosition - capsule.m_start;

	float t = DotProduct2D(posToReference, boneDirection) / DotProduct2D(boneDirection, boneDirection);

	t = GetClampedZeroToOne(t);

	// Calculate the nearest point on the line segment
	Vec2 nearestPointOnLine = capsule.m_start + t * boneDirection;

	// Calculate the squared distance from the reference position to the nearest point on the line
	float squaredDistanceToLine = DotProduct2D(nearestPointOnLine - referencePosition, nearestPointOnLine - referencePosition);

	// Calculate the squared radius
	float squaredRadius = capsule.m_radius * capsule.m_radius;

	// If the squared distance is less than or equal to the squared radius, the reference point is inside the capsule
	if (squaredDistanceToLine <= squaredRadius)
	{
		return referencePosition;
	}
	else
	{
		// Calculate the unit direction vector from the nearest point on the line to the reference position
		Vec2 directionToLine = (nearestPointOnLine - referencePosition) / sqrtf(squaredDistanceToLine);

		// Calculate the nearest point on the capsule by moving from the nearest point on the line in the direction of the reference position
		Vec2 nearestPointOnCapsule = nearestPointOnLine - capsule.m_radius * directionToLine;

		return nearestPointOnCapsule;
	}
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePosition, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 boneDirection = boneEnd - boneStart;

	Vec2 posToReference = referencePosition - boneStart;

	float t = DotProduct2D(posToReference, boneDirection) / DotProduct2D(boneDirection, boneDirection);

	t = GetClampedZeroToOne(t);

	// Calculate the nearest point on the line segment
	Vec2 nearestPointOnLine = boneStart + t * boneDirection;

	// Calculate the squared distance from the reference position to the nearest point on the line
	float squaredDistanceToLine = DotProduct2D(nearestPointOnLine - referencePosition, nearestPointOnLine - referencePosition);

	// Calculate the squared radius
	float squaredRadius = radius * radius;

	// If the squared distance is less than or equal to the squared radius, the reference point is inside the capsule
	if (squaredDistanceToLine <= squaredRadius) 
	{
		return referencePosition;
	}
	else 
	{
		// Calculate the unit direction vector from the nearest point on the line to the reference position
		Vec2 directionToLine = (nearestPointOnLine - referencePosition) / sqrtf(squaredDistanceToLine);

		// Calculate the nearest point on the capsule by moving from the nearest point on the line in the direction of the reference position
		Vec2 nearestPointOnCapsule = nearestPointOnLine - radius * directionToLine;

		return nearestPointOnCapsule;
	}
}

Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePosition, OBB2 const& orientedBox)
{
	Vec2 jBasisNorm = orientedBox.m_iBasisNormal.GetRotated90Degrees();
	Vec2 distanceFromCenterToPoint = referencePosition - orientedBox.m_center;
	float pointI = DotProduct2D(distanceFromCenterToPoint, orientedBox.m_iBasisNormal);
	float pointJ = DotProduct2D(distanceFromCenterToPoint, jBasisNorm);

	float nearestPointI = GetClamped(pointI, -orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.x);
	float nearestPointJ = GetClamped(pointJ, -orientedBox.m_halfDimensions.y, orientedBox.m_halfDimensions.y);

	return orientedBox.m_center + (orientedBox.m_iBasisNormal * nearestPointI) + (jBasisNorm * nearestPointJ);
}

Vec2 const GetNearestPointOnPlane2D(Vec2 const& referencePosition, Plane2D const& plane)
{
	Vec2 distanceToPlane = referencePosition - (plane.m_normal * plane.m_distance);
	float point = DotProduct2D(distanceToPlane, plane.m_normal);
	return referencePosition - (point * plane.m_normal);
}

Vec3 const GetNearestPointOnSphere(Vec3 const& referencePosition, Vec3 const& sphereCenter, float sphereRadius)
{
	// Calculate the vector from the sphere center to the reference position
	float dx = referencePosition.x - sphereCenter.x;
	float dy = referencePosition.y - sphereCenter.y;
	float dz = referencePosition.z - sphereCenter.z;

	// Calculate the distance from the sphere center to the reference position
	float distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));

	// If the reference position is inside the sphere, it's already the nearest point
	if (distance <= sphereRadius)
	{
		return referencePosition;
	}

	// Calculate the normalized vector from the sphere center to the reference position
	float normalizedX = dx / distance;
	float normalizedY = dy / distance;
	float normalizedZ = dz / distance;

	// Calculate the nearest point on the sphere along the line from the center
	float nearestPointX = sphereCenter.x + normalizedX * sphereRadius;
	float nearestPointY = sphereCenter.y + normalizedY * sphereRadius;
	float nearestPointZ = sphereCenter.z + normalizedZ * sphereRadius;

	return Vec3(nearestPointX, nearestPointY, nearestPointZ);
}

Vec3 const GetNearestPointOnHemisphere(Vec3 const& referencePosition, Vec3 const& hemisphereCenter, float hemisphereRadius)
{
	// Calculate the vector from the hemisphere center to the reference position
	float dx = referencePosition.x - hemisphereCenter.x;
	float dy = referencePosition.y - hemisphereCenter.y;
	float dz = referencePosition.z - hemisphereCenter.z;

	// Calculate the distance from the hemisphere center to the reference position
	float distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));

	// If the reference position is inside the hemisphere, and above the center, it's already the nearest point
	if (distance <= hemisphereRadius && referencePosition.z >= hemisphereCenter.z)
	{
		return referencePosition;
	}

	// Calculate the normalized vector from the hemisphere center to the reference position
	float normalizedX = dx / distance;
	float normalizedY = dy / distance;
	float normalizedZ = dz / distance;

	// Calculate the nearest point on the sphere along the line from the center
	float nearestPointX = hemisphereCenter.x + normalizedX * hemisphereRadius;
	float nearestPointY = hemisphereCenter.y + normalizedY * hemisphereRadius;
	float nearestPointZ = hemisphereCenter.z + normalizedZ * hemisphereRadius;

	// Ensure the nearest point lies in the upper half of the hemisphere
	if (nearestPointZ < hemisphereCenter.z)
	{
		nearestPointZ = hemisphereCenter.z; // Clamp it to the plane dividing the hemisphere
	}

	return Vec3(nearestPointX, nearestPointY, nearestPointZ);
}

Vec3 const GetNearestPointOnAABB3D(Vec3 const& referencePosition, AABB3& box)
{
	return box.GetNearestPoint(referencePosition);
}

Vec3 const GetNearestPointOnZCylinder(Vec3 const& referencePosition, Vec2 const& cylinderCenterXY, FloatRange cylinderMinMaxZ, float radius)
{
	// Clamp the Z position to the cylinder's Z extents, if outside them
	Vec2 toPointXY = Vec2(referencePosition.x, referencePosition.y) - cylinderCenterXY;
	float distance = toPointXY.GetLength();
	if (distance > radius)
	{
		distance = radius;
	}
	float zPos = GetClamped(referencePosition.z, cylinderMinMaxZ.m_min, cylinderMinMaxZ.m_max);
	Vec3 nearestPoint = Vec3(cylinderCenterXY.x, cylinderCenterXY.y, zPos) + Vec3(toPointXY.x, toPointXY.y, 0.f).GetNormalized() * distance;
	return nearestPoint;
}

Vec3 const GetNearestPointOnZAlignedCapsuleBottom(Vec3 const& capsuleStart, float capsuleRadius, Plane3D const& fixedPlane)
{
	// The capsule bottom hemisphere center is at capsuleStart

	// Project the capsule start (bottom hemisphere center) onto the plane
	float distToPlane = fixedPlane.GetAltitudeOfPoint(capsuleStart);

	// If the bottom hemisphere is closer to or touching the plane, the nearest point will be capsuleStart - plane normal * capsule radius
	if (distToPlane < capsuleRadius) 
	{
		return capsuleStart - fixedPlane.m_normal * capsuleRadius;
	}

	// If the capsule is not intersecting the plane, the nearest point will be on the surface of the hemisphere closer to the plane
	return capsuleStart - fixedPlane.m_normal * distToPlane;
}

Vec3 const GetNearestPointOnOBB3D(Vec3 const& referencePosition, OBB3 const& orientedBox)
{
	Vec3 distanceFromCenterToPoint = referencePosition - orientedBox.m_center;;
	float pointI = DotProduct3D(distanceFromCenterToPoint, orientedBox.m_iBasisNormal);
	float pointJ = DotProduct3D(distanceFromCenterToPoint, orientedBox.m_jBasisNormal);
	float pointK = DotProduct3D(distanceFromCenterToPoint, orientedBox.m_kBasisNormal);

	float nearestPointI = GetClamped(pointI, -orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.x);
	float nearestPointJ = GetClamped(pointJ, -orientedBox.m_halfDimensions.y, orientedBox.m_halfDimensions.y);
	float nearestPointK = GetClamped(pointK, -orientedBox.m_halfDimensions.z, orientedBox.m_halfDimensions.z);

	return orientedBox.m_center + (orientedBox.m_iBasisNormal * nearestPointI) + (orientedBox.m_jBasisNormal * nearestPointJ) + (orientedBox.m_kBasisNormal * nearestPointK);
}

Vec3 const GetNearestPointOnPlane3D(Vec3 const& referencePosition, Plane3D const& plane)
{
	Vec3 distanceToPlane = referencePosition - (plane.m_normal * plane.m_distance);
	float point = DotProduct3D(distanceToPlane, plane.m_normal);
	return referencePosition - (point * plane.m_normal);
}

Vec3 const GetNearestPointOnTriangle3D(Vec3 const& referencePosition, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2)
{
	// Project the reference point onto the triangle's plane
	Vec3 edge0 = vertex1 - vertex0;
	Vec3 edge1 = vertex2 - vertex0;
	Vec3 normal = CrossProduct3D(edge0, edge1).GetNormalized();
	float distance = DotProduct3D(normal, vertex0);
	Plane3D trianglePlane = Plane3D(normal, distance);

	Vec3 projectedPoint = GetNearestPointOnPlane3D(referencePosition, trianglePlane);

	// Check if the projected point is inside the triangle
	if (IsPointInsideTriangle3D(projectedPoint, vertex0, vertex1, vertex2))
	{
		return projectedPoint; // If the point is inside the triangle, then that's the nearest point
	}

	// Find the nearest point on the triangle edges if the point is outside the triangle
	Vec3 closestPointOnEdge0 = GetNearestPointOnLineSegment3D(referencePosition, vertex0, vertex1);
	Vec3 closestPointOnEdge1 = GetNearestPointOnLineSegment3D(referencePosition, vertex1, vertex2);
	Vec3 closestPointOnEdge2 = GetNearestPointOnLineSegment3D(referencePosition, vertex2, vertex0);

	// Return the closest of the three points
	float distToEdge0 = (closestPointOnEdge0 - projectedPoint).GetLengthSquared();
	float distToEdge1 = (closestPointOnEdge1 - projectedPoint).GetLengthSquared();
	float distToEdge2 = (closestPointOnEdge2 - projectedPoint).GetLengthSquared();

	if (distToEdge0 < distToEdge1 && distToEdge0 < distToEdge2)
	{
		return closestPointOnEdge0;
	}
	else if (distToEdge1 < distToEdge2)
	{
		return closestPointOnEdge1;
	}
	else
	{
		return closestPointOnEdge2;
	}
}

Vec3 const GetNearestPointOnTriangleEdge3D(Vec3 const& referencePosition, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2)
{
	// Project the reference point onto the triangle's plane
	Vec3 edge0 = vertex1 - vertex0;
	Vec3 edge1 = vertex2 - vertex0;
	Vec3 normal = CrossProduct3D(edge0, edge1).GetNormalized();
	float distance = DotProduct3D(normal, vertex0);
	Plane3D trianglePlane = Plane3D(normal, distance);

	Vec3 projectedPoint = GetNearestPointOnPlane3D(referencePosition, trianglePlane);

	// Find the nearest point on the triangle edges if the point is outside the triangle
	Vec3 closestPointOnEdge0 = GetNearestPointOnLineSegment3D(referencePosition, vertex0, vertex1);
	Vec3 closestPointOnEdge1 = GetNearestPointOnLineSegment3D(referencePosition, vertex1, vertex2);
	Vec3 closestPointOnEdge2 = GetNearestPointOnLineSegment3D(referencePosition, vertex2, vertex0);

	// Return the closest of the three points
	float distToEdge0 = (closestPointOnEdge0 - projectedPoint).GetLengthSquared();
	float distToEdge1 = (closestPointOnEdge1 - projectedPoint).GetLengthSquared();
	float distToEdge2 = (closestPointOnEdge2 - projectedPoint).GetLengthSquared();

	if (distToEdge0 < distToEdge1 && distToEdge0 < distToEdge2)
	{
		return closestPointOnEdge0;
	}
	else if (distToEdge1 < distToEdge2)
	{
		return closestPointOnEdge1;
	}
	else
	{
		return closestPointOnEdge2;
	}
}

Vec3 const GetNearestPointInTriangle3D(Vec3 const& referencePosition, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2)
{
	// Project the reference point onto the triangle's plane
	Vec3 edge0 = vertex1 - vertex0;
	Vec3 edge1 = vertex2 - vertex0;
	Vec3 normal = CrossProduct3D(edge0, edge1).GetNormalized();
	float distance = DotProduct3D(normal, vertex0);
	Plane3D trianglePlane = Plane3D(normal, distance);

	Vec3 projectedPoint = GetNearestPointOnPlane3D(referencePosition, trianglePlane);

	// Check if the projected point is inside the triangle
	float u, v;
	if (IsPointInsideTriangle3D(projectedPoint, vertex0, vertex1, vertex2, u, v))
	{
		return projectedPoint; // If the point is inside the triangle, then that's the nearest point
	}

	// Clamp barycentric coordinates to ensure that the point stays inside the triangle
	u = GetClamped(u, 0.f, 1.f);
	v = GetClamped(v, 0.f, 1.f);
	float w = 1.f - u - v; // Ensure barycentric coordinates sum to 1

	// Compute & return the nearestPoint inside the triangle 
	return vertex0 * w + vertex1 * u + vertex2 * v;
}

Vec3 const GetNearestPointOnLineSegment3D(Vec3 const& referencePosition, Vec3 const& lineSegStart, Vec3 const& lineSegEnd)
{
	Vec3 segmentDistance = lineSegEnd - lineSegStart;

	if (segmentDistance.GetLengthSquared() < 1e-6f)
	{
		return lineSegStart;
	}

	Vec3 startDistPoint = referencePosition - lineSegStart;

	if (DotProduct3D(startDistPoint, segmentDistance) <= 0.f)
	{
		return lineSegStart;
	}

	Vec3 endDistPoint = referencePosition - lineSegEnd;

	if (DotProduct3D(endDistPoint, segmentDistance) >= 0.f)
	{
		return lineSegEnd;
	}

	segmentDistance.GetNormalized();
	Vec3 displacement = GetProjectedOnto3D(startDistPoint, segmentDistance);

	if (!displacement.IsValid())
	{
		return lineSegStart;
	}

	return lineSegStart + displacement;
}

bool PushSphereOutOfFixedPoint3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3 const& fixedPoint)
{
	// Calculate the vector from the fixed point to the sphere center
	Vec3 displacement = mobileSphereCenter - fixedPoint;

	// Calculate the distance between the fixed point and the sphere center
	float distance = GetDistance3D(fixedPoint, mobileSphereCenter);

	if (distance == 0.f)
	{
		// give an arbitrary direction e.g., up
		displacement = Vec3(0.f, 0.f, sphereRadius);
		distance = sphereRadius;
	}

	// Check if the sphere is overlapping with the fixed point
	if (distance < sphereRadius)
	{
		// Calculate the displacement needed to push the sphere out of the fixed point
		float overlap = sphereRadius - distance;
		displacement = (displacement / distance) * overlap;

		// Update the sphere center to the new position
		mobileSphereCenter += displacement;

		return true; // The sphere was pushed out of the fixed point
	}

	return false; // The disc is not overlapping with the fixed point
}

bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	// Calculate the vector from the fixed point to the disc center
	Vec2 displacement = mobileDiscCenter - fixedPoint;

	// Calculate the distance between the fixed point and the disc center
	float distance = GetDistance2D(fixedPoint, mobileDiscCenter);
	
	if (distance == 0)
	{
		// give an arbitrary direction e.g., to the right
		displacement.x = discRadius;
		displacement.y = 0.f;
		distance = discRadius;
		/*return false;*/
	}

	// Check if the disc is overlapping with the fixed point
	if (distance < discRadius) 
	{
		// Calculate the displacement needed to push the disc out of the fixed point
		float overlap = discRadius - distance;
		displacement.x = (displacement.x / distance) * overlap;
		displacement.y = (displacement.y / distance) * overlap;

		// Update the disc center to the new position
		mobileDiscCenter += displacement;

		return true; // The disc was pushed out of the fixed point
	}

	return false; // The disc is not overlapping with the fixed point
}

bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	// Calculate the squared distance between the centers of the two discs
	float squaredDistance = GetDistanceSquared2D(mobileDiscCenter, fixedDiscCenter);

	// Calculate the sum of the radii squared
	float sumOfRadiiSquared = (mobileDiscRadius + fixedDiscRadius) * (mobileDiscRadius + fixedDiscRadius);

	// Check if the discs are overlapping (compare squared values for efficiency)
	if (squaredDistance < sumOfRadiiSquared) 
	{
		// Calculate the normalized displacement vector
		Vec2 displacement = fixedDiscCenter - mobileDiscCenter;
		displacement.Normalize();

		// Calculate the overlap amount
		float overlap = sqrtf(sumOfRadiiSquared) - sqrtf(squaredDistance);

		// Update the mobile disc center to be pushed out of the fixed disc
		mobileDiscCenter.x -= overlap * displacement.x;
		mobileDiscCenter.y -= overlap * displacement.y;

		return true; // The mobile disc was pushed out of the fixed disc
	}

	return false; // The discs are not overlapping
}

bool PushDiscOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	// Calculate the vector from the center of disc A to the center of disc B
	Vec2 displacement = bCenter - aCenter;

	// Calculate the distance between the centers of the two discs
	float distance = displacement.GetLength();

	// Total radius between the two disc
	float totalRadius = aRadius + bRadius;

	// Check if the discs are overlapping
	if (distance < totalRadius) 
	{
		// Calculate the normalized displacement vector
		displacement.Normalize();

		// Calculate the overlap amount
		float overlap = (aRadius + bRadius) - distance;

		// Adjust the positions of both discs
		// Multiply by 0.5f to evenly  distribute the overlap correction between the two discs when they overlap. It ensures that each disc is moved by half of the overlap distance.
		aCenter.x -= 0.5f * overlap * displacement.x;
		aCenter.y -= 0.5f * overlap * displacement.y;

		bCenter.x += 0.5f * overlap * displacement.x;
		bCenter.y += 0.5f * overlap * displacement.y;

		return true; // Both discs were pushed out of each other
	}

	return false; // The discs are not overlapping
}

bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 newDisc = fixedBox.GetNearestPoint(mobileDiscCenter);

	if (!PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, newDisc))
	{
		return false;
	}

	return true;
}

bool PushDiscOutOfFixedCapsule2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& boneStart, Vec2 const& boneEnd, float capsuleRadius, float* outDisplacement /*= nullptr*/)
{
	Vec2 nearestPoint = GetNearestPointOnCapsule2D(mobileDiscCenter, boneStart, boneEnd, capsuleRadius);
	Vec2 originalDiscCenter = mobileDiscCenter;
	if (!PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPoint))
	{
		return false;
	}
	if (outDisplacement)
	{
		*outDisplacement = (mobileDiscCenter - originalDiscCenter).GetLength();
	}
	return true;
}

bool PushDiscOutOfFixedCapsule2D(Vec2& mobileDiscCenter, float discRadius, Capsule2 const& capsule)
{
	Vec2 nearestPoint = GetNearestPointOnCapsule2D(mobileDiscCenter, capsule);
	if (IsPointInsideDisc2D(nearestPoint, mobileDiscCenter, discRadius))
	{
		PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPoint);
		return true;
	}
	return false;
}

bool PushDiscOutOfFixedOBB2D(Vec2& mobileDiscCenter, float discRadius, OBB2 const& fixedBox)
{
	Vec2 nearestPoint = GetNearestPointOnOBB2D(mobileDiscCenter, fixedBox);
	if (IsPointInsideDisc2D(nearestPoint, mobileDiscCenter, discRadius))
	{
		PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPoint);
		return true;
	}
	return false;
}

bool PushSphereOutOfFixedPlane(Vec3& mobleSphereCenter, float sphereRadius, Plane3D const& fixedPlane)
{
	Vec3 nearestPoint = GetNearestPointOnPlane3D(mobleSphereCenter, fixedPlane);
	if (IsPointInsideSphere(nearestPoint, mobleSphereCenter, sphereRadius))
	{
		PushSphereOutOfFixedPoint3D(mobleSphereCenter, sphereRadius, nearestPoint);
		return true;
	}
	return false;
}

bool PushBottomZCapsuleOutOfFixedPoint(Vec3& mobileCapsuleStart, float capsuleRadius, Vec3 const& fixedPoint)
{
	// Calculate the vector from the fixed point to the bottom hemisphere center (capsuleStart)
	Vec3 displacement = mobileCapsuleStart - fixedPoint;

	// Calculate the distance between the fixed point and the bottom hemisphere center
	float distance = GetDistance3D(fixedPoint, mobileCapsuleStart);

	// Handle the case where the fixed point coincides with the hemisphere center
	if (distance == 0)
	{
		// Provide an arbitrary direction (for example, pushing upwards along Z-axis)
		displacement = Vec3(0.f, 0.f, capsuleRadius);
		distance = capsuleRadius;
	}

	// Check if the fixed point is inside the bottom hemisphere
	if (distance < capsuleRadius)
	{
		// Calculate how much the fixed point is overlapping with the bottom hemisphere
		float overlap = capsuleRadius - distance;

		// Normalize the displacement vector and scale it by the overlap
		displacement = (displacement / distance) * overlap;

		// Update the position of the bottom hemisphere center (capsuleStart)
		mobileCapsuleStart += displacement;

		return true; // The bottom hemisphere was pushed out of the fixed point
	}

	return false;
}

bool PushBottomZCapsuleOutOfFixedPlane(Vec3& mobileCapsuleStart, float capsuleRadius, Plane3D const& fixedPlane)
{
	Vec3 nearestPoint = GetNearestPointOnPlane3D(mobileCapsuleStart, fixedPlane);
	if (IsPointInsideBottomZCapsule(nearestPoint, mobileCapsuleStart, capsuleRadius))
	{
		PushBottomZCapsuleOutOfFixedPoint(mobileCapsuleStart, capsuleRadius, nearestPoint);
		return true;
	}
	return false;

}

// Transform Utilities
Vec2 RotatePointAroundPivot(Vec2 point, Vec2 pivot, float angleRadians)
{
	Vec2 relative = point - pivot;
	float x = relative.x * cosf(angleRadians) - relative.y * sinf(angleRadians);
	float y = relative.x * sinf(angleRadians) + relative.y * cosf(angleRadians);
	return pivot + Vec2(x, y);
}

void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation)
{
	posToTransform.x *= uniformScale;
	posToTransform.y *= uniformScale;

	float rotationRadians = (pi / 180.f) * rotationDegrees;
	float distXY = sqrtf((posToTransform.x * posToTransform.x) + (posToTransform.y * posToTransform.y));
	float thetaRadians = atan2f(posToTransform.y, posToTransform.x);
	thetaRadians += rotationRadians;

	posToTransform.x = translation.x + (distXY * cosf(thetaRadians));
	posToTransform.y = translation.y + (distXY * sinf(thetaRadians));
}

void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	float worldSpaceX = translation.x + ((posToTransform.x * iBasis.x)) + ((posToTransform.y * jBasis.x));
	float worldSpaceY = translation.y + ((posToTransform.x * iBasis.y)) + ((posToTransform.y * jBasis.y));

	posToTransform = Vec2(worldSpaceX, worldSpaceY);
}

void TransformPositionXY3D(Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)
{
	posToTransform.x *= scaleXY;
	posToTransform.y *= scaleXY;

	float rotationRadians = (pi / 180.f) * zRotationDegrees;
	float distXY = sqrtf((posToTransform.x * posToTransform.x) + (posToTransform.y * posToTransform.y));
	float thetaRadians = atan2f(posToTransform.y, posToTransform.x);
	thetaRadians += rotationRadians;

	posToTransform.x = translationXY.x + (distXY * cosf(thetaRadians));
	posToTransform.y = translationXY.y + (distXY * sinf(thetaRadians));
}

void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	float worldSpaceX = translation.x + ((posToTransform.x * iBasis.x)) + ((posToTransform.y * jBasis.x));
	float worldSpaceY = translation.y + ((posToTransform.x * iBasis.y)) + ((posToTransform.y * jBasis.y));

	posToTransform.x = worldSpaceX;
	posToTransform.y = worldSpaceY;
}

Mat44 GetBillboardMatrix(BillBoardType billboardType, Mat44 const& cameraMatrix, const Vec3& billboardPosition, const Vec2& billboardScale /*= Vec2(1.f, 1.f)*/)
{
	Mat44 billboardMatrix;

	Vec3 cameraForward = cameraMatrix.GetIBasis3D().GetNormalized();
	Vec3 cameraRight = cameraMatrix.GetJBasis3D().GetNormalized();
	Vec3 cameraUp = cameraMatrix.GetKBasis3D().GetNormalized();
	Vec3 cameraPos = cameraMatrix.GetTranslation3D();

	if (billboardType == BillBoardType::WORLD_UP_CAMERA_FACING)
	{
		Vec3 worldUp(0.f, 0.f, 1.f);
		Vec3 axis = cameraMatrix.GetTranslation3D() - billboardPosition;
		Vec3 iBasis = axis.GetNormalized() * billboardScale.x;
 		Vec3 right = CrossProduct3D(worldUp, iBasis) * billboardScale.y;

		billboardMatrix.SetIJKT3D(iBasis, right, worldUp, billboardPosition);
	}
	else if (billboardType == BillBoardType::WORLD_UP_CAMERA_OPPOSING)
	{
		Vec3 worldUp(0.f, 0.f, 1.f);
		Vec3 backward = -cameraForward;
		Vec3 right = CrossProduct3D(worldUp, backward).GetNormalized() * billboardScale.x;
		Vec3 up = CrossProduct3D(backward, right) * billboardScale.y;

		billboardMatrix.SetIJKT3D(cameraForward, right, up, billboardPosition);
	}
	else if (billboardType == BillBoardType::FULL_CAMERA_FACING)
	{
		// Match the camera orientation
		billboardMatrix.SetIJKT3D(cameraForward, cameraRight * billboardScale.x, cameraUp * billboardScale.y, billboardPosition);
	}
	else if (billboardType == BillBoardType::FULL_CAMERA_OPPOSING)
	{
		// Use the camera's orientation but with forward and right negated
		billboardMatrix.SetIJKT3D(-cameraForward, -cameraRight, cameraUp, billboardPosition);
	}

	return billboardMatrix;
}

float ComputeCubicBezier1D(float A, float B, float C, float D, float t)
{
	// First level of lerp
	float AB = Interpolate(A, B, t);
	float BC = Interpolate(B, C, t);
	float CD = Interpolate(C, D, t);

	// Second level of lerp
	float ABC = Interpolate(AB, BC, t);
	float BCD = Interpolate(BC, CD, t);

	// Final lerp
	float ABCD = Interpolate(ABC, BCD, t);

	return ABCD;
}

float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
{
	// First level of lerp
	float AB = Interpolate(A, B, t);
	float BC = Interpolate(B, C, t);
	float CD = Interpolate(C, D, t);
	float DE = Interpolate(D, E, t);
	float EF = Interpolate(E, F, t);

	// Second level of lerp
	float ABC = Interpolate(AB, BC, t);
	float BCD = Interpolate(BC, CD, t);
	float CDE = Interpolate(CD, DE, t);
	float DEF = Interpolate(DE, EF, t);

	// Third level of lerp
	float ABCD = Interpolate(ABC, BCD, t);
	float BCDE = Interpolate(BCD, CDE, t);
	float CDEF = Interpolate(CDE, DEF, t);

	// Fourth level of lerp
	float ABCDE = Interpolate(ABCD, BCDE, t);
	float BCDEF = Interpolate(BCDE, CDEF, t);

	// Final lerp
	float ABCDEF = Interpolate(ABCDE, BCDEF, t);

	return ABCDEF;
}

float SmoothStart2(float t)
{
	return t * t;
}

float SmoothStart3(float t)
{
	return t * t * t;
}

float SmoothStart4(float t)
{
	return t * t * t * t;
}

float SmoothStart5(float t)
{
	return t * t * t * t * t;
}

float SmoothStart6(float t)
{
	return t * t * t * t * t * t;
}

float SmoothStop2(float t)
{
	float u = 1 - t;
	return 1 - (u * u);
}

float SmoothStop3(float t)
{
	float u = 1 - t;
	return 1 - (u * u * u);
}

float SmoothStop4(float t)
{
	float u = 1 - t;
	return 1 - (u * u * u * u);
}

float SmoothStop5(float t)
{
	float u = 1 - t;
	return 1 - (u * u * u * u * u);
}

float SmoothStop6(float t)
{
	float u = 1 - t;
	return 1 - (u * u * u * u * u * u);
}

float SmoothStep3(float t)
{
	return Interpolate(SmoothStart2(t), SmoothStop2(t), t);
}

float SmoothStep5(float t)
{
	return ComputeQuinticBezier1D(0, 0, 0, 1, 1, 1, t);
}

float Hesitate3(float t)
{
	return ComputeCubicBezier1D(0, 1, 0, 1, t);
}

float Hesitate5(float t)
{
	return ComputeQuinticBezier1D(0, 1, 0, 1, 0, 1, t);
}

float CustomFunkyEasingFunction(float t)
{
	float result = 0.0f; // This will accumulate the total result of the sine waves.

	const int numberOfWaves = 5;

	// Define the base frequency of the first sine wave.
	const float baseFrequency = 2.0f;
	if (t <= 0.25f)
	{
		float frequency1 = 5.f; // Frequency of the oscillation
		float amplitude1 = 0.25f; // Amplitude of the oscillation
		float cubicEasing1 = t * t * (3 - 2 * t); // Simple cubic easing
		float oscillation1 = amplitude1 * SinDegrees(frequency1 * pi * t); // Oscillation factor using 
		result = cubicEasing1 + oscillation1 * (1 - t);
		return result;
	}
	else if (t <= 0.8f)
	{
		// Loop over the number of waves to sum them up
		for (int i = 0; i < numberOfWaves; i++)
		{
			float frequency = baseFrequency * (i + 1);
			result += sin(5.0f * pi * frequency * t);
		}
		result = (result + numberOfWaves) / (2 * numberOfWaves);
		return result;
	}
	else
	{
		float frequency2 = 5.f; 
		float amplitude2 = 0.25f; 
		float cubicEasing2 = t * t * (3 - 2 * t); // Simple cubic easing
		float oscillation2 = amplitude2 * SinDegrees(frequency2 * pi * t); 
		result = cubicEasing2 + oscillation2 * (1 - t);
		return result;
	}
}
