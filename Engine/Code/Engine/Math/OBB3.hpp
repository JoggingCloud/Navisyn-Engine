#pragma once
#include "Engine/Math/Vec3.hpp"

class OBB3
{
public:
	OBB3() = default;
	OBB3(Vec3 const& center, Vec3 const& iBasisNormal, Vec3 const& jBasisNormal, Vec3 const& kBasisNormal, Vec3 const& halfDimensions);
	~OBB3();

public:
	Vec3 m_center;
	Vec3 m_iBasisNormal;
	Vec3 m_jBasisNormal;
	Vec3 m_kBasisNormal;
	Vec3 m_halfDimensions;
};