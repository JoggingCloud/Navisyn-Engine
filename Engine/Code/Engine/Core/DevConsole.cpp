#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/AI/MarkovSystem.hpp"
#include <sstream>
#include <algorithm>

DevConsole* g_theConsole = nullptr;

Rgba8 const DevConsole::ERROR                 = Rgba8(255, 0, 0, 255);
Rgba8 const DevConsole::WARNING               = Rgba8(255, 242, 0, 255);
Rgba8 const DevConsole::INFO_MAJOR            = Rgba8(45, 170, 214, 255);
Rgba8 const DevConsole::INFO_MINOR            = Rgba8(0, 255, 0, 255);
Rgba8 const DevConsole::COMMAND_ECHO          = Rgba8(235, 72, 235, 255);
Rgba8 const DevConsole::COMMAND_REMOTE_ECHO   = Rgba8(154, 255, 251, 255);
Rgba8 const DevConsole::MARKOV_INFO           = Rgba8(255, 165, 0, 255);
Rgba8 const DevConsole::RESPOND_TEXT          = Rgba8(213, 242, 253, 255);
Rgba8 const DevConsole::INPUT_TEXT            = Rgba8(235, 72, 235, 255);
Rgba8 const DevConsole::INPUT_INSERTION_POINT = Rgba8(255, 255, 255, 255);
Rgba8 const DevConsole::INPUT_SEMITRANSPARENT = Rgba8(255, 255, 255, 25);

extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;

DevConsole::~DevConsole()
{
}

DevConsole::DevConsole(DevConsoleConfig const& config)
	:m_config(config)
{

}

void DevConsole::Startup()
{
	m_clock = new Clock();
	m_insertionPointBlinkTimer = new Timer();
	m_insertionPointBlinkTimer->Start();
	m_insertionPointBlinkTimer->m_period = 0.7f;

	g_theEventSystem->SubscribeEventCallbackFunction("Clear", Command_Clear);
	g_theEventSystem->SubscribeEventCallbackFunction("Echo", Command_Echo);
	g_theEventSystem->SubscribeEventCallbackFunction("Help", Command_Help);
	g_theEventSystem->SubscribeEventCallbackFunction("message", Command_XmlData);
	
	g_theEventSystem->SubscribeEventCallbackFunction("CharInput", Event_CharInput);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("Xml", Event_Xml);
}

void DevConsole::ShutDown()
{

}

void DevConsole::BeginFrame()
{
	m_currentFrameNumber++; 

	if (m_insertionPointBlinkTimer->DecrementPeriodIfElapsed())
	{
		m_insertionPointVisible = !m_insertionPointVisible;
	}

	if (g_theInput->WasMouseWheelJustScrolledDown())
	{
		if (m_scrollOffset > 0)
		{
			m_scrollOffset--;
		}
	}
	if (g_theInput->WasMouseWheelJustScrolledUp())
	{
		if (m_scrollOffset < (int)m_lines.size() - 1)
		{
			m_scrollOffset++;
		}
	}

	// Markov system only
	if (m_isPreparingToLoadDataSet)
	{
		// Show the disc this frame
		m_isLoadingDataSet = true;
		m_isPreparingToLoadDataSet = false;
		return; // Skip loading until next frame
	}
	UpdateDiscRender();
	if (m_isLoadingDataSet && m_topicToLoadNextFrame.has_value())
	{
		std::string topic = m_topicToLoadNextFrame.value();

		g_theMarkov->LoadDataSetByTopic(topic);
		m_isLoadingDataSet = false;
		m_topicToLoadNextFrame.reset();
		g_theConsole->AddLine(DevConsole::INFO_MAJOR, topic + " " + "was successfully loaded!");
	}
}

void DevConsole::EndFrame()
{

}

void DevConsole::HandleUnknownCommand(std::string const& eventName)
{
	std::string displayText = Stringf("%s unknown event", eventName.c_str());

	AddLine(ERROR, displayText);
}

