#include "Engine/Math/AABB2.hpp"

AABB2 const AABB2::ZERO_TO_ONE = AABB2(Vec2(0.f,0.f), Vec2(1.f, 1.f));

AABB2::AABB2()
{

}

AABB2::~AABB2()
{

}

AABB2::AABB2(AABB2 const& copyFrom)

	:m_mins(copyFrom.m_mins)
	, m_maxs(copyFrom.m_maxs)
{
}

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: m_mins(Vec2(minX, minY))
	, m_maxs(Vec2(maxX, maxY))
{
}

AABB2::AABB2(Vec2 const& mins, Vec2 const maxs)
	:m_mins(mins)
	, m_maxs(maxs)
{
}

// Const Methods 
bool AABB2::IsPointInside(Vec2 const& point) const
{
	if (point.x >= m_mins.x && point.x <= m_maxs.x && point.y >= m_mins.y && point.y <= m_maxs.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Vec2 const AABB2::GetCenter() const
{
	return (m_maxs - m_mins) * 0.5f + m_mins;
}

Vec2 const AABB2::GetDimensions() const
{
	return m_maxs - m_mins;
}

Vec2 const AABB2::GetNearestPoint(Vec2 const& referencePosition) const
{
	float clampedX = referencePosition.x; // initialize the x
	if (clampedX < m_mins.x) // Check if they are outside the boundaries 
	{
		clampedX = m_mins.x; // set to corresponding value if they are 
	}
	else if (clampedX > m_maxs.x) 
	{
		clampedX = m_maxs.x;
	}

	float clampedY = referencePosition.y; // Initialize the y
	if (clampedY < m_mins.y) // Check if they are outside the boundaries 
	{
		clampedY = m_mins.y; // set to corresponding value if they are 
	}
	else if (clampedY > m_maxs.y) 
	{
		clampedY = m_maxs.y;
	}

	// Return the clamped position as the nearest point
	return Vec2(clampedX, clampedY);
}

Vec2 const AABB2::GetPointAtUV(Vec2 const& uv) const
{
	float x = m_maxs.x - m_mins.x;
	float y = m_maxs.y - m_mins.y;


	float i = x * uv.x + m_mins.x;
	float j = y * uv.y + m_mins.y;

	return Vec2(i, j);
}

Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
	Vec2 dist = (m_maxs - m_mins);
	Vec2 newPoint = point - m_mins;
	float x = newPoint.x / dist.x;
	float y = newPoint.y / dist.y;

	return Vec2(x, y);
}

AABB2 const AABB2::GetBoxAtUVs(Vec2 uvMins, Vec2 uvMaxs) const
{
	Vec2 boxMins = Vec2(m_mins.x + (m_maxs.x - m_mins.x) * uvMins.x, m_mins.y + (m_maxs.y - m_mins.y) * uvMins.y);
	Vec2 boxMaxs = Vec2(m_mins.x + (m_maxs.x - m_mins.x) * uvMaxs.x, m_mins.y + (m_maxs.y - m_mins.y) * uvMaxs.y);

	return AABB2(boxMins, boxMaxs);
}

// Non-const Methods 
void AABB2::Translate(Vec2 const& translationToApply)
{
	m_mins.x += translationToApply.x;
	m_maxs.x += translationToApply.x;

	m_mins.y += translationToApply.y;
	m_maxs.y += translationToApply.y;
}

void AABB2::SetCenter(Vec2 const& newCenter)
{
	Translate(newCenter - GetCenter());
}

void AABB2::SetDimensions(Vec2 const& newDimensions)
{
	Vec2 currentCenter = GetCenter();

	m_maxs.x = currentCenter.x + newDimensions.x * 0.5f;
	m_maxs.y = currentCenter.y + newDimensions.y * 0.5f;

	m_mins.x = currentCenter.x - newDimensions.x * 0.5f;
	m_mins.y = currentCenter.y - newDimensions.y * 0.5f;
}

void AABB2::StretchToIncludePoint(Vec2 const& point)
{
	if (m_mins.x > point.x)
	{
		m_mins.x = point.x;
	}
	if (m_maxs.x < point.x)
	{
		m_maxs.x = point.x;
	}

	if (m_mins.y > point.y)
	{
		m_mins.y = point.y;
	}
	if (m_maxs.y < point.y)
	{
		m_maxs.y = point.y;
	}
}

void AABB2::operator=(const AABB2& copyFrom)
{
	m_mins = copyFrom.m_mins;
	m_maxs = copyFrom.m_maxs;
}

void AABB2::operator+=(const AABB2& aabb2ToAdd)
{
	m_mins += aabb2ToAdd.m_mins;
	m_maxs += aabb2ToAdd.m_maxs;
}
