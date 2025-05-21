#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Window.hpp"

const unsigned char MOUSE_WHEEL = WH_MOUSE;

const unsigned char KEYCODE_ONE = VK_NUMPAD1;
const unsigned char KEYCODE_TWO = VK_NUMPAD2;
const unsigned char KEYCODE_THREE = VK_NUMPAD3;
const unsigned char KEYCODE_FOUR = VK_NUMPAD4;
const unsigned char KEYCODE_FIVE = VK_NUMPAD5;
const unsigned char KEYCODE_SIX = VK_NUMPAD6;
const unsigned char KEYCODE_SEVEN = VK_NUMPAD7;
const unsigned char KEYCODE_EIGHT = VK_NUMPAD8;
const unsigned char KEYCODE_NINE = VK_NUMPAD9;

const unsigned char KEYCODE_F1 = VK_F1;
const unsigned char KEYCODE_F2 = VK_F2;
const unsigned char KEYCODE_F3 = VK_F3;
const unsigned char KEYCODE_F4 = VK_F4;
const unsigned char KEYCODE_F5 = VK_F5;
const unsigned char KEYCODE_F6 = VK_F6;
const unsigned char KEYCODE_F7 = VK_F7;
const unsigned char KEYCODE_F8 = VK_F8;
const unsigned char KEYCODE_F9 = VK_F9;
const unsigned char KEYCODE_F10 = VK_F10;
const unsigned char KEYCODE_F11 = VK_F11;
const unsigned char KEYCODE_ESC = VK_ESCAPE;
const unsigned char KEYCODE_SPACE = VK_SPACE;
const unsigned char KEYCODE_SHIFT = VK_SHIFT;
const unsigned char KEYCODE_ENTER = VK_RETURN;
const unsigned char KEYCODE_INSERT = VK_INSERT;
const unsigned char KEYCODE_DELETE = VK_DELETE;
const unsigned char KEYCODE_BACKSPACE = VK_BACK;
const unsigned char KEYCODE_HOME = VK_HOME;
const unsigned char KEYCODE_END = VK_END;

const unsigned char KEYCODE_UPARROW = VK_UP;
const unsigned char KEYCODE_DOWNARROW = VK_DOWN;
const unsigned char KEYCODE_LEFTARROW = VK_LEFT;
const unsigned char KEYCODE_RIGHTARROW = VK_RIGHT;

const unsigned char KEYCODE_LEFTMOUSE = VK_LBUTTON;
const unsigned char KEYCODE_RIGHTMOUSE = VK_RBUTTON;
const unsigned char KEYCODE_TILDE = 0xC0;
const unsigned char KEYCODE_LEFTBRACKET = 0xDB;
const unsigned char KEYCODE_RIGHTBRACKET = 0xDD;

const unsigned char KEYCODE_GREATERTHAN = VK_OEM_PERIOD;
const unsigned char KEYCODE_LESSTHAN = VK_OEM_COMMA;

extern InputSystem* g_theInput;
extern Window* g_theWindow;

InputSystem::InputSystem(InputConfig const& config)
	:m_config(config)
{

}

InputSystem::~InputSystem()
{

}

void InputSystem::Startup()
{
	for (int controllerNum = 0; controllerNum < NUM_XBOX_CONTROLLERS; controllerNum++)
	{
		m_controllers[controllerNum] = XboxController();
		m_controllers[controllerNum].m_id = 0;
	}

	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", InputSystem::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyReleased", InputSystem::Event_KeyReleased);
	g_theEventSystem->SubscribeEventCallbackFunction("MouseScroll", InputSystem::Event_MouseScroll);
}

void InputSystem::Shutdown()
{

}

void InputSystem::BeginFrame()
{
	// Update each of the system's 4 XboxControllers
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].Update();
	}
	
	if (g_theWindow->GetHwnd() == GetActiveWindow())
	{
		// Get last cursor pos
		IntVec2 lastCursorPos = m_cursorState.m_cursorClientPosition;

		HWND windowHandle = HWND(Window::GetMainWindowInstance()->GetHwnd());
		// Get current mouse pos and set it equal to last mouse pos
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(windowHandle, &pt);
		// Set previous mouse pos to current pos
		m_cursorState.m_cursorClientPosition.x = pt.x;
		m_cursorState.m_cursorClientPosition.y = pt.y;

		if (m_cursorState.m_relativeMode)
		{
			// Calculate the delta by doing current minus previous
			m_cursorState.m_cursorClientDelta.x = m_cursorState.m_cursorClientPosition.x - lastCursorPos.x;
			m_cursorState.m_cursorClientDelta.y = m_cursorState.m_cursorClientPosition.y - lastCursorPos.y;

			POINT centerPoint;
			centerPoint.x = Window::GetMainWindowInstance()->GetClientDimensions().x / 2;
			centerPoint.y = Window::GetMainWindowInstance()->GetClientDimensions().y / 2;
			ClientToScreen(windowHandle, &centerPoint);
			SetCursorPos(centerPoint.x, centerPoint.y);
			GetCursorPos(&pt);
			ScreenToClient(windowHandle, &pt);
			m_cursorState.m_cursorClientPosition.x = pt.x;
			m_cursorState.m_cursorClientPosition.y = pt.y;
		}
		else
		{
			// Set the client delta to zero
			m_cursorState.m_cursorClientDelta.x = 0;
			m_cursorState.m_cursorClientDelta.y = 0;
		}

	}
	else 
	{
		m_cursorState.m_relativeMode = false;
		m_cursorState.m_cursorClientDelta.x = 0;
		m_cursorState.m_cursorClientDelta.y = 0;
	}
	if (m_cursorState.m_hiddenMode)
	{
		while (ShowCursor(false) >= 0);
	}
	else
	{
		while (ShowCursor(true) < 0);
	}
}