void DevConsole::Execute(std::string const& consoleCommandText)
{
	// Split text into individual lines
	std::vector<std::string> commandLines = SplitStringOnDelimiter(consoleCommandText, '\n');

	// Loop through each command line, process each argument first and then fire 
	for (const std::string& commandLine : commandLines)
	{
		// Split quotes & command name and parts 
		std::vector<std::string> commandParts = SplitStringWithQuotes(commandLine, ' ');

		if (!commandParts.empty())
		{
			std::string commandName = commandParts[0];
			std::transform(commandName.begin(), commandName.end(), commandName.begin(), [](unsigned char c) -> unsigned char { return (unsigned char)std::tolower(c); });

			if (commandParts.size() > 1) // Only proceed if there are additional arguments
			{
				// Loop through each argument and split into key and value
				EventArgs args;

				for (size_t argumentIndex = 1; argumentIndex < commandParts.size(); argumentIndex++)
				{
					std::string argument = commandParts[argumentIndex];

					// Check if the argument is in key-value format (contains '=')
					size_t equalSignPos = argument.find('=');
					if (equalSignPos != std::string::npos)
					{
						std::string key = argument.substr(0, equalSignPos);
						std::string value = argument.substr(equalSignPos + 1);
						std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) -> unsigned char { return (unsigned char)std::tolower(c); });

						// Handle quoted values with spaces
						if (!value.empty() && value.front() == '"' && value.back() == '"')
						{
							value = value.substr(1, value.size() - 2); // Remove quotes
						}
						args.SetValue<std::string>(key, value);
					}
					else
					{
						if (commandName == "echo")
						{
							args.SetValue<std::string>("rawInput", argument);
						}
						else
						{
							AddLine(ERROR, "Arguments expect a name-value pair with a '=' sign.");
							return;
						}
					}
				}
				// Fire the event for the parsed command and arguments
				FireEvent(commandName, args);
			}
			else
			{
				// No arguments, fire the event with just the command name
				if ((commandName == "generateforward" || commandName == "generatebackward" || commandName == "generateblend"))
				{
					m_shouldShowGreeting = false;
					FireEvent(commandName);
					return;
				}
				else
				{
					if ((commandName == "yes" || commandName == "no"))
					{
						EventArgs args;
						args.SetValue("response", commandName);
						FireEvent("response", args);
						return;
					}
					FireEvent(commandName);
				}
			}
		}
	}
}

void DevConsole::ExecuteXmlCommandScriptNode(XmlElement const& commandScriptXmlElement)
{
	for (XmlElement const* commandElement = commandScriptXmlElement.FirstChildElement(); commandElement != nullptr; commandElement = commandElement->NextSiblingElement())
	{
		std::string commandName = commandElement->Name();
		EventArgs args;

		for (XmlAttribute const* attribute = commandElement->FirstAttribute(); attribute != nullptr; attribute = attribute->Next())
		{
			std::string attributeName = attribute->Name();
			std::string attributeValue = attribute->Value();
			args.SetValue<std::string>(attributeName, attributeValue); // Store as a string initially since the receiving function can convert
		}

		FireEvent(commandName, args);
	}
}

void DevConsole::ExecuteXmlCommandScriptFile(std::string& commandScriptXmlFilePathName)
{
	tinyxml2::XMLDocument document;
	if (document.LoadFile(commandScriptXmlFilePathName.c_str()) == tinyxml2::XML_SUCCESS)
	{
		// Get the root element 
		tinyxml2::XMLElement* rootElement = document.RootElement();
		GUARANTEE_OR_DIE(rootElement, "XML couldn't be loaded"); // if files doesn't exist then print this text

		// Populate with attributes
		ExecuteXmlCommandScriptNode(*rootElement);
	}
}

void DevConsole::AddLine(Rgba8 const& color, std::string const& text, float fontSize)
{
	std::lock_guard<std::mutex> lock(m_linesMutex);
	DevConsoleLine line = DevConsoleLine(color, text, fontSize);
	m_lines.emplace_back(line);
}

