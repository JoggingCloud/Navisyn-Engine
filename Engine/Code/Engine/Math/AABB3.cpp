#include "AABB3.hpp"

AABB3::AABB3()
{
}

AABB3::~AABB3()
{
}

AABB3::AABB3(AABB3 const& copyFrom)
	: m_mins(copyFrom.m_mins)
	, m_maxs(copyFrom.m_maxs)
{
}

AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	: m_mins(Vec3(minX, minY, minZ))
	, m_maxs(Vec3(maxX, maxY, maxZ))
{
}

AABB3::AABB3(Vec3 const& mins, Vec3 const& maxs)
	: m_mins(mins)
	, m_maxs(maxs)
{
}

bool AABB3::IsPointInside(Vec3 const& point) const
{
	if (point.x >= m_mins.x && point.x <= m_maxs.x && point.y >= m_mins.y && point.y <= m_maxs.y && point.z >= m_mins.z && point.z <= m_maxs.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Vec3 const AABB3::GetCenter() const
{
	return (m_maxs - m_mins) * 0.5f + m_mins;
}

Vec3 const AABB3::GetNearestPoint(Vec3 const& referencePosition) const
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

	float clampedZ = referencePosition.z; // Initialize the z
	if (clampedZ < m_mins.z) // Check if they are outside the boundaries 
	{
		clampedZ = m_mins.z; // set to corresponding value if they are 
	}
	else if (clampedZ > m_maxs.z)
	{
		clampedZ = m_maxs.z;
	}

	// Return the clamped position as the nearest point
	return Vec3(clampedX, clampedY, clampedZ);
}

Vec3 const AABB3::GetDimensions() const
{
	return m_maxs - m_mins;
}

Vec3 const AABB3::GetPointAtUV(Vec3 const& uv) const
{
	float x = m_maxs.x - m_mins.x;
	float y = m_maxs.y - m_mins.y;
	float z = m_maxs.z - m_mins.z;

	float i = x * uv.x + m_mins.x;
	float j = y * uv.y + m_mins.y;
	float k = z * uv.z + m_mins.z;

	return Vec3(i, j, k);
}

Vec3 const AABB3::GetUVForPoint(Vec3 const& point) const
{
	Vec3 dist = (m_maxs - m_mins);
	Vec3 newPoint = point - m_mins;

	float x = newPoint.x / dist.x;
	float y = newPoint.y / dist.y;
	float z = newPoint.z / dist.z;

	return Vec3(x, y, z);
}

void AABB3::Translate(Vec3 const& translationToApply)
{
	m_mins.x += translationToApply.x;
	m_maxs.x += translationToApply.x;

	m_mins.y += translationToApply.y;
	m_maxs.y += translationToApply.y;

	m_mins.z += translationToApply.z;
	m_maxs.z += translationToApply.z;
}

void AABB3::SetCenter(Vec3 const& newCenter)
{
	Translate(newCenter - GetCenter());
}

void AABB3::SetDimensions(Vec3 const& newDimensions)
{
	Vec3 currentCenter = GetCenter();

	m_mins.x = currentCenter.x - newDimensions.x * 0.5f;
	m_maxs.x = currentCenter.x + newDimensions.x * 0.5f;

	m_mins.y = currentCenter.y - newDimensions.y * 0.5f;
	m_maxs.y = currentCenter.y + newDimensions.y * 0.5f;

	m_mins.z = currentCenter.z - newDimensions.z * 0.5f;
	m_maxs.z = currentCenter.z + newDimensions.z * 0.5f;
}

void AABB3::StretchToIncludePoint(Vec3 const& point)
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

	if (m_mins.z > point.z)
	{
		m_mins.z = point.z;
	}
	if (m_maxs.z < point.z)
	{
		m_maxs.z = point.z;
	}
}

AABB3 AABB3::FromTriangle(Vec3 const& v0, Vec3 const& v1, Vec3 const& v2)
{
	Vec3 mins = Vec3::Min(Vec3::Min(v0, v1), v2);
	Vec3 maxs = Vec3::Max(Vec3::Max(v0, v1), v2);
	return AABB3(mins, maxs);
}
