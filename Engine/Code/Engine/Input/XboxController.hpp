#pragma once
#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"



class XboxController
{
	friend class InputSystem;

public:
	// This is an issue
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
	XboxController();
	~XboxController();
	bool IsConnected() const;
	int GetControllerID() const;
	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
 	KeyButtonState const& GetButton(XboxButtonID buttonID) const;
 	bool IsButtonDown(XboxButtonID buttonID)const;
	bool WasButtonJustPressed(XboxButtonID buttonID) const;
	bool WasButtonJustReleased(XboxButtonID buttonID) const;

public:
	float m_rangeX;
	float m_rangeY;

private:
	void Update();
	void Reset();
	void UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY);
	void UpdateTrigger(float& out_triggerValue, unsigned char rawValue);
	void UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag);

private:
	int m_id = -1;
	bool m_isConnected = false;
	float m_leftTrigger = 0.f;
	float m_rightTrigger = 0.f;
	KeyButtonState m_buttons[(int)XboxButtonID::XBOXBUTTON_NUM];
	AnalogJoystick m_leftStick;
	AnalogJoystick m_rightStick;
};