#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <math.h>
#include <cmath>

EulerAngles const EulerAngles::ZERO = EulerAngles(0.f, 0.f, 0.f);

EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees)
	: m_yawDegrees(yawDegrees), m_pitchDegrees(pitchDegrees), m_rollDegrees(rollDegrees)
{

}

void EulerAngles::GetAsVectors_IFwd_JLeft_KUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis)
{
	Mat44 rotation = GetAsMatrix_IFwd_JLeft_KUp();
	out_forwardIBasis = Vec3(rotation.m_values[rotation.Ix], rotation.m_values[rotation.Iy], rotation.m_values[rotation.Iz]);
	out_leftJBasis = Vec3(rotation.m_values[rotation.Jx], rotation.m_values[rotation.Jy], rotation.m_values[rotation.Jz]);
	out_upKBasis = Vec3(rotation.m_values[rotation.Kx], rotation.m_values[rotation.Ky], rotation.m_values[rotation.Kz]);
}

Mat44 EulerAngles::GetAsMatrix_IFwd_JLeft_KUp() const
{
	Mat44 rotation;
	rotation.AppendZRotation(m_yawDegrees);
	rotation.AppendYRotation(m_pitchDegrees);
	rotation.AppendXRotation(m_rollDegrees);

	return rotation;
}

EulerAngles EulerAngles::GetFromMatrix(Mat44 const& matrix)
{
	EulerAngles angles;

	float yaw = Atan2Degrees(matrix.m_values[Mat44::Iy], matrix.m_values[Mat44::Ix]);
	angles.m_yawDegrees = yaw;
	
	float roll = Atan2Degrees(matrix.m_values[Mat44::Jz], matrix.m_values[Mat44::Kz]);
	angles.m_rollDegrees = roll;

	float pitch = Atan2Degrees(-matrix.m_values[Mat44::Kx],	std::sqrt(matrix.m_values[Mat44::Ix] * matrix.m_values[Mat44::Ix] + 
		matrix.m_values[Mat44::Jx] * matrix.m_values[Mat44::Jx]));
	angles.m_pitchDegrees = pitch;

	return angles;
}

Vec3 EulerAngles::GetForwardVector() const
{
	return GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D();
}

Vec3 EulerAngles::GetRightVector() const
{
	return -GetAsMatrix_IFwd_JLeft_KUp().GetJBasis3D();
}

Vec3 EulerAngles::GetLeftVector() const
{
	return GetAsMatrix_IFwd_JLeft_KUp().GetJBasis3D();
}

Vec3 EulerAngles::GetUpVector() const
{
	return GetAsMatrix_IFwd_JLeft_KUp().GetKBasis3D();
}

void EulerAngles::SetFromText(char const* text)
{
	Strings components = SplitStringOnDelimiter(text, ',');

	if (components.size() == 3)
	{
		m_yawDegrees = static_cast<float>(std::stof(components[0].c_str()));
		m_pitchDegrees = static_cast<float>(std::stof(components[1].c_str()));
		m_rollDegrees = static_cast<float>(std::stof(components[2].c_str()));
	}
	else
	{
		m_yawDegrees = m_pitchDegrees = m_rollDegrees = 0.0f;
	}
}

EulerAngles EulerAngles::operator*(float scalar) const
{
	return EulerAngles(m_yawDegrees * scalar, m_pitchDegrees * scalar, m_rollDegrees * scalar);
}

void EulerAngles::operator+=(const EulerAngles& eulerToAdd)
{
	m_yawDegrees += eulerToAdd.m_yawDegrees;
	m_pitchDegrees += eulerToAdd.m_pitchDegrees;
	m_rollDegrees += eulerToAdd.m_rollDegrees;
}
