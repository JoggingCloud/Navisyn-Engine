#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/MathUtils.hpp"

Capsule3::Capsule3(Vec3 const& startPos, Vec3 const& endPos, float radius)
{
	m_start = startPos;
	m_end = endPos;
	m_radius = radius;
}

Capsule3::~Capsule3()
{
}

bool Capsule3::IsPointInsideCapsule3D(Vec3 const& point)
{
	Vec3 startToPoint = point - m_start;
	Vec3 startToEnd = m_end - m_start;
	float lenSq = startToEnd.GetLengthSquared();
	float dot = DotProduct3D(startToPoint, startToEnd);

	if (dot < 0.0f)
	{
		return startToPoint.GetLengthSquared() < m_radius * m_radius;
	}
	else if (dot > lenSq)
	{
		Vec3 endToPoint = point - m_end;
		return endToPoint.GetLengthSquared() < m_radius * m_radius;
	}
	else
	{
		float d = dot / lenSq;
		Vec3 closestPoint = m_start + d * startToEnd;
		return (closestPoint - point).GetLengthSquared() < m_radius * m_radius;
	}
}
