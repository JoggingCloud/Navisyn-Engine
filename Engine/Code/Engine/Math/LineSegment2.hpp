#pragma once
#include "Engine/Math/Vec2.hpp"

struct Rgba8;

class LineSegment2
{
public:
	LineSegment2(Vec2 const& startPos, Vec2 const& endPos);
	LineSegment2() = default;
	~LineSegment2();

public:
	Vec2 m_start;
	Vec2 m_end;
};
