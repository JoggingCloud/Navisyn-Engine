#pragma once
#include "Engine/Math/MathUtils.hpp"

struct RaycastResult2D
{
	// Basic ray cast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec2	m_impactPos;
	Vec2	m_impactNormal;

	// Original ray cast information (optional)
	Vec2	m_rayFwdNormal;
	Vec2	m_rayStartPos;
	float	m_rayMaxLength = 1.f;
};

RaycastResult2D RaycastVsDisc2D( Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius );
RaycastResult2D RaycastVsAABB2D( Vec2 startPos, Vec2 fwdNormal, float raycastLength, AABB2 const& box );
RaycastResult2D RaycastVsLineSegment2D( Vec2 startPos, Vec2 fwdNormal, float raycastLength, Vec2 lineSegmentStart, Vec2 lineSegmentEnd );
RaycastResult2D RaycastVsPlane2D(Vec2 startPos, Vec2 fwdNormal, float raycastLength, Plane2D const& plane );
RaycastResult2D RaycastVsConvexHull2D(Vec2 startPos, Vec2 fwdNormal, float raycastLength, ConvexHull2D const& convexHull );

struct RaycastResult3D 
{
	// Basic ray cast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;
	IntVec2 m_impactTileCoord = IntVec2::ZERO;

	// Original ray cast information (optional)
	Vec3	m_rayFwdNormal;
	Vec3	m_rayStartPos;
	float	m_rayMaxLength = 1.f;
};

RaycastResult3D RaycastVsAll(Vec3 startPos, Vec3 fwdNormal, float raycastLength, 
	AABB3 box = AABB3(Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f)),
	OBB3 orientedBox = OBB3(
		Vec3(0.0f, 0.0f, 0.0f),   // Center
		Vec3(1.0f, 0.0f, 0.0f),   // iBasis (x-axis)
		Vec3(0.0f, 1.0f, 0.0f),   // jBasis (y-axis)
		Vec3(0.0f, 0.0f, 1.0f),   // kBasis (z-axis)
		Vec3(0.5f, 0.5f, 0.5f)    // Half dimensions
	),
	Vec3 sphereCenter = Vec3(0.0f, 0.0f, 0.0f),
	float sphereRadius = 1.f,
	Vec2 cylinderCenter = Vec2(0.0f, 0.0f), FloatRange cylinderZRange = FloatRange(0.f, 1.f),
	float cylinderRadius = 1.f);

RaycastResult3D RaycastVsAABB3D(Vec3 startPos, Vec3 fwdNormal, float raycastLength, AABB3 box);
RaycastResult3D RaycastVsSphere(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Vec3 sphereCenter, float sphereRadius);
RaycastResult3D RaycastVsHemisphere(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Vec3 hemisphereCenter, Vec3 hemisphereNormal, float hemisphereRadius);
RaycastResult3D RaycastVsZCylinder(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius);
RaycastResult3D RaycastVsZCapsule(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Capsule3 capsule, float capsuleRadius);
RaycastResult3D RaycastVsOBB3D(Vec3 startPos, Vec3 fwdNormal, float raycastLength, OBB3 box);
RaycastResult3D RaycastVsPlane3D(Vec3 startPos, Vec3 fwdNormal, float raycastLength, Plane3D plane);

struct BoxCastResult3D
{
	// Basic box cast result information (required)
	bool m_didImpact = false;
	float m_impactDist = 0.0f;
	Vec3 m_impactPos;
	Vec3 m_impactNormal;
	IntVec2 m_impactTileCoord = IntVec2::ZERO;

	// Original box cast information (optional)
	Vec3 m_boxFwdNormal;
	Vec3 m_boxStartPos;
	float m_maxBoxLength = 1.0f;

	// Additional box cast information
	Vec3 m_boxHalfExtents;
//	Mat44 m_boxOrientation;
};

BoxCastResult3D BoxCastVsAABB3D(Vec3 startPos, Vec3 fwdNormal, float raycastLength, AABB3 const& box, Vec3 const& boxHalfExtents);