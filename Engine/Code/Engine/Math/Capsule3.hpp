#pragma once
#include "Engine/Math/Vec3.hpp"

class Capsule3
{
public:
	Capsule3(Vec3 const& startPos, Vec3 const& endPos, float radius);
	Capsule3() = default;
	~Capsule3();

	bool IsPointInsideCapsule3D(Vec3 const& point);

public:
	Vec3 m_start;
	Vec3 m_end;
	float m_radius;
};