void DevConsole::AddCenterLine(Rgba8 const& color, std::string const& text, float fontSize /*= 1.f*/)
{
	std::lock_guard<std::mutex> lock(m_linesMutex);
	DevConsoleLine line = DevConsoleLine(color, text, fontSize, true);
	m_lines.emplace_back(line);
}

void DevConsole::Render(AABB2 const& bounds, Renderer* rendererOverride)
{
	if (m_mode == DevConsoleMode::HIDDEN)
	{
		return;
	}

	if (m_mode == DevConsoleMode::VISIBLE)
	{
		Render_OpenFull(bounds, *rendererOverride, *rendererOverride->CreateOrGetBitmapFont(m_config.m_filePath.c_str()), 0.5f);
	}
}

DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}

void DevConsole::SetMode(DevConsoleMode mode)
{
	m_mode = mode;
}

void DevConsole::ToggleOpen()
{
	if (m_mode == HIDDEN)
	{
		m_mode = DevConsoleMode::VISIBLE;
	}
	else if (m_mode == VISIBLE)
	{
		m_mode = DevConsoleMode::HIDDEN;
	}
}

bool DevConsole::IsOpen() const
{
	return m_mode == VISIBLE;
}

float DevConsole::GetCurrentTimeInSeconds()
{
	double currentTimeSeconds = ::GetCurrentTimeSeconds();
	return static_cast<float>(currentTimeSeconds);
}

float DevConsole::GetNumLines() const
{
	return m_config.m_numLinesVisible;
}

bool DevConsole::Event_KeyPressed(EventArgs& args)
{	
	unsigned char keyCode = static_cast<unsigned char>(args.GetValue<int>("KeyCode", -1));
	
	if (keyCode == KEYCODE_TILDE)
	{
		g_theConsole->ToggleOpen();
		return true;
	}

	if (g_theConsole->IsOpen())
	{
		if (keyCode == KEYCODE_END)
		{
			g_theConsole->AddLine(INFO_MAJOR, "TEST");
		}

		if (keyCode == KEYCODE_ESC)
		{
			if (g_theConsole->m_inputText.empty())
			{
				g_theConsole->SetMode(HIDDEN);
				return true;
			}
			else
			{
				g_theConsole->m_inputText.clear();
				g_theConsole->m_insertionPointPosition = 0;
				return true;
			}
		}

		if (keyCode == KEYCODE_ENTER)
		{
			if (g_theConsole->m_inputText.empty())
			{
				g_theConsole->SetMode(HIDDEN);
				return true;
			}
			
			g_theConsole->Execute(g_theConsole->m_inputText);
			g_theConsole->m_commandHistory.emplace_back(g_theConsole->m_inputText);
			g_theConsole->m_historyIndex = static_cast<int>(g_theConsole->m_commandHistory.size());
			g_theConsole->m_inputText.clear();
			g_theConsole->m_insertionPointPosition = 0;

			return true;
		}

		if (keyCode == KEYCODE_LEFTARROW)
		{
			if (g_theConsole->m_insertionPointPosition > 0)
			{
				// Move left within the current line
				g_theConsole->m_insertionPointPosition--;
			}
			else if (g_theConsole->m_currentLineIndex > 0)
			{
				// Move to the end of the previous line
				g_theConsole->m_currentLineIndex--;
				g_theConsole->m_insertionPointPosition = (int)g_theConsole->m_lines[g_theConsole->m_currentLineIndex].m_text.size();
			}
			return true;
		}

		if (keyCode == KEYCODE_RIGHTARROW)
		{
			if (g_theConsole->m_insertionPointPosition < (int)g_theConsole->m_lines[g_theConsole->m_currentLineIndex].m_text.size())
			{
				// Move right within the current line
				g_theConsole->m_insertionPointPosition++;
			}
			else if (g_theConsole->m_currentLineIndex < static_cast<int>(g_theConsole->m_lines.size()) - 1)
			{
				// Move to the start of the next line
				g_theConsole->m_currentLineIndex++;
				g_theConsole->m_insertionPointPosition = 0;
			}
			return true;
		}

		if (keyCode == KEYCODE_HOME)
		{
			g_theConsole->m_insertionPointPosition = 0;
			return true;
		}

		if (keyCode == KEYCODE_END)
		{
			g_theConsole->m_insertionPointPosition = static_cast<int>(g_theConsole->m_inputText.size());
			return true;
		}

		if (keyCode == KEYCODE_DELETE)
		{
			if (g_theConsole->m_insertionPointPosition == g_theConsole->m_inputText.size())
			{
				return false;
			}
			g_theConsole->m_inputText.erase(g_theConsole->m_insertionPointPosition, 1);
			return true;
		}

		if (keyCode == KEYCODE_BACKSPACE)
		{
			if (g_theConsole->m_insertionPointPosition == 0)
			{
				return false;
			}
			g_theConsole->m_inputText.erase(--g_theConsole->m_insertionPointPosition, 1);
			return true;
		}

		if (keyCode == KEYCODE_UPARROW)
		{
			if (!g_theConsole->m_commandHistory.empty())
			{
				if (g_theConsole->m_historyIndex > 0)
				{
					g_theConsole->m_historyIndex--;
					g_theConsole->m_inputText = g_theConsole->m_commandHistory[g_theConsole->m_historyIndex];
					g_theConsole->m_insertionPointPosition = static_cast<int>(g_theConsole->m_inputText.size());
				}
			}
		}

		if (keyCode == KEYCODE_DOWNARROW)
		{
			if (!g_theConsole->m_commandHistory.empty())
			{
				if (g_theConsole->m_historyIndex < static_cast<int>(g_theConsole->m_commandHistory.size()) - 1)
				{
					g_theConsole->m_historyIndex++;
					g_theConsole->m_inputText = g_theConsole->m_commandHistory[g_theConsole->m_historyIndex];
					g_theConsole->m_insertionPointPosition = static_cast<int>(g_theConsole->m_inputText.size());
				}
				else if (g_theConsole->m_historyIndex == static_cast<int>(g_theConsole->m_commandHistory.size()) - 1)
				{
					g_theConsole->m_historyIndex++;
					g_theConsole->m_inputText.clear();
					g_theConsole->m_insertionPointPosition = 0;
				}
			}
		}
		g_theConsole->m_insertionPointBlinkTimer->Stop();
		return true;
	}
	return false;
}

