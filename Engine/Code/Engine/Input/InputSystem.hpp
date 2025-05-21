#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EventSystem.hpp"
#include <cstdlib>
#include <bitset>

class Camera;

extern unsigned char const MOUSE_WHEEL;

extern unsigned char const KEYCODE_ONE;
extern unsigned char const KEYCODE_TWO;
extern unsigned char const KEYCODE_THREE;
extern unsigned char const KEYCODE_FOUR;
extern unsigned char const KEYCODE_FIVE;
extern unsigned char const KEYCODE_SIX;
extern unsigned char const KEYCODE_SEVEN;
extern unsigned char const KEYCODE_EIGHT;
extern unsigned char const KEYCODE_NINE;

extern unsigned char const KEYCODE_F1;
extern unsigned char const KEYCODE_F2;
extern unsigned char const KEYCODE_F3;
extern unsigned char const KEYCODE_F4;
extern unsigned char const KEYCODE_F5;
extern unsigned char const KEYCODE_F6;
extern unsigned char const KEYCODE_F7;
extern unsigned char const KEYCODE_F8;
extern unsigned char const KEYCODE_F9;
extern unsigned char const KEYCODE_F10;
extern unsigned char const KEYCODE_F11;

extern unsigned char const KEYCODE_ESC;
extern unsigned char const KEYCODE_SPACE;
extern unsigned char const KEYCODE_SHIFT;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_INSERT;
extern unsigned char const KEYCODE_DELETE;
extern unsigned char const KEYCODE_BACKSPACE;
extern unsigned char const KEYCODE_HOME;
extern unsigned char const KEYCODE_END;

extern unsigned char const KEYCODE_UPARROW;
extern unsigned char const KEYCODE_DOWNARROW;
extern unsigned char const KEYCODE_LEFTARROW;
extern unsigned char const KEYCODE_RIGHTARROW;

extern unsigned char const KEYCODE_LEFTMOUSE;
extern unsigned char const KEYCODE_RIGHTMOUSE;

extern unsigned char const KEYCODE_TILDE;

extern unsigned char const KEYCODE_LEFTBRACKET;
extern unsigned char const KEYCODE_RIGHTBRACKET;

extern unsigned char const KEYCODE_GREATERTHAN;
extern unsigned char const KEYCODE_LESSTHAN;

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;
constexpr int NUM_XBOX_BUTTONS = 14;

struct InputConfig
{

};

struct CursorState
{
	IntVec2 m_cursorClientDelta;
	IntVec2 m_cursorClientPosition;

	bool m_hiddenMode = false;
	bool m_relativeMode = false;
};

class InputSystem
{
public:
	InputSystem(InputConfig const& config);
	~InputSystem();
	
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	// Hidden mode controls whether the cursor is visible or not. Relative
	// mode will calculate a cursor client delta and then reset the cursor
	// to the client region center each frame. Both of these together can be
	// used to implement an FPS-style mouse look camera.
	void SetCursorMode(bool hiddenMode, bool relativeMode);
	// Returns the current frame cursor delta in pixels, relative to the client
	// region. Only valid in relative mode, will be zero otherwise.
	Vec2 GetCursorClientDelta() const;
	// Returns the cursor position, normalized to the range [0, 1], relative
	// to the client region, with the y-axis inverted to map from Windows
	// conventions to game screen camera conventions
	Vec2 GetCursorNormalizedPosition() const;

	void ResetCursorMode();

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_KeyReleased(EventArgs& args);
	static bool Event_MouseScroll(EventArgs& args);

	Vec3 GetMouseForward(float fov, const Camera& camera);

	bool WasMouseWheelJustScrolledUp();
	bool WasMouseWheelJustScrolledDown();
	bool IsKeyDown(unsigned char keyCode);
	bool IsKeyUp(unsigned char keyCode);
 	bool WasKeyJustPressed(unsigned char keyCode);

	bool HandleKeyPressed(unsigned char keyCode);
	bool HandleKeyReleased(unsigned char keyCode);
	XboxController const& GetController(int controllerID);

protected:
	KeyButtonState m_keyStates[NUM_KEYCODES];
	XboxController m_controllers[NUM_XBOX_CONTROLLERS];

protected:
	InputConfig m_config;
	CursorState m_cursorState;

	int m_scrollState = 0;
};