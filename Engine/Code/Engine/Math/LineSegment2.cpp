#include "Engine/Math/LineSegment2.hpp"

LineSegment2::LineSegment2(Vec2 const& startPos, Vec2 const& endPos)
{
	m_start = startPos;
	m_end = endPos;
}

LineSegment2::~LineSegment2()
{
}