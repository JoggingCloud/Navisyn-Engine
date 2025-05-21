#include "Engine/Math/OBB2.hpp"

OBB2::OBB2(Vec2 const& center, Vec2 const& iBasisNormal, Vec2 const& halfDimensions)
{
	m_center = center;
	m_iBasisNormal = iBasisNormal;
	m_halfDimensions = halfDimensions;
}

OBB2::~OBB2()
{
}