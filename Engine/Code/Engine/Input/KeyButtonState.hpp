#pragma once

struct KeyButtonState
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
	KeyButtonState();

	void Reset();

public:
	bool m_isPressedThisFrame = false;
	bool m_wasPressedLastFrame = false;
};

