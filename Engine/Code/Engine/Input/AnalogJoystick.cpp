#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/XboxController.hpp"
#include <math.h>

AnalogJoystick::AnalogJoystick()
{

}

AnalogJoystick::~AnalogJoystick()
{

}

Vec2 AnalogJoystick::GetPosition() const
{
	return m_correctedPosition;
}

float AnalogJoystick::GetMagnitude() const
{
	float magnitude = sqrtf((m_correctedPosition.x * m_correctedPosition.x) + (m_correctedPosition.y * m_correctedPosition.y));
	return magnitude;
}

float AnalogJoystick::GetOrientationDegrees() const
{
	float orientation = ConvertRadiansToDegrees(atan2f(m_correctedPosition.y, m_correctedPosition.x));
	return orientation;
}

Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosiiton;
}

float AnalogJoystick::GetInnerDeadZoneFraction() const
{
	return m_innerDeadZoneFraction;
}

float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_outerDeadZoneFraction;
}

// For XboxController 
void AnalogJoystick::Reset()
{
	m_rawPosiiton = Vec2(0.0f,0.0f);
}

void AnalogJoystick::SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold)
{
	normalizedInnerDeadzoneThreshold = 0.3f;
	normalizedOuterDeadzoneThreshold = 0.95f;
}

void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
	// Normalize the raw position 
	m_rawPosiiton = Vec2(rawNormalizedX, rawNormalizedY);
	// Get the length of the raw joy stick
	float rawJoystick = m_rawPosiiton.GetLength();
	// 
	float joystickAngle = m_rawPosiiton.GetOrientationDegrees(); // Theta and R
	// Clamp the raw position of the joy stick
	float correctedLength = RangeMapClamped(rawJoystick, m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.f, 1.f);
	// Convert to polar coordinates 
	m_correctedPosition = Vec2::MakeFromPolarDegrees(joystickAngle, correctedLength);
}

