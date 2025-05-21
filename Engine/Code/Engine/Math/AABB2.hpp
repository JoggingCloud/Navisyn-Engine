#pragma once
#include "Engine/Math/Vec2.hpp"

struct AABB2
{
public:
	Vec2 m_mins;
	Vec2 m_maxs;

	static const AABB2 ZERO_TO_ONE;

public:
	AABB2();
	~AABB2();
	AABB2(AABB2 const& copyFrom);
	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(Vec2 const& mins, Vec2 const maxs);


	// Const Methods 
	bool IsPointInside(Vec2 const& point) const;
	Vec2 const GetCenter() const;
	Vec2 const GetDimensions() const;
	Vec2 const GetNearestPoint(Vec2 const& referencePosition) const;
	Vec2 const GetPointAtUV(Vec2 const& uv) const; // uv = (0, 0) is at mins; uv = (1, 1) is at maxs
	Vec2 const GetUVForPoint(Vec2 const& point) const; // uv = (0.5, 0.5) at center; U or V outside [0, 1] extrapolated
	AABB2 const GetBoxAtUVs(Vec2 uvMins, Vec2 uvMaxs) const;

	// Non-const Methods 
	void Translate(Vec2 const& translationToApply);
	void SetCenter(Vec2 const& newCenter);
	void SetDimensions(Vec2 const& newDimensions);
	void StretchToIncludePoint(Vec2 const& point); // Does minimal stretching required (none if already on point).

	// Operators (self-mutating / non-const)
	void		operator+=(const AABB2& aabb2ToAdd); // AABB2 += AABB2
	void		operator=( const AABB2& copyFrom ); // AABB2 = AABB2
};