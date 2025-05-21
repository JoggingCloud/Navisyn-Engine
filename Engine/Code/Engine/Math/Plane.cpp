#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"

Plane3D::Plane3D(Vec3 const& normal, float distance)
	: m_normal(normal), m_distance(distance)
{
}

float Plane3D::GetAltitudeOfPoint(Vec3 const& point) const
{
	return DotProduct3D(point, m_normal) - m_distance;
}

Plane2D::Plane2D(Vec2 const& normal, float distance)
	: m_normal(normal), m_distance(distance)
{
}

float Plane2D::GetAltitudeOfPoint(Vec2 const& point) const
{
	return DotProduct2D(point, m_normal) - m_distance;
}

bool Plane2D::IsPointInFrontOfPlane(Vec2 const& point)
{
	return GetAltitudeOfPoint(point) > 0.f;
}