bool DevConsole::Event_CharInput(EventArgs& args)
{
	if (!g_theConsole->IsOpen())
	{
		return false;
	}

	std::string characterString = args.GetValue<std::string>("Character", "");
	if (characterString.size() == 0)
	{
		return false;
	}
	unsigned char character = characterString[0];
	if (character < 32 || character > 126 || character == '~' || character == '`')
	{
		return false;
	}

	g_theConsole->m_inputText.insert(g_theConsole->m_inputText.begin() + g_theConsole->m_insertionPointPosition, character);
	g_theConsole->m_insertionPointPosition++;
	g_theConsole->m_insertionPointBlinkTimer->Stop();

	return true;
}

bool DevConsole::Event_Xml(EventArgs& args)
{
	std::string filePath = args.GetValue<std::string>("file", "No file provided");
	if (filePath != "No file provided")
	{
		g_theConsole->ExecuteXmlCommandScriptFile(filePath);
	}
	return true;
}

bool DevConsole::Command_XmlData(EventArgs& args)
{
	std::string helloTest = args.GetValue<std::string>("message", "Unknown message");
	g_theConsole->AddLine(INFO_MAJOR, helloTest);
	return true;
}

bool DevConsole::Command_Clear(EventArgs& args)
{
	UNUSED(args);

	if (!g_theConsole->IsOpen())
	{
		return false;
	}

	g_theConsole->m_lines.clear();
	g_theConsole->m_insertionPointPosition = 0;
	return true;
}

