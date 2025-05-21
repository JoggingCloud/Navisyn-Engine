#include "Engine/Math/OBB3.hpp"

OBB3::OBB3(Vec3 const& center, Vec3 const& iBasisNormal, Vec3 const& jBasisNormal, Vec3 const& kBasisNormal, Vec3 const& halfDimensions)
{
	m_center = center;
	m_iBasisNormal = iBasisNormal;
	m_jBasisNormal = jBasisNormal;
	m_kBasisNormal = kBasisNormal;
	m_halfDimensions = halfDimensions;
}

OBB3::~OBB3()
{
}