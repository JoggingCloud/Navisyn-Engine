#pragma once
#include <string>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

class InputSystem;

struct WindowConfig
{
	InputSystem* m_inputSystem = nullptr;
	float m_aspectRatio = 2.f;
	std::string m_windowTitle = "UNNAMED APPLICATION"; // Screen title
	bool m_fullScreen = false; // Full screen flag
	IntVec2 m_size = IntVec2(-1, -1); // Screen size 
	IntVec2 m_pos = IntVec2(-1, -1); // Screen position
};

bool IsMousePresent();

class Window
{
public:
	Window(WindowConfig const& config);

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void SetClientDimensions(IntVec2 const& dimensions);

	void* GetHwnd() const;
	IntVec2 GetClientDimensions() const;

	WindowConfig const& GetConfig() const;
	float GetAspect() const;
	void* GetDisplayContext() const;

	bool GetFileName(std::string& outPath);

	static Window* GetMainWindowInstance();

protected:
	void CreateOSWindow();
	void RunMessagePump();

protected:
	static Window* s_theWindow; // Just a declaration (advertisement for a .cpp- defined global!

protected:
	WindowConfig m_config;
	void* m_displayContext = nullptr; // HDC in Windows (handle to the display device context)
	void* m_hwnd;
	IntVec2 m_clientDimensions;
};