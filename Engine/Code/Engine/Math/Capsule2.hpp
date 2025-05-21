#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/LineSegment2.hpp"

struct Rgba8;

class Capsule2
{
public:
	Capsule2(Vec2 const& startPos, Vec2 const& endPos, float radius);
	Capsule2() = default;
	~Capsule2();


public:
	Vec2 m_start;
	Vec2 m_end;
	float m_radius;
	//LineSegment2 m_bone;
};

