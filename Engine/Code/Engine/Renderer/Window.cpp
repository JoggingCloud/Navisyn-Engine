#include "Engine/Renderer/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/ImGui/imgui_impl_win32.h"
#include "ThirdParty/ImGui/imgui_impl_dx11.h"
#include <algorithm>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX  
#include <windows.h> 
#include <commdlg.h>  // For file dialogs
#include <windows.h>
#undef max

Window* Window::s_theWindow = nullptr;


LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam))
		return true;

	Window* window = Window::GetMainWindowInstance();
	GUARANTEE_OR_DIE(window != nullptr, "Window was null!");

	InputSystem* input = window->GetConfig().m_inputSystem;
	GUARANTEE_OR_DIE(input != nullptr, "Window's InputSystem was null!");


	switch (wmMessageCode)
	{
	case WM_CHAR:
	{
		EventArgs args;
		args.SetValue<std::string>("Character", Stringf("%c", (unsigned char)wParam));
		FireEvent("CharInput", args);
		return 0;
	}

	// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:
	{
		FireEvent("Quit");
		return 0; // "Consumes" this message (tells Windows "okay, we handled it")
	}

	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED)
		{
			FireEvent("WindowMinimized");
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			FireEvent("WindowMaximized");
		}
		else if (wParam == SIZE_RESTORED)
		{
			FireEvent("WindowRestored");

			if (window->GetConfig().m_fullScreen)
			{
				RECT restoreRect = { 0, 0, 1400, 700 };
				DWORD style = WS_OVERLAPPEDWINDOW;
				AdjustWindowRectEx(&restoreRect, style, FALSE, WS_EX_APPWINDOW);

				int width = restoreRect.right - restoreRect.left;
				int height = restoreRect.bottom - restoreRect.top;

				// Find the monitor the window is on (or nearest to)
				HWND hwnd = reinterpret_cast<HWND>(window->GetHwnd());
				HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO monitorInfo = {};
				monitorInfo.cbSize = sizeof(monitorInfo);
				GetMonitorInfo(monitor, &monitorInfo);

				// Get the usable work area of the monitor (excluding taskbar, etc.)
				RECT workArea = monitorInfo.rcWork;

				// Clamp to work area so window fits on screen
				int maxX = std::max(0, static_cast<int>(workArea.right - workArea.left - width));
				int maxY = std::max(0, static_cast<int>(workArea.bottom - workArea.top - height));

				int posX = workArea.left + (maxX / 2); // center horizontally
				int posY = workArea.top + (maxY / 2);  // center vertically

				SetWindowPos(hwnd,
					NULL,
					posX,
					posY,
					width,
					height,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		return 0;
	}

	// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
	case WM_KEYDOWN: // When pressing and holding a key down 
	{
		EventArgs args;
		args.SetValue<int>("KeyCode", static_cast<int>((unsigned char)wParam));
		FireEvent("KeyPressed", args);
		return 0;
	}

	// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
	case WM_KEYUP: // When releasing from the key down method 
	{
		EventArgs args;
		args.SetValue<int>("KeyCode", static_cast<int>((unsigned char)wParam));
		FireEvent("KeyReleased", args);
		return 0;
	}

	// Treat this special mouse-button windows message as if it were an ordinary key down
	case WM_LBUTTONDOWN:
	{
		EventArgs args;
		args.SetValue<int>("KeyCode", static_cast<int>(KEYCODE_LEFTMOUSE));
		FireEvent("KeyPressed", args);
		return 0;
	}

	case WM_LBUTTONUP:
	{
		EventArgs args;
		args.SetValue<int>("KeyCode", static_cast<int>(KEYCODE_LEFTMOUSE));
		FireEvent("KeyReleased", args);
		return 0;
	}

	case WM_RBUTTONDOWN:
	{
		EventArgs args;
		args.SetValue<int>("KeyCode", static_cast<int>(KEYCODE_RIGHTMOUSE));
		FireEvent("KeyPressed", args);
		return 0;
	}

	case WM_RBUTTONUP:
	{
		EventArgs args;
		args.SetValue<int>("KeyCode", static_cast<int>(KEYCODE_RIGHTMOUSE));
		FireEvent("KeyReleased", args);
		return 0;
	}

	case WM_MOUSEWHEEL:
	{
		if (IsMousePresent())
		{
			EventArgs args;
			args.SetValue<int>("Mouse", GET_WHEEL_DELTA_WPARAM(wParam));
			FireEvent("MouseScroll", args);
			return 0;
		}
	}

	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

bool IsMousePresent()
{
	return GetSystemMetrics(SM_MOUSEPRESENT) != 0;
}

Window::Window(WindowConfig const& config)
	:m_config(config)
{
	s_theWindow = this;
}

void Window::Startup()
{
	CreateOSWindow();
}

void Window::BeginFrame()
{
	RunMessagePump();
}

void Window::EndFrame()
{

}

void Window::Shutdown()
{

}

void Window::SetClientDimensions(IntVec2 const& dimensions)
{
	m_clientDimensions = dimensions;
}

void* Window::GetHwnd() const
{
	return m_hwnd;
}

IntVec2 Window::GetClientDimensions() const
{
	return m_clientDimensions;
}

WindowConfig const& Window::GetConfig() const
{
	return m_config;
}

float Window::GetAspect() const
{
	float ratio = static_cast<float>(m_clientDimensions.x) / static_cast<float>(m_clientDimensions.y);
	return ratio;
}

void* Window::GetDisplayContext() const
{
	return m_displayContext;
}

bool Window::GetFileName(std::string& outPath)
{
	char filename[MAX_PATH];
	filename[0] = '\0';

	OPENFILENAMEA data = { };
	data.lStructSize = sizeof(data);
	data.lpstrFile = filename;
	data.nMaxFile = sizeof(filename);
	data.lpstrFilter = "All\0*.*\0";
	data.nFilterIndex = 1;
	data.lpstrInitialDir = NULL; // Open the dialog in the system default directory
	data.hwndOwner = (HWND)GetHwnd();
	data.Flags = OFN_PATHMUSTEXIST | OFN_PATHMUSTEXIST;

	char currentDirectory[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, currentDirectory);

	if (GetOpenFileNameA(&data))
	{
		outPath = filename;
		SetCurrentDirectoryA(currentDirectory);
		return true;
	}
	else
	{
		// If dialog fails, then retrieve the error 
		DWORD error = ::GetLastError();
		if (error != 0) // 0 means no error
		{
			// Format the error message 
			LPSTR messageBuffer = nullptr;
			DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
			FormatMessageA(flags, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, NULL);
			ERROR_RECOVERABLE(messageBuffer);
			LocalFree(messageBuffer);
		}
	}
	return false;
}

Window* Window::GetMainWindowInstance()
{
	return s_theWindow;
}

void Window::CreateOSWindow()
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	// Set window style
	DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_OVERLAPPED;
	RECT clientRect;
	if (m_config.m_size.x != -1 && m_config.m_size.y != -1) 
	{
		clientRect.left = 0;
		clientRect.top = 0;
		clientRect.right = m_config.m_size.x;
		clientRect.bottom = m_config.m_size.y;
	}
	else 
	{
		clientRect.left = 0;
		clientRect.top = 0;
		clientRect.right = 1400; // or any default width
		clientRect.bottom = 700; // or any default height
	}

	if (m_config.m_fullScreen)
	{
		// Use WS_OVERLAPPEDWINDOW to keep the title bar, minimize/maximize/close buttons
		windowStyleFlags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

		// Get screen dimensions
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// Get system border and title bar sizes
		int borderX = GetSystemMetrics(SM_CXFRAME);  // Width of window border
		int borderY = GetSystemMetrics(SM_CYFRAME);  // Height of window border
		int titleBarHeight = GetSystemMetrics(SM_CYCAPTION); // Title bar height

		// Adjust window size to allow visibility of the title bar and borders
		clientRect.left = borderX;
		clientRect.top = titleBarHeight + borderY;
		clientRect.right = screenWidth - borderX;
		clientRect.bottom = screenHeight - borderY;

		// Shrink the window slightly so the OS does not auto-hide the title bar
		clientRect.right -= borderX * 2;
		clientRect.bottom -= borderY * 2;
	}
	else
	{
		if (m_config.m_size.x == -1 && m_config.m_size.y != -1) 
		{
			// Height is specified, calculate width based on aspect ratio
			int adjustedWidth = static_cast<int>(m_config.m_size.y * m_config.m_aspectRatio);
			clientRect.right = clientRect.left + adjustedWidth;
		}
		else if (m_config.m_size.y == -1 && m_config.m_size.x != -1) 
		{
			// Width is specified, calculate height based on aspect ratio
			int adjustedHeight = static_cast<int>(m_config.m_size.x / m_config.m_aspectRatio);
			clientRect.bottom = clientRect.top + adjustedHeight;
		}

		// Set window position if specified
		if (m_config.m_pos.x >= 0 && m_config.m_pos.y >= 0)
		{
			clientRect.left += m_config.m_pos.x;
			clientRect.top += m_config.m_pos.y;
			clientRect.right += m_config.m_pos.x;
			clientRect.bottom += m_config.m_pos.y;
		}
		else
		{
			// Default center position if not specified
			int screenWidth = GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = GetSystemMetrics(SM_CYSCREEN);
			clientRect.left = (screenWidth - (clientRect.right - clientRect.left)) / 2;
			clientRect.top = (screenHeight - (clientRect.bottom - clientRect.top)) / 2;
			clientRect.right = clientRect.left + (clientRect.right - clientRect.left);
			clientRect.bottom = clientRect.top + (clientRect.bottom - clientRect.top);
		}
	}

	// Adjust window rect
	AdjustWindowRectEx(&clientRect, windowStyleFlags, FALSE, WS_EX_APPWINDOW);

	// Create the window
	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	
	HWND hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,                             // Extended Style
        windowClassDescription.lpszClassName,        // Class Name
		windowTitle,								 // Window Title
        windowStyleFlags,                            // Style
        clientRect.left, clientRect.top,             // Window position
        clientRect.right - clientRect.left,          // Window width
        clientRect.bottom - clientRect.top,          // Window height
        NULL,                                        // No parent
        NULL,                                        // No menu
        GetModuleHandle(NULL),                       // App instance
        NULL);                                       // No creation parameters

	// Display and focus the window
	if (m_config.m_fullScreen)
	{
		ShowWindow(hwnd, SW_MAXIMIZE);
	}
	else
	{
		ShowWindow(hwnd, SW_SHOW);
	}
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	UpdateWindow(hwnd);

	// Store the handle and context
	m_displayContext = GetDC(hwnd);
	m_hwnd = static_cast<void*>(hwnd);

	RECT finalClientRect;
	GetClientRect(hwnd, &finalClientRect);
	m_clientDimensions = IntVec2(finalClientRect.right - finalClientRect.left, finalClientRect.bottom - finalClientRect.top);

	if (hwnd == NULL) 
	{
		DWORD error = GetLastError();
		// Log or handle the error code to determine the issue.
		char errorMsg[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, 0, errorMsg, sizeof(errorMsg), NULL);
		ERROR_RECOVERABLE(Stringf("Failed to create window: %s", errorMsg));
	}

	// Set cursor
	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);
}

void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function

	}
}