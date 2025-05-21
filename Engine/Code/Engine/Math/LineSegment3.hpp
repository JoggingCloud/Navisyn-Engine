#pragma once
#include "Engine/Math/Vec3.hpp"

struct Rgba8;

class LineSegment3
{
public:
	LineSegment3(Vec3 const& startPos, Vec3 const& endPos);
	LineSegment3() = default;
	~LineSegment3();

public:
	Vec3 m_start;
	Vec3 m_end;
};