bool DevConsole::Command_Echo(EventArgs& args)
{
	std::string keyName = "message";
	ToLower(keyName);

	if (args.HasArgument(keyName))
	{
		std::string message = args.GetValue<std::string>(keyName, "");
		std::string echoMessage = "Echo Message=" + message;

		g_theConsole->AddLine(DevConsole::COMMAND_ECHO, echoMessage);
		g_theConsole->AddLine(DevConsole::COMMAND_REMOTE_ECHO, message);
	}
	else
	{
		// Handle the case where the "message" argument is missing
		g_theConsole->AddLine(DevConsole::ERROR, "Error: Arguments are missing or incorrect");
	}
	return true;
}

bool DevConsole::Command_Help(EventArgs& args)
{
	UNUSED(args);
	g_theConsole->AddLine(INFO_MAJOR, "_____________________", 1.5f);
	g_theConsole->AddLine(INFO_MINOR, "List of Help Commands", 1.5f);
	g_theConsole->AddLine(INFO_MAJOR, "---------------------", 1.5f);
	Strings registeredCommands = g_theEventSystem->GetRegisteredCommandNames();

	// Display each registered command
	for (size_t commandRegistered = 0; commandRegistered < registeredCommands.size(); commandRegistered++)
	{
		g_theConsole->AddLine(INFO_MINOR, "- " + registeredCommands[commandRegistered], 1.25f);
	}
	g_theConsole->AddLine(INFO_MAJOR, "---------------------", 1.5f);
	return true;
}

void DevConsole::UpdateDiscRender()
{
	if (!m_isLoadingDataSet) return;

	float deltaSeconds = m_clock->GetDeltaSeconds();

	// Loop lerp factor every second
	m_lerpFactor += deltaSeconds * 2.f;

	if (m_lerpFactor >= 1.f)
	{
		m_lerpFactor -= 1.f;
	}
}