void InputSystem::EndFrame()
{
	for (int i = 0; i < NUM_KEYCODES; i++)
	{
		m_keyStates[i].m_wasPressedLastFrame = m_keyStates[i].m_isPressedThisFrame;
	}

	m_scrollState = 0;
}

void InputSystem::SetCursorMode(bool hiddenMode, bool relativeMode)
{
	// Set the cursor mode based on the provided parameters
	m_cursorState.m_hiddenMode = hiddenMode;
	m_cursorState.m_relativeMode = relativeMode;
}

Vec2 InputSystem::GetCursorClientDelta() const
{
	return Vec2(static_cast<float>(m_cursorState.m_cursorClientDelta.x), static_cast<float>(m_cursorState.m_cursorClientDelta.y));
}

Vec2 InputSystem::GetCursorNormalizedPosition() const
{
	HWND windowHandle = ::GetActiveWindow();
	POINT cursorCoords;
	RECT clientRect;
	::GetCursorPos(&cursorCoords);
	::ScreenToClient(windowHandle, &cursorCoords); // in screen coordinates, (0,0) top-left
	::GetClientRect(windowHandle, &clientRect); // size of window interior (0,0 to width, height)
	float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);
	float cursorX = float(cursorCoords.x) / clientWidth; // normalized x position
	float cursorY = float(cursorCoords.y) / clientHeight; // normalized y position
	return Vec2(cursorX, 1.f - cursorY); // We want (0,0) in the bottom-left
}

void InputSystem::ResetCursorMode()
{
	if (m_cursorState.m_hiddenMode)
	{
		while (ShowCursor(false) >= 0);
	}
	else
	{
		while (ShowCursor(true) < 0);
	}
}

bool InputSystem::Event_KeyPressed(EventArgs& args)
{
	if (g_theConsole && g_theConsole->IsOpen())
	{
		return false;
	}

	unsigned char keyCode = static_cast<unsigned char>(args.GetValue<int>("KeyCode", -1));
	if (keyCode == KEYCODE_ESC && g_theConsole)
	{
		g_theConsole->SetMode(HIDDEN);
	}

	g_theInput->HandleKeyPressed(keyCode);
	return true;
}

bool InputSystem::Event_KeyReleased(EventArgs& args)
{
	unsigned char keyCode = static_cast<unsigned char>(args.GetValue<int>("KeyCode", -1));
	g_theInput->HandleKeyReleased(keyCode);
	return true;
}

bool InputSystem::Event_MouseScroll(EventArgs& args)
{
	g_theInput->m_scrollState = args.GetValue<int>("Mouse", 0);
	// reset the scroll aback to 0 after each scroll. 
	return true;
}

Vec3 InputSystem::GetMouseForward(float fov, const Camera& camera)
{
	Vec2 cursorPos = GetCursorNormalizedPosition();

	float screenWidth = (float)g_theWindow->GetClientDimensions().x;
	float screenHeight = (float)g_theWindow->GetClientDimensions().y;

	float planeDistance = screenHeight / (2.f * TanDegrees(fov * 0.5f));

	Vec3 forward = camera.GetModelMatrix().GetIBasis3D();
	Vec3 left = camera.GetModelMatrix().GetJBasis3D();
	Vec3 up = camera.GetModelMatrix().GetKBasis3D();

	Vec3 center = camera.m_position + forward * planeDistance;
	Vec3 rightOffset = -1.f * left * (cursorPos.x - 0.5f) * screenWidth;
	Vec3 upOffset = up * (cursorPos.y - 0.5f) * screenHeight;

	Vec3 target = center + rightOffset + upOffset;

	return (target - camera.m_position).GetNormalized();
}

bool InputSystem::WasMouseWheelJustScrolledUp()
{
	if (m_scrollState <= 0)
	{
		return false;
	}
	return true;
}

bool InputSystem::WasMouseWheelJustScrolledDown()
{
	if (m_scrollState >= 0)
	{
		return false;
	}
	return true;
}

bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return 	m_keyStates[keyCode].m_isPressedThisFrame;
}

bool InputSystem::IsKeyUp(unsigned char keyCode)
{
	return !m_keyStates[keyCode].m_isPressedThisFrame && m_keyStates[keyCode].m_wasPressedLastFrame;
}

bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	if (!m_keyStates[keyCode].m_wasPressedLastFrame && m_keyStates[keyCode].m_isPressedThisFrame)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressedThisFrame = true;
	return false;
}

bool InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressedThisFrame = false;
	return false;
}

XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controllers[controllerID];
}