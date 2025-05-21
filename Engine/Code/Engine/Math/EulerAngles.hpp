#pragma once
#include "Engine/Math/Mat44.hpp"

struct EulerAngles 
{
public:
	static const EulerAngles ZERO;

public:
	EulerAngles() = default;
	EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees);
	void GetAsVectors_IFwd_JLeft_KUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis);
	Mat44 GetAsMatrix_IFwd_JLeft_KUp() const; // Given the values, what would I, J, K be?
	static EulerAngles GetFromMatrix(Mat44 const& matrix);
	Vec3 GetForwardVector() const;
	Vec3 GetRightVector() const;
	Vec3 GetLeftVector() const;
	Vec3 GetUpVector() const;
	void SetFromText(char const* text);

	void		operator+=(const EulerAngles& eulerToAdd); // EulerAngle += EulerAngle
	EulerAngles operator*(float scalar) const;

public:
	float m_yawDegrees = 0.f;
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;
};