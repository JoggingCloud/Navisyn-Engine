#pragma once
#include "Engine/Math/Vec3.hpp"

struct Plane3D
{
	Plane3D() = default;
	Plane3D(Vec3 const& normal, float distance);
	float GetAltitudeOfPoint(Vec3 const& point) const;

	Vec3 m_normal;
	float m_distance;
};

struct Plane2D
{
	Plane2D() = default;
	Plane2D(Vec2 const& normal, float distance);
	float GetAltitudeOfPoint(Vec2 const& point) const;
	bool IsPointInFrontOfPlane(Vec2 const& point);

	Vec2 m_normal;
	float m_distance;
};