#include "Engine/Math/Capsule2.hpp"


Capsule2::Capsule2(Vec2 const& startPos, Vec2 const& endPos, float radius)
{
	m_start = startPos;
	m_end = endPos;
	m_radius = radius;
}

Capsule2::~Capsule2()
{
}