void DevConsole::Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect) const
{
	renderer.SetRasterizerState(RasterizerMode::SOLID_CULL_NONE);
	renderer.SetBlendMode(BlendMode::ALPHA);

	float shadowOffset = 1.f;
	float lineHeight = bounds.GetDimensions().y / m_config.m_numLinesVisible; 
	
	AABB2 currentLineBox(bounds.m_mins.x, bounds.m_mins.y + lineHeight, bounds.m_maxs.x, bounds.m_mins.y + lineHeight * 2.f);
	AABB2 shadowLineBox(bounds.m_mins.x + shadowOffset, bounds.m_mins.y + lineHeight, bounds.m_maxs.x, bounds.m_mins.y + lineHeight * 2.f);
	AABB2 centerLineBox(bounds.m_mins.x, bounds.GetCenter().y - lineHeight * 0.5f, bounds.m_maxs.x, bounds.GetCenter().y + lineHeight * 0.5f);

	static std::vector<Vertex_PCU> textureVerts;
	textureVerts.reserve(200000); // Adjust based on number of potential characters on screen
	textureVerts.clear();
	
	// Render actual text
	int lastLineToRender = static_cast<int>(m_lines.size()) - 1 - m_scrollOffset;
	float firstLineToRenderF = lastLineToRender - (m_config.m_numLinesVisible - 1);
	int firstLineToRender = static_cast<int>(std::floor(firstLineToRenderF));

	if (firstLineToRender < 0) 
	{
		firstLineToRender = 0;
	}

	for (int lineIndex = lastLineToRender; lineIndex >= firstLineToRender; lineIndex--)
	{
		std::string displayText = m_lines[lineIndex].m_text.c_str();
		float lineFontSize = m_lines[lineIndex].m_fontSize;

		if (m_lines[lineIndex].m_isCentered && !m_shouldShowGreeting) continue;

		if (m_lines[lineIndex].m_isCentered)
		{
			font.AddVertsForTextInBox2D(textureVerts, centerLineBox, lineHeight * m_lines[lineIndex].m_fontSize, m_lines[lineIndex].m_text, m_lines[lineIndex].m_color, fontAspect, Vec2(0.5f, 0.5f), TextDrawMode::SHRINK_TO_FIT);
		}
		else
		{
			int numWrappedLines = font.GetWrappedTextCount(displayText, currentLineBox.GetDimensions().x, lineHeight * lineFontSize, fontAspect);
			if (numWrappedLines == 0) numWrappedLines = 1;

			font.AddVertsForTextInBox2D(textureVerts, shadowLineBox, lineHeight * lineFontSize, displayText, m_lines[lineIndex].m_color, fontAspect, Vec2(0.f, 0.5f), TextDrawMode::WRAP);
			font.AddVertsForTextInBox2D(textureVerts, currentLineBox, lineHeight * lineFontSize, displayText, m_lines[lineIndex].m_color, fontAspect, Vec2(0.f, 0.5f), TextDrawMode::WRAP);

			float wrappedTextHeight = lineHeight * lineFontSize * numWrappedLines;
			shadowLineBox.Translate(Vec2(0.f, wrappedTextHeight));
			currentLineBox.Translate(Vec2(0.f, wrappedTextHeight));
		}
	}
	
	AABB2 inputTextBox(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.x, bounds.m_mins.y + lineHeight);
	AABB2 shadowInputTextBox(bounds.m_mins.x + shadowOffset, bounds.m_mins.y, bounds.m_maxs.x, bounds.m_mins.y + lineHeight);

	std::string displayText = Stringf("%s", m_inputText.c_str());
	font.AddVertsForTextInBox2D(textureVerts, shadowInputTextBox, lineHeight, displayText, INPUT_SEMITRANSPARENT, fontAspect, Vec2(0.f, 0.5f), TextDrawMode::SHRINK_TO_FIT);
	font.AddVertsForTextInBox2D(textureVerts, inputTextBox, lineHeight, displayText, INPUT_TEXT, fontAspect, Vec2(0.f, 0.5f), TextDrawMode::SHRINK_TO_FIT);

	if (m_isLoadingDataSet)
	{
		float pulse = 0.5f * (1.f + sinf(m_lerpFactor * 2.f * pi)); // 0 to 1 smooth oscillation
		float currentRadius = Interpolate(m_startRadius, m_endRadius, pulse);
		static std::vector<Vertex_PCU> discVerts;
		discVerts.reserve(1000); // Adjust based on number of potential verts
		discVerts.clear();
		AddVertsForDisc2D(discVerts, inputTextBox.m_mins + Vec2(10.f, 10.f), currentRadius, 64, Rgba8::WHITE);
		renderer.BindTexture(0, nullptr);
		renderer.BindShader(nullptr);
		renderer.DrawVertexArray(static_cast<int>(discVerts.size()), discVerts.data());
	}

	float characterWidth = lineHeight * fontAspect;
	float halfCharacterWidth = characterWidth * 0.5f;
	float characterPos = characterWidth * m_insertionPointPosition;

	AABB2 insertionPoint(
		inputTextBox.m_mins.x - halfCharacterWidth + characterPos,
		inputTextBox.m_mins.y + lineHeight * m_currentLineIndex,  // Adjust for current line index
		inputTextBox.m_mins.x - halfCharacterWidth + characterPos + lineHeight,
		inputTextBox.m_mins.y + lineHeight * (m_currentLineIndex + 1)
	);

	char insertionChar = '|';
	std::string displayInsertion = Stringf("%c", insertionChar);
	if (m_insertionPointVisible && !m_isLoadingDataSet)
	{
		font.AddVertsForTextInBox2D(textureVerts, insertionPoint, lineHeight, displayInsertion, INPUT_INSERTION_POINT, fontAspect, Vec2(0.f, 0.5f), TextDrawMode::SHRINK_TO_FIT);
	}

 	renderer.SetModelConstants();
	renderer.BindTexture(0, &font.GetTexture());
	renderer.BindShader(nullptr);
	renderer.DrawVertexArray(static_cast<int>(textureVerts.size()), textureVerts.data());
}

DevConsoleLine::DevConsoleLine(Rgba8 const& color, std::string text, float fontSize /*= 1.f*/, bool isCentered /*= false*/)
	:m_color(color), m_text(text), m_fontSize(fontSize), m_isCentered(isCentered)
{
}
