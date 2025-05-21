#pragma once
#include "Engine/Math/Vec3.hpp"

struct AABB3 
{
public:
	Vec3 m_mins;
	Vec3 m_maxs;

public:
	AABB3();
	~AABB3();
	AABB3(AABB3 const& copyFrom);
	explicit AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	explicit AABB3(Vec3 const& mins, Vec3 const& maxs);

	// Const Methods 
	bool IsPointInside(Vec3 const& point) const;
	Vec3 const GetCenter() const;
	Vec3 const GetNearestPoint(Vec3 const& referencePosition) const;
	Vec3 const GetDimensions() const;
	Vec3 const GetPointAtUV(Vec3 const& uv) const; // uv = (0, 0) is at mins; uv = (1, 1) is at maxs
	Vec3 const GetUVForPoint(Vec3 const& point) const; // uv = (0.5, 0.5) at center; U or V outside [0, 1] extrapolated

	// Non-const Methods 
	void Translate(Vec3 const& translationToApply);
	void SetCenter(Vec3 const& newCenter);
	void SetDimensions(Vec3 const& newDimensions);
	void StretchToIncludePoint(Vec3 const& point); // Does minimal stretching required (none if already on point).

	static AABB3 FromTriangle(Vec3 const& v0, Vec3 const& v1, Vec3 const& v2);
};