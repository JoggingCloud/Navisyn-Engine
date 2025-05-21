#pragma once
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/ConvexHull2.h"
#include "Engine/Core/Rgba8.hpp"

constexpr float pi = 3.14159265359f;
constexpr float EPSILON = 1e-6f;

struct Mat44;

enum class BillBoardType
{
	NONE = -1,
	WORLD_UP_CAMERA_FACING,
	WORLD_UP_CAMERA_OPPOSING,
	FULL_CAMERA_FACING,
	FULL_CAMERA_OPPOSING,
	COUNT
};

// Bezier Functions
float ComputeCubicBezier1D(float A, float B, float C, float D, float t);
float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t);

// Easing Functions
float SmoothStart2(float t); // EaseInQuadratic
float SmoothStart3(float t); // EaseInCubic
float SmoothStart4(float t); // EaseInQuartic
float SmoothStart5(float t); // EaseInQuintic
float SmoothStart6(float t); // EaseIn6thOrder
								 
float SmoothStop2(float t);	 // EaseOutQuadratic
float SmoothStop3(float t);	 // EaseOutCubic
float SmoothStop4(float t);	 // EaseOutQuartic
float SmoothStop5(float t);	 // EaseOutQuintic
float SmoothStop6(float t);	 // EaseOut6thOrder

float SmoothStep3(float t); // SmoothStep
float SmoothStep5(float t); // SmootherStep

float Hesitate3(float t); // Use Cubic Bezier1D
float Hesitate5(float t); // Use Quintic Bezier1D

float CustomFunkyEasingFunction(float t);

// Clamp & Lerp
Vec3 GetClamped(Vec3& value, Vec3& minValue, Vec3& maxValue);
Vec3 GetClamped(Vec3& value, float minValue, float maxValue);
float GetClamped(float value, float minValue, float maxValue);
float GetClampedZeroToOne(float value);
float NormalizeByte(unsigned char byte);
unsigned char DenormalizeByte(float f);
float NormalizeRgbaByte(unsigned char value);
unsigned char DenormalizeRgbaByte(float value);
Vec3 Interpolate(Vec3 start, Vec3 end, float t);
Rgba8 Interpolate(Rgba8 start, Rgba8 end, float fractionOfEnd);
float Interpolate(float start, float end, float t);
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd);
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
int RoundDownToInt(float value);

// Angle Utilities
float ConvertDegreesToRadians(float degrees);
float ConvertRadiansToDegrees(float radians);
float CosDegrees(float degrees);
float SinDegrees(float degrees);
float TanDegrees(float degrees);
float Atan2Degrees(float y, float x);
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees);
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees);
float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);
float GetAngleDegreesBetweenVectors3D(Vec3 const& a, Vec3 const& b);

// Dot and Cross
float DotProduct2D(Vec2 const& a, Vec2 const b);
float DotProduct3D(Vec3 const& a, Vec3 const b);
float DotProduct4D(Vec4 const& a, Vec4 const b);
float CrossProduct2D(Vec2 const& a, Vec2 const b);
Vec3 CrossProduct3D(Vec3 const& a, Vec3 const b);
Vec3 ZCrossProduct3D(Vec3 const& a, Vec3 const b); // Cross product function that only accounts for the Z

// IntVec2 Distance Utilities
float GetDistance2D(IntVec2 const& positionA, IntVec2 const& positionB);
int GetDistanceSquared2D(IntVec2 const& positionA, IntVec2 const& positionB);

// 2D Distance Utilities 
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB);
int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
int GetHexTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto); // Works if Vectors are not normalized
Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto); // Works if vectors are not normalized
float GetMagnitude2D(Vec2 const& vector);
float GetMagnitudeSquared2D(Vec2 const& vector);

// 3D Distance Utilities
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& posiitonB);
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float GetProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto); // Works if Vectors are not normalized
Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto); // Works if vectors are not normalized
float GetMagnitude3D(Vec3 const& vector);
float GetMagnitudeSquared3D(Vec3 const& vector);

// Geometric Query Utilities
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool IsPointInsideSphere(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius);
bool IsPointInsideHemisphere(Vec3 const& point, Vec3 const& hemisphereCenter, Vec3 const& hemisphereNormal, float hemisphereRadius);
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box);
bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box);
bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule);
bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox);
bool IsPointInsideOBB3D(Vec3 const& point, OBB3 const& orientedBox);
bool IsPointInsideZCylinder(Vec3 const& point, Vec2 const& cylinderCenterXY, FloatRange cylinderZRange, float radius);
bool IsPointInsideCylinder(Vec3 const& point, Vec3 const& startPos, Vec3 const& endPos, float radius);
bool IsPointInsideBottomZCapsule(Vec3 const& point, Vec3 const& capsuleStart, float capsuleRadius);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideTriangle3D(Vec3 const& point, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2);
bool IsPointInsideTriangle3D(Vec3 const& point, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2, float& outU, float& outV);
bool IsPointInsideHexagon3D(Vec3 const& point, Vec3 const& hexCenter, float hexRadius);
bool IsPointInsideConvexHull2D(Vec2 const& point, ConvexHull2D const& convexhull);

bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool DoAABB2DsOverlap(AABB2 const& boxA, AABB2 const& boxB);
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool DoAABB3DsOverlap(AABB3 const& boxA, AABB3 const& boxB);
bool DoZCylindersOverlap3D(Vec2 cylinder1CenterXY, float cylinder1Radius, FloatRange cylinder1MinMaxZ, Vec2 cylinder2CenterXY, float cylinder2Radius, FloatRange cylinder2MinMaxZ);
bool DoZCylinderAndAABB3DOverlap(Vec2 cylinderCenterXY, float cylinderRadius, FloatRange cylinderMinMaxZ, AABB3 box);
bool DoZCylinderAndSphereOverlap(Vec2 cylinderCenterXY, float cylinderRadius, FloatRange cylinderMinMaxZ, Vec3 sphereCenter, float sphereRadius);
bool DoSphereAndAABB3DOverlap(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& box);
bool DoOBB3DAndPlaneOverlap(OBB3 const& obb, Plane3D const& plane);
bool DoSphereAndPlaneOverlap(Vec3 const& sphereCenter, float sphereRadius, Plane3D const& plane);
bool DoPlaneAndZCapsuleBottomOverlap(Vec3 const& capsuleStart, float capsuleRadius, Plane3D const& plane);
bool DoLineSegmentIntersect3D(Vec3 const& lineAStart, Vec3 const& lineAEnd, Vec3 const& lineBStart, Vec3 const& lineBEnd, Vec3& outIntersection);

Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius);
Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePosition, AABB2& box);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, LineSegment2 const& infiniteLine);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePosition, Vec2 const& lineSegStart, Vec2 const& anotherPointOnLine);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePosition, LineSegment2 const& lineSegment);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePosition, Vec2 const& lineSegStart, Vec2 const& lineSegEnd);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePosition, Capsule2 const& capsule);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePosition, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePosition, OBB2 const& orientedBox);
Vec2 const GetNearestPointOnPlane2D(Vec2 const& referencePosition, Plane2D const& plane);

Vec3 const GetNearestPointOnSphere(Vec3 const& referencePosition, Vec3 const& sphereCenter, float sphereRadius);
Vec3 const GetNearestPointOnHemisphere(Vec3 const& referencePosition, Vec3 const& hemisphereCenter, float hemisphereRadius);
Vec3 const GetNearestPointOnAABB3D(Vec3 const& referencePosition, AABB3& box);
Vec3 const GetNearestPointOnZCylinder(Vec3 const& referencePosition, Vec2 const& cylinderCenterXY, FloatRange cylinderMinMaxZ, float radius);
Vec3 const GetNearestPointOnZAlignedCapsuleBottom(Vec3 const& capsuleStart, float capsuleRadius, Plane3D const& fixedPlane);
Vec3 const GetNearestPointOnOBB3D(Vec3 const& referencePosition, OBB3 const& orientedBox);
Vec3 const GetNearestPointOnPlane3D(Vec3 const& referencePosition, Plane3D const& plane);
Vec3 const GetNearestPointOnTriangle3D(Vec3 const& referencePosition, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2);
Vec3 const GetNearestPointOnTriangleEdge3D(Vec3 const& referencePosition, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2);
Vec3 const GetNearestPointInTriangle3D(Vec3 const& referencePosition, Vec3 const& vertex0, Vec3 const& vertex1, Vec3 const& vertex2);
Vec3 const GetNearestPointOnLineSegment3D(Vec3 const& referencePosition, Vec3 const& lineSegStart, Vec3 const& lineSegEnd);

bool PushSphereOutOfFixedPoint3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3 const& fixedPoint);
bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint);
bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
bool PushDiscOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);
bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox);
bool PushDiscOutOfFixedCapsule2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& boneStart, Vec2 const& boneEnd, float capsuleRadius, float* outDisplacement = nullptr);
bool PushDiscOutOfFixedCapsule2D(Vec2& mobileDiscCenter, float discRadius, Capsule2 const& capsule);
bool PushDiscOutOfFixedOBB2D(Vec2& mobileDiscCenter, float discRadius, OBB2 const& fixedBox);
bool PushSphereOutOfFixedPlane(Vec3& mobleSphereCenter, float sphereRadius, Plane3D const& fixedPlane);
bool PushBottomZCapsuleOutOfFixedPoint(Vec3& mobileCapsuleStart, float capsuleRadius, Vec3 const& fixedPoint);
bool PushBottomZCapsuleOutOfFixedPlane(Vec3& mobileCapsuleStart, float capsuleRadius, Plane3D const& fixedPlane);

// Transform Utilities 
Vec2 RotatePointAroundPivot(Vec2 point, Vec2 pivot, float angleRadians);
void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation);
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
void TransformPositionXY3D(Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY);
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);

Mat44 GetBillboardMatrix(BillBoardType billboardType, Mat44 const& cameraMatrix, const Vec3& billboardPosition, const Vec2& billboardScale = Vec2(1.f, 1.f));