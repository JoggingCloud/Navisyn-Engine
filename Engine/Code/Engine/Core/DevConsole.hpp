#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <optional>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"

#ifdef ERROR
#undef ERROR
#endif // 

class Renderer;
class Camera;
class BitmapFont;
class Timer;
struct AABB2;

class DevConsole;

struct PendingCommand
{
	std::string commandText;
	float timeRemaining = 0.f;
};

enum DevConsoleMode
{
	HIDDEN = 0,
	VISIBLE,
};

struct DevConsoleConfig
{
	bool m_enableTimeStamp = true;
	bool m_enableFrameNumber = true;
	float m_numLinesVisible = 50.5f; 
	std::string m_filePath;
};

struct DevConsoleLine
{
	DevConsoleLine(Rgba8 const& color, std::string text, float fontSize = 1.f, bool isCentered = false);
	Rgba8 const& m_color;
	std::string m_text;
	float m_fontSize = 1.f;
	bool m_isCentered = false;
};

class DevConsole
{
public:
	DevConsole() = default;
	~DevConsole();

	DevConsole(DevConsoleConfig const& config);
	void Startup();
	void ShutDown();
	void BeginFrame();
	void EndFrame();

	//void PasteFromClipboard();

	void HandleUnknownCommand(std::string const& eventName);
	void Execute(std::string const& consoleCommandText);
	void ExecuteXmlCommandScriptNode(XmlElement const& commandScriptXmlElement);
	void ExecuteXmlCommandScriptFile(std::string& commandScriptXmlFilePathName);
	void AddLine(Rgba8 const& color, std::string const& text, float fontSize = 1.f);
	void AddCenterLine(Rgba8 const& color, std::string const& text, float fontSize = 1.f);
	void Render(AABB2 const& bounds, Renderer* rendererOverride = nullptr);

	DevConsoleMode GetMode() const;
	void SetMode(DevConsoleMode mode);
	void ToggleOpen();
	bool IsOpen() const;
	float GetCurrentTimeInSeconds();
	float GetNumLines() const;

	static const Rgba8 ERROR;
	static const Rgba8 WARNING;
	static const Rgba8 INFO_MAJOR;
	static const Rgba8 INFO_MINOR;
	static const Rgba8 COMMAND_ECHO;
	static const Rgba8 COMMAND_REMOTE_ECHO;
	static const Rgba8 MARKOV_INFO;
	static const Rgba8 RESPOND_TEXT;
	static const Rgba8 INPUT_TEXT;
	static const Rgba8 INPUT_INSERTION_POINT;
	static const Rgba8 INPUT_SEMITRANSPARENT;

	// Handle key input
	static bool Event_KeyPressed(EventArgs& args);

	// Handle char input by appending valid characters to out current input line
	static bool Event_CharInput(EventArgs& args);

	// Test reading and parsing each child element
	static bool Event_Xml(EventArgs& args);

	static bool Command_XmlData(EventArgs& args);

	// Clear all lines of text
	static bool Command_Clear(EventArgs& args);
	
	// Echo the text in the argument to the dev console
	static bool Command_Echo(EventArgs& args);

	// Display all currently registered commands in the event system
	static bool Command_Help(EventArgs& args);

public:
	// For Markov System only 
	bool m_hasShownGreeting = false;
	bool m_shouldShowGreeting = false;
	bool m_isPreparingToLoadDataSet = false;
	bool m_isLoadingDataSet = false;
	std::optional<std::string> m_topicToLoadNextFrame;

protected:
	void UpdateDiscRender();
 	void Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect) const;

protected:
	DevConsoleConfig m_config;
	DevConsoleMode m_mode;
	std::vector<DevConsoleLine> m_lines;
	int m_currentFrameNumber = 0;

protected:
	// True if the dev console is currently visible and accepting input
	bool m_isOpen = false;

	// Our current line of input 
	std::string m_inputText;

	// Index of the insertion point in our current input text 
	int m_insertionPointPosition = 0;

	// True if our insertion point is currently in the visible phase of blinking 
	bool m_insertionPointVisible = true;

	// time for controlling insertion point visibility 
	Timer* m_insertionPointBlinkTimer;

	// History of all commands executed. 
	std::vector<std::string> m_commandHistory;

	// Current index in our history of commands as we are scrolling 
	int m_historyIndex = -1;

private:
	std::mutex m_linesMutex;

	int m_scrollOffset = 0;
	int m_currentLineIndex = 0;

	Clock* m_clock = nullptr;

private:
	float m_startRadius = 5.f;
	float m_endRadius = 10.f;

	bool m_isLerping = true;
	float m_lerpFactor = 0.f;
};