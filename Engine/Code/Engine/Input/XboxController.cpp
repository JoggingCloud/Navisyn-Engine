#pragma comment( lib, "xinput9_1_0" ) 
#include <Windows.h>
#include <Xinput.h>
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"


XboxController::XboxController()
{

}

XboxController::~XboxController()
{

}

bool XboxController::IsConnected() const
{
	return m_isConnected;
}

int XboxController::GetControllerID() const
{
	// Remember its own id
	return m_id;
}

AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	return m_buttons[buttonID];
}

bool XboxController::IsButtonDown(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].m_isPressedThisFrame;
}

bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	if (!m_buttons[buttonID].m_wasPressedLastFrame && m_buttons[buttonID].m_isPressedThisFrame)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	m_buttons[buttonID].m_isPressedThisFrame;
	return false;
}

void XboxController::Update()
{
	XINPUT_STATE xboxControllerState = {};
	DWORD errorStatus = XInputGetState( m_id, &xboxControllerState );
	if (errorStatus != ERROR_SUCCESS)
	{
		Reset();
		m_isConnected = false;
		return;
	}
	m_isConnected = true;

	// Update internal data structure(s) based on raw controller state
	XINPUT_GAMEPAD const& state = xboxControllerState.Gamepad;

	UpdateJoystick(m_leftStick, state.sThumbLX, state.sThumbLY);
	UpdateJoystick(m_rightStick, state.sThumbRX, state.sThumbRY);

	UpdateTrigger(m_leftTrigger, state.bLeftTrigger);
	UpdateTrigger(m_rightTrigger, state.bRightTrigger);

	// This is an issue 
	UpdateButton(XBOXBUTTON_A,					state.wButtons, XINPUT_GAMEPAD_A);
	UpdateButton(XBOXBUTTON_B,					state.wButtons, XINPUT_GAMEPAD_B);
	UpdateButton(XBOXBUTTON_X,					state.wButtons, XINPUT_GAMEPAD_X);
	UpdateButton(XBOXBUTTON_Y,					state.wButtons, XINPUT_GAMEPAD_Y);
	UpdateButton(XBOXBUTTON_BACK,				state.wButtons, XINPUT_GAMEPAD_BACK);
	UpdateButton(XBOXBUTTON_START,				state.wButtons, XINPUT_GAMEPAD_START);
	UpdateButton(XBOXBUTTON_LEFT_SHOULDER,		state.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
	UpdateButton(XBOXBUTTON_RIGHT_SHOULDER,		state.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	UpdateButton(XBOXBUTTON_LEFT_THUMB,			state.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
	UpdateButton(XBOXBUTTON_RIGHT_THUMB,		state.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);
	UpdateButton(XBOXBUTTON_DPAD_RIGHT,			state.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
	UpdateButton(XBOXBUTTON_DPAD_UP,			state.wButtons, XINPUT_GAMEPAD_DPAD_UP);
	UpdateButton(XBOXBUTTON_DPAD_LEFT,			state.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
	UpdateButton(XBOXBUTTON_DPAD_DOWN,			state.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
}

void XboxController::Reset()
{
	m_rightTrigger = 0.0f;
	m_leftTrigger = 0.0f;
	m_leftStick.Reset();
	m_rightStick.Reset();
	for (int buttonID = 0; buttonID < NUM_XBOX_BUTTONS; buttonID++)
	{
		m_buttons->Reset();
	}
}

void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	// You want the GetClamped()
	// Stick max and min range in cartesian 
	m_rangeX = RangeMap(rawX, -32628.f, 32627.f, -1.f, 1.f);
	m_rangeY = RangeMap(rawY, -32628.f, 32627.f, -1.f, 1.f);
	// Get the length
	out_joystick.UpdatePosition(m_rangeX, m_rangeY);
}

void XboxController::UpdateTrigger(float& out_triggerValue, unsigned char rawValue)
{
	out_triggerValue = (float)rawValue / 255.f;
}

void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag)
{
	m_buttons[buttonID].m_wasPressedLastFrame = m_buttons[buttonID].m_isPressedThisFrame;
	if ((buttonFlag & buttonFlags) == buttonFlag)
	{
 		m_buttons[buttonID].m_isPressedThisFrame = true;
	}
	else
	{
		m_buttons[buttonID].m_isPressedThisFrame = false;
	}
}