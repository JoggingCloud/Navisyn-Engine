#pragma once
#include "Engine/Math/Vec2.hpp"

class OBB2
{
public:
	OBB2() = default;
	OBB2(Vec2 const& center, Vec2 const& iBasisNormal, Vec2 const& halfDimensions);
	~OBB2();

public:
	Vec2 m_center;
	Vec2 m_iBasisNormal;
	Vec2 m_halfDimensions;

private:

};

