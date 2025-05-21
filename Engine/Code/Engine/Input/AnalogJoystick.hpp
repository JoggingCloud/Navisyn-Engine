#pragma once
#include "Engine/Math/Vec2.hpp"

class AnalogJoystick
{
public:
	enum XboxButtonID
	{
		XBOXBUTTON_A,
		XBOXBUTTON_B,
		XBOXBUTTON_X,
		XBOXBUTTON_Y,
		XBOXBUTTON_DPAD_RIGHT,
		XBOXBUTTON_DPAD_UP,
		XBOXBUTTON_DPAD_LEFT,
		XBOXBUTTON_DPAD_DOWN,
		XBOXBUTTON_RIGHT_THUMB,
		XBOXBUTTON_LEFT_THUMB,
		XBOXBUTTON_BACK,
		XBOXBUTTON_START,
		XBOXBUTTON_LEFT_SHOULDER,
		XBOXBUTTON_RIGHT_SHOULDER,
		XBOXBUTTON_NUM
	};

public:
	AnalogJoystick();
	~AnalogJoystick();

public:
	Vec2 GetPosition() const;
	float GetMagnitude() const;
	float GetOrientationDegrees() const;

	Vec2 GetRawUncorrectedPosition()const;
	float GetInnerDeadZoneFraction()const;
	float GetOuterDeadZoneFraction()const;

	// For XboxController 
	void Reset();
	void SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold);
	void UpdatePosition(float rawNormalizedX, float rawNormalizedY);
	
public:
	float m_normRangeX;
	float m_normRangeY;

protected:
	Vec2 m_rawPosiiton;
	Vec2 m_correctedPosition;
	
	float m_innerDeadZoneFraction = 0.3f; // If R < this% R = 0; "input range start" for corrective range map
	float m_outerDeadZoneFraction = 0.95f; // If R > this% R = 1; "input range end" for corrective range map
};