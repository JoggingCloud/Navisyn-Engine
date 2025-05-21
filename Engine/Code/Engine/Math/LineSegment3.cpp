#include "Engine/Math/LineSegment3.hpp"

LineSegment3::LineSegment3(Vec3 const& startPos, Vec3 const& endPos)
	: m_start(startPos), m_end(endPos)
{
}

LineSegment3::~LineSegment3()
{
}
