#include "Engine/AI/MarkovSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <numeric>
#include <cstdlib>

MarkovSystem* g_theMarkov = nullptr;

MarkovSystem::~MarkovSystem()
{
}

MarkovSystem::MarkovSystem(MarkovConfig const& config)
	: m_config(config)
{
}

void MarkovSystem::Startup()
{
	LoadUsername();
	RegisterMarkovCommand("StartupMarkov", Command_StartupMarkov);
}

void MarkovSystem::ShutDown()
{
}

void MarkovSystem::BeginFrame()
{
}

void MarkovSystem::EndFrame()
{
}

void MarkovSystem::LoadDataSet(const std::string& dataSetFilePath)
{
	FileUtils file;
	int result = file.FileReadLinesToVector(m_dataSet, dataSetFilePath);
	GUARANTEE_OR_DIE(result > 0, "Failed to load TXT dataset!");

	ParseLoadedDataSet();
}

void MarkovSystem::ParseLoadedDataSet()
{
	if (m_config.m_enableForward)
	{
		ForwardParsing();
	}

	if (m_config.m_enableBackward)
	{
		BackwardParsing();
	}
}

void MarkovSystem::ForwardParsing()
{
	// Parse words from each line of m_dataSet
	for (const std::string& line : m_dataSet)
	{
		std::istringstream stream(line); // Extracts individual words from a line
		std::deque<std::string> stateQueue;
		std::string word;

		// Add '[START]' token
		std::string startToken = "[START]";
		stateQueue.emplace_back(startToken);
		GetHashedWordIndex(startToken);

		// Fill the state queue with initial words from the dataset
		while (stateQueue.size() < m_order - 1 && stream >> word)
		{
			GetHashedWordIndex(word);
			stateQueue.emplace_back(word);
		}

		// Tokenize lines into words
		while (stream >> word)
		{
			size_t hashedWordIndex = GetHashedWordIndex(word);

			if (stateQueue.size() == m_order)
			{
				CreateTransitionBetweenWords(stateQueue, hashedWordIndex, m_forwardTransitionMatrix);
				stateQueue.pop_front(); // Remove the first word
			}
			stateQueue.emplace_back(word); // Add the next word
		}

		// Add '[END]' token after the last word
		std::string endToken = "[END]";
		size_t endTokenIndex = GetHashedWordIndex(endToken);
		if (stateQueue.size() == m_order)
		{
			CreateTransitionBetweenWords(stateQueue, endTokenIndex, m_forwardTransitionMatrix);
			stateQueue.pop_front();
		}

		// Pad if queue is too small
		while (stateQueue.size() < m_order)
		{
			stateQueue.emplace_back("[END]");
		}
		CreateTransitionBetweenWords(stateQueue, endTokenIndex, m_forwardTransitionMatrix);
	}

	NormalizeTransitionMatrix(m_forwardTransitionMatrix); // Ensure probabilities in matrix are valid (Meaning nothing greater than 1 or less than 0, and probabilities sum up to 1)
	DebuggerPrintf("Markov model built with %zu states.\n", m_states.size());
}

void MarkovSystem::BackwardParsing() // #ToDo Debug & fix to get it to fucking work
{

	for (const std::string& line : m_dataSet)
	{
		std::istringstream stream(line);
		std::vector<std::string> words;
		std::string word;

		// Tokenize line
		while (stream >> word)
		{
			GetHashedWordIndex(word);
			words.emplace_back(word);
		}

		// Add [START] and [END] explicitly
		std::string endToken = "[END]";
		std::string startToken = "[START]";
		words.emplace_back(endToken); // simulate sentence end
		std::reverse(words.begin(), words.end());
		words.emplace_back(startToken); // simulate sentence start (since we're going backwards)

		GetHashedWordIndex(endToken);
		GetHashedWordIndex(startToken);

		// Now slide over reversed list and create transitions
		for (size_t i = 0; i + m_order <= words.size() - 1; ++i)
		{
			std::deque<std::string> stateQueue;
			for (size_t j = 0; j < m_order; ++j)
			{
				stateQueue.emplace_back(words[i + j]);
			}

			size_t toWordIndex = GetHashedWordIndex(words[i + m_order]); // "previous" word in the original sentence
			CreateTransitionBetweenWords(stateQueue, toWordIndex, m_backwardTransitionMatrix, true);
		}
	}

	NormalizeTransitionMatrix(m_backwardTransitionMatrix);
	DebuggerPrintf("Markov model built with %zu states.\n", m_states.size());
}

size_t MarkovSystem::GetHashedWordIndex(const std::string& word)
{
	// Check if the word already exists in m_states
	auto it = std::find(m_states.begin(), m_states.end(), word);
	if (it != m_states.end())
	{
		// If found, return its index
		size_t index = std::distance(m_states.begin(), it);
		return index;
	}
	else
	{
		// If not found, add it to m_states and return its new index
		m_states.emplace_back(word);
		return m_states.size() - 1;
	}
}

void MarkovSystem::CreateTransitionBetweenWords(const std::deque<std::string>& stateQueue, size_t toStateIndex, std::unordered_map<IntVec4<size_t>, std::vector<WeightedWord>>& matrix, bool isBackward /*= false*/)
{
	if (stateQueue.size() < m_order) return;

	IntVec4<size_t> hashedState = isBackward ? GetBackwardStateFromQueue(stateQueue) : GetForwardStateFromQueue(stateQueue);

	std::vector<WeightedWord>& transitions = matrix[hashedState];

	DebuggerPrintf("Adding transition key: [%llu, %llu, %llu, %llu] = [%s, %s, %s, %s]\n",
		hashedState.x, hashedState.y, hashedState.z, hashedState.w,
		(hashedState.x != INVALID_INDEX ? m_states[hashedState.x].c_str() : "N/A"),
		(hashedState.y != INVALID_INDEX ? m_states[hashedState.y].c_str() : "N/A"),
		(hashedState.z != INVALID_INDEX ? m_states[hashedState.z].c_str() : "N/A"),
		(hashedState.w != INVALID_INDEX ? m_states[hashedState.w].c_str() : "N/A")
	);

	auto it = std::find_if(transitions.begin(), transitions.end(), [&](const WeightedWord& word) { return word.m_wordIndex == toStateIndex; });

	if (it != transitions.end())
	{
		it->m_probability += 1.f;
	}
	else
	{
		transitions.emplace_back(static_cast<int>(toStateIndex), 1.f);
	}
}

IntVec4<size_t> MarkovSystem::GetForwardStateFromQueue(const std::deque<std::string>& stateQueue)
{
	IntVec4<size_t> key(INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX);

	if (stateQueue.size() >= 1) key.x = GetHashedWordIndex(stateQueue[0]);
	if (stateQueue.size() >= 2) key.y = GetHashedWordIndex(stateQueue[1]);
	if (stateQueue.size() >= 3) key.z = GetHashedWordIndex(stateQueue[2]);
	if (stateQueue.size() >= 4) key.w = GetHashedWordIndex(stateQueue[3]);
	return key;
}

IntVec4<size_t> MarkovSystem::GetBackwardStateFromQueue(const std::deque<std::string>& stateQueue)
{
	IntVec4<size_t> key(INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX);
	size_t size = stateQueue.size();

	if (size >= 1) key.x = GetHashedWordIndex(stateQueue[size - 1]);
	if (size >= 2) key.y = GetHashedWordIndex(stateQueue[size - 2]);
	if (size >= 3) key.z = GetHashedWordIndex(stateQueue[size - 3]);
	if (size >= 4) key.w = GetHashedWordIndex(stateQueue[size - 4]);

	return key;
}

void MarkovSystem::NormalizeTransitionMatrix(std::unordered_map<IntVec4<size_t>, std::vector<WeightedWord>>& matrix)
{
	RandomNumberGenerator rng;
	rng.SetSeed(GetRandomSeedFromTime());

	for (auto& entry : matrix)
	{
		std::vector<WeightedWord>& transitions = entry.second;
		float totalProbability = 0.f;

		// Extract the correct state index from IntVec4<size_t>
		std::string stateStr = "[Unknown]";
		std::vector<int> indices = {
			static_cast<int>(entry.first.x),
			static_cast<int>(entry.first.y),
			static_cast<int>(entry.first.z),
			static_cast<int>(entry.first.w)
		};

		// Convert indices into words (only valid ones)
		std::vector<std::string> words;
		for (int index : indices)
		{
			if (index >= 0 && index < static_cast<int>(m_states.size())) // Ignore invalid index
			{
				words.emplace_back(m_states[index]);
			}
		}

		// Join words into a single string
		if (!words.empty())
		{
			stateStr = words[0]; // Start with the first word
			for (size_t i = 1; i < words.size(); i++)
			{
				stateStr += " " + words[i]; // Concatenate with spaces
			}
		}

		//DebuggerPrintf("Before Normalization [%s]:\n", stateStr.c_str());
		for (WeightedWord& word : transitions)
		{
			std::string wordStr = "[Unknown]";
			if (word.m_wordIndex < m_states.size())
			{
				wordStr = m_states[word.m_wordIndex];
			}
			totalProbability += word.m_probability;

			//DebuggerPrintf("   -> [%s] prob: %f\n", wordStr.c_str(), word.m_probability);
		}

		// Normalize probabilities
		if (totalProbability > 0.f)
		{
			float sumAfterRandomization = 0.f;
			for (WeightedWord& word : transitions)
			{
				word.m_probability /= totalProbability; // First, normalize properly
				float randomProbFactor = rng.SRollRandomFloatInRange(0.9f, 1.1f);
				word.m_probability *= randomProbFactor; // Apply randomization AFTER normalization

				sumAfterRandomization += word.m_probability;
			}

			// Second normalization pass to ensure sum is exactly 1.0
			if (sumAfterRandomization > 0.f)
			{
				for (WeightedWord& word : transitions)
				{
					word.m_probability /= sumAfterRandomization; // Re-normalize again!
				}
			}

			// Attempts to minimize the precision error
			if (!transitions.empty())
			{
				float sumFix = 1.0f - sumAfterRandomization;
				transitions.back().m_probability += sumFix; // Adjust last probability slightly
			}
		}

		// Print after normalization with actual words
		if (m_config.m_enableForward)
		{
			PrintTransitions(BuildStateString(words), transitions);
		}
		else if (m_config.m_enableBackward)
		{
			PrintTransitions(BuildStateString(words), transitions);
		}
	}
}

void MarkovSystem::PrintTransitions(const std::string& stateString, const std::vector<WeightedWord>& transitions)
{
	DebuggerPrintf("After Normalization [%s]:\n", stateString.c_str());

	for (const WeightedWord& word : transitions)
	{
		std::string wordStr = "[Unknown]";
		if (word.m_wordIndex < m_states.size())
		{
			wordStr = m_states[word.m_wordIndex];
		}
		DebuggerPrintf("   -> [%s] prob: %f\n", wordStr.c_str(), word.m_probability);
	}
}

std::string MarkovSystem::BuildStateString(const std::vector<std::string>& words)
{
	if (words.empty()) return "[Unknown]";
	std::string wordString = words[0];
	for (size_t i = 1; i < words.size(); ++i)
	{
		wordString += " " + words[i];
	}
	return wordString;
}

//------------------------------------------------------------------------------------------------------------------------

bool MarkovSystem::Command_StartupMarkov([[maybe_unused]] EventArgs& args)
{
	if (!g_theConsole->m_hasShownGreeting)
	{
		g_theConsole->m_hasShownGreeting = true;
		g_theConsole->m_shouldShowGreeting = true;
		std::string timeGreeting = g_theMarkov->GetTimeGreeting();
		std::string greeting = Stringf("%s %s", timeGreeting.c_str(), g_theMarkov->m_username.c_str());
		
		g_theConsole->AddCenterLine(DevConsole::MARKOV_INFO, "", 1.5f); // Spacer
		g_theConsole->AddCenterLine(Rgba8::WHITE, greeting, 2.f);
		g_theConsole->AddCenterLine(DevConsole::MARKOV_INFO, "", 1.5f); // Spacer
	}

	g_theConsole->AddLine(DevConsole::INFO_MINOR, "________________________", 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Markov System Startup Information:", 1.5f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "------------------------", 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Current Order: " + std::to_string(g_theMarkov->m_order), 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Default number of responses: " + std::to_string(g_theMarkov->m_numResponses), 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Default minimum response length: " + std::to_string(g_theMarkov->m_minResponseLength), 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Default maximum response length: " + std::to_string(g_theMarkov->m_maxResponseLength), 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Markov-Specific Commands:", 1.5f);

	g_theConsole->AddCenterLine(DevConsole::MARKOV_INFO, "", 1.5f); // Spacer

	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Available Topics:", 1.5f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "- GameDev", 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "- Sports", 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "- Economics", 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "- Education", 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "- ClimateChange", 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "- Healthcare", 1.25f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "- Other (default if unrecognized)", 1.25f);

	g_theMarkov->RegisterAllMarkovCommands();

	for (const std::string command : g_theMarkov->m_markovCommands)
	{
		g_theConsole->AddLine(DevConsole::MARKOV_INFO, "- " + command, 1.25f);
	}
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "------------------------", 1.25f);

	return true;
}

bool MarkovSystem::Command_ReBuildMarkov(EventArgs& args)
{
	int order = std::stoi(args.GetValue<std::string>("order", "1"));

	g_theMarkov->Build(order);

	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Markov model built successfully.");

	return true;
}

bool MarkovSystem::Command_GenerateForward([[maybe_unused]] EventArgs& args)
{
	int numResponses = g_theMarkov->GetNumResponses();
	int minimumResponseLength = g_theMarkov->GetMinimumResponseLength();
	int maximumResponseLength = g_theMarkov->GetMaximumResponseLength();

	if (numResponses <= 0)
	{
		g_theConsole->AddLine(DevConsole::ERROR, "Error: Please set a value greater than 0 for number of responses.");
		return false;
	}
	else if (minimumResponseLength <= 0)
	{
		g_theConsole->AddLine(DevConsole::ERROR, "Error: Please set a value greater than 0 fro the response length.");
		return false;
	}
	else
	{
		for (int i = 0; i < numResponses; i++)
		{
			g_theMarkov->GenerateResponseForward(minimumResponseLength, maximumResponseLength);
		}
	}

	return true;
}

bool MarkovSystem::Command_GenerateBackward([[maybe_unused]] EventArgs& args)
{
	int numResponses = g_theMarkov->GetNumResponses();
	int minimumResponseLength = g_theMarkov->GetMinimumResponseLength();
	int maximumResponseLength = g_theMarkov->GetMaximumResponseLength();

	if (numResponses <= 0)
	{
		g_theConsole->AddLine(DevConsole::ERROR, "Error: Please set a value greater than 0 for number of responses.");
		return false;
	}
	else if (minimumResponseLength <= 0)
	{
		g_theConsole->AddLine(DevConsole::ERROR, "Error: Please set a value greater than 0 fro the response length.");
		return false;
	}
	else
	{
		for (int i = 0; i < numResponses; i++)
		{
			g_theMarkov->GenerateResponseBackward(minimumResponseLength, maximumResponseLength);
		}
	}

	return true;
}

bool MarkovSystem::Command_SetOrder(EventArgs& args)
{
	int order = std::stoi(args.GetValue<std::string>("order", "1"));

	g_theMarkov->SetMarkovOrder(order);

	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Markov model order has successfully been changed to: " + std::to_string(order));

	return false;
}

bool MarkovSystem::Command_SetResponseLength(EventArgs& args)
{
	int length = std::stoi(args.GetValue<std::string>("length", "10"));

	g_theMarkov->SetResponseLength(length);

	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Markov model response length has successfully been changed to: " + std::to_string(length));

	return false;
}

bool MarkovSystem::Command_SetResponseNumber(EventArgs& args)
{
	int responseNumber = std::stoi(args.GetValue<std::string>("number", "1"));

	g_theMarkov->SetNumResponses(responseNumber);

	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Markov model number of responses has successfully been changed to: " + std::to_string(responseNumber));

	return false;
}

bool MarkovSystem::Command_DisplayStates([[maybe_unused]] EventArgs& args)
{
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "---------------------------");
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "List of generated States!");
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "---------------------------");

	g_theMarkov->DisplayAvailableStates();

	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "---------------------------");

	return false;
}

bool MarkovSystem::Command_SetUsername(EventArgs& args)
{
	std::string newName = args.GetValue<std::string>("name", "");
	if (newName.empty())
	{
		g_theConsole->AddLine(DevConsole::ERROR, "Usage: setusername name=YourName");
		return false;
	}

	g_theMarkov->m_username = newName;
	g_theMarkov->SaveUsername();
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Username set to: " + newName);
	return true;
}	

bool MarkovSystem::Command_ResetUsername([[maybe_unused]] EventArgs& args)
{
	g_theMarkov->m_username = g_theMarkov->GetSystemUsername();
	g_theMarkov->SaveUsername();
	g_theConsole->AddLine(DevConsole::INFO_MAJOR, "Username reset to system default");
	return true;
}

bool MarkovSystem::Command_LoadDataSet(EventArgs& args)
{
	std::string topic = args.GetValue<std::string>("topic", "Other");
	
	g_theConsole->m_isPreparingToLoadDataSet = true;
	g_theConsole->m_topicToLoadNextFrame = topic;


	return true;
}

bool MarkovSystem::Command_Response(EventArgs& args) // #ToDo remove m_pending 
{
	std::string response = args.GetValue<std::string>("response", "no");
	if (response == "yes")
	{
		std::string command = g_theMarkov->m_lastUsedForwardMode ? "generateforward" : "generatebackward";
		g_theConsole->Execute(command);
	}
	else
	{
		g_theMarkov->ClearAllData();
		g_theConsole->AddLine(DevConsole::MARKOV_INFO, "What topic would you like to switch to?");
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------------------

void MarkovSystem::Build(int order)
{
	if (m_dataSet.empty())
	{
		g_theConsole->AddLine(DevConsole::ERROR, "Data set is empty. Cannot build Markov Model.");
		return;
	}

	m_order = order;

	m_states.clear();
	m_forwardTransitionMatrix.clear();
	m_backwardTransitionMatrix.clear();

	// Rebuild dataset with the new order
	ParseLoadedDataSet();
}

void MarkovSystem::GenerateResponseForward(int minResponseLength, int maxResponseLength)
{
	std::deque<std::string> stateQueue;
	stateQueue.emplace_back("[START]");

	RandomNumberGenerator rng;
	if (m_config.m_seedNumber == -1)
	{
		rng.SetSeed(GetRandomSeedFromTime());
	}
	else
	{
		rng.SetSeed(m_config.m_seedNumber);
	}

	int randomStartLine = rng.SRollRandomIntInRange(0, static_cast<int>(m_dataSet.size()) - 1);

	// Add the first `m_order - 1` words from the dataset after [START]
	if (randomStartLine < static_cast<int>(m_dataSet.size()))
	{
		std::istringstream stream(m_dataSet[randomStartLine]); // Pick random line
		std::string word;

		// Read exactly (m_order - 1) words
		for (int j = 0; j < m_order - 1 && stream >> word; j++)
		{
			stateQueue.emplace_back(word);
			DebuggerPrintf("First State(s) Selected: [%s]\n", word.c_str());
		}
	}

	std::string result;
	int wordCount = 0;

	while (wordCount < maxResponseLength) // Continue until max length + 1 to account for forced [END]
	{
		if (stateQueue.size() < m_order)
		{
			g_theConsole->AddLine(DevConsole::ERROR, "StateQueue does not have enough elements!");
			return;
		}

		IntVec4<size_t> currentState(INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX);
		if (stateQueue.size() >= 1)
		{
			auto it = std::find(m_states.begin(), m_states.end(), stateQueue[0]);
			if (it != m_states.end())
			{
				currentState.x = std::distance(m_states.begin(), it);
			}
			else
			{
				g_theConsole->AddLine(DevConsole::ERROR, "StateQueue[0] not found in m_states!");
				return;
			}
		}
		if (stateQueue.size() >= 2)
		{
			auto it = std::find(m_states.begin(), m_states.end(), stateQueue[1]);
			if (it != m_states.end())
			{
				currentState.y = std::distance(m_states.begin(), it);
			}
			else
			{
				g_theConsole->AddLine(DevConsole::ERROR, "StateQueue[1] not found in m_states!");
				return;
			}
		}
		if (stateQueue.size() >= 3)
		{
			auto it = std::find(m_states.begin(), m_states.end(), stateQueue[2]);
			if (it != m_states.end())
			{
				currentState.z = std::distance(m_states.begin(), it);
			}
			else
			{
				g_theConsole->AddLine(DevConsole::ERROR, "StateQueue[2] not found in m_states!");
				return;
			}
		}
		if (stateQueue.size() >= 4)
		{
			auto it = std::find(m_states.begin(), m_states.end(), stateQueue[3]);
			if (it != m_states.end())
			{
				currentState.w = std::distance(m_states.begin(), it);
			}
			else
			{
				g_theConsole->AddLine(DevConsole::ERROR, "StateQueue[3] not found in m_states!");
				return;
			}
		}

		// Look up the current state in the transition matrix
		auto it = m_forwardTransitionMatrix.find(currentState);
		if (it == m_forwardTransitionMatrix.end())
		{
			g_theConsole->AddLine(DevConsole::ERROR, "Error: State not found in the Markov model!");
			return;
		}

		const std::vector<WeightedWord>& nextWordCandidates = it->second;
		float randomValue = rng.SRollRandomFloatZeroToOne();
		float cumulativeProbability = 0.f;
		bool isNextStateFound = false;

		// Select the next word based on probability
		for (const WeightedWord& weightedWord : nextWordCandidates)
		{
			cumulativeProbability += weightedWord.m_probability;

			// Check that ensures that the probability is valid (meaning above 0)
			if (cumulativeProbability <= 0.f || cumulativeProbability >= 1.1f)
			{
				std::string nextWord = m_states[weightedWord.m_wordIndex];
				DebuggerPrintf("Next State Selected: [%s]\n", nextWord.c_str());
				return;
			}

			if (randomValue <= cumulativeProbability)
			{
				std::string nextWord = m_states[weightedWord.m_wordIndex];

				DebuggerPrintf("Next State Selected: [%s]\n", nextWord.c_str());

				// Handle [END] rules
				if (nextWord == "[END]")
				{
					if (wordCount < minResponseLength)
					{
						bool foundAlternativeWord = false;
						for (const WeightedWord& alternativeWord : m_forwardTransitionMatrix[currentState])
						{
							if (m_states[alternativeWord.m_wordIndex] != "[END]")
							{
								nextWord = m_states[alternativeWord.m_wordIndex];
								foundAlternativeWord = true;
								break;
							}
						}

						if (!foundAlternativeWord)
						{
							g_theConsole->AddLine(DevConsole::MARKOV_INFO, "Generated Text:", 1.25f);
							WrapTextResult(result);
							m_conversationLog.push_back(result);
							SaveSession();
							m_lastUsedForwardMode = true;
							RunConversationLoop();
							return;
						}
					}
					else
					{
						g_theConsole->AddLine(DevConsole::MARKOV_INFO, "Generated Text:", 1.25f);
						WrapTextResult(result);
						m_conversationLog.push_back(result);
						SaveSession();
						m_lastUsedForwardMode = true;
						RunConversationLoop();
						return;
					}
				}

				if (result.empty())
				{
					for (const std::string& word : stateQueue)
					{
						if (word != "[START]") // Ensure we skip the start token
						{
							if (!result.empty()) result += " ";
							result += word;
						}
					}
				}

				// Add the next state word to the result
				size_t spaceIndex = nextWord.find_last_of(' ');
				if (spaceIndex != std::string::npos)
				{
					std::string extractedNextWord = nextWord.substr(spaceIndex + 1);
					if (!result.empty())
					{
						result += " "; // Add a space only if the result is not empty
					}

					result += extractedNextWord;

					stateQueue.pop_front();
					stateQueue.emplace_back(extractedNextWord);
				}
				else
				{
					if (nextWord == ".")
					{
						result += ".";
						stateQueue.pop_front();
						stateQueue.emplace_back(nextWord);
					}
					else
					{
						// Add next state with space
						if (!result.empty()) result += " "; // Add a space only if the result is not empty
						result += nextWord;
						stateQueue.pop_front();
						stateQueue.emplace_back(nextWord);
					}
				}

				isNextStateFound = true;
				wordCount++;
				break;
			}
		}

		if (!isNextStateFound && !nextWordCandidates.empty())
		{
			std::string nextWord = m_states[nextWordCandidates.back().m_wordIndex];
			DebuggerPrintf("Next State Selected: [%s]\n", nextWord.c_str());

			// Add the next state word to the result
			size_t spaceIndex = nextWord.find_last_of(' ');
			if (spaceIndex != std::string::npos)
			{
				std::string extractedNextWord = nextWord.substr(spaceIndex + 1);
				if (!result.empty())
				{
					result += " "; // Add a space only if the result is not empty
				}

				result += extractedNextWord;

				stateQueue.pop_front();
				stateQueue.emplace_back(extractedNextWord);
			}
			else
			{
				if (nextWord == ".")
				{
					result += ".";
					stateQueue.pop_front();
					stateQueue.emplace_back(nextWord);
				}
				else
				{
					// Add next state with space
					if (!result.empty()) result += " "; // Add a space only if the result is not empty
					result += nextWord;
					stateQueue.pop_front();
					stateQueue.emplace_back(nextWord);
				}
			}
			isNextStateFound = true;
			wordCount++;
		}
		else if (!isNextStateFound && nextWordCandidates.empty())
		{
			g_theConsole->AddLine(DevConsole::ERROR, "Generated Text (Incomplete):" + result);
			return;
		}
	}

	// Force [END] if exceeded max length
	if (wordCount >= maxResponseLength)
	{
		while (true)
		{
			IntVec4<size_t> currentState(INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX);
			if (stateQueue.size() >= 1)
			{
				auto it = std::find(m_states.begin(), m_states.end(), stateQueue[0]);
				if (it != m_states.end())
				{
					currentState.x = std::distance(m_states.begin(), it);
				}
				else
				{
					g_theConsole->AddLine(DevConsole::ERROR, "StateQueue[0] not found in m_states!");
					return;
				}
			}
			if (stateQueue.size() >= 2)
			{
				auto it = std::find(m_states.begin(), m_states.end(), stateQueue[1]);
				if (it != m_states.end())
				{
					currentState.y = std::distance(m_states.begin(), it);
				}
				else
				{
					g_theConsole->AddLine(DevConsole::ERROR, "StateQueue[1] not found in m_states!");
					return;
				}
			}
			if (stateQueue.size() >= 3)
			{
				auto it = std::find(m_states.begin(), m_states.end(), stateQueue[2]);
				if (it != m_states.end())
				{
					currentState.z = std::distance(m_states.begin(), it);
				}
				else
				{
					g_theConsole->AddLine(DevConsole::ERROR, "StateQueue[2] not found in m_states!");
					return;
				}
			}
			if (stateQueue.size() >= 4)
			{
				auto it = std::find(m_states.begin(), m_states.end(), stateQueue[3]);
				if (it != m_states.end())
				{
					currentState.w = std::distance(m_states.begin(), it);
				}
				else
				{
					g_theConsole->AddLine(DevConsole::ERROR, "StateQueue[3] not found in m_states!");
					return;
				}
			}

			// Look up the current state in the transition matrix
			auto it = m_forwardTransitionMatrix.find(currentState);
			if (it == m_forwardTransitionMatrix.end())
			{
				g_theConsole->AddLine(DevConsole::ERROR, "Generated Text (Incomplete): " + result);
				return;
			}

			// Check if [END] is a valid transition
			const std::vector<WeightedWord>& nextWordCandidates = it->second;
			float cumulativeProbability = 0.f;

			for (const WeightedWord& weightedWord : nextWordCandidates)
			{
				cumulativeProbability += weightedWord.m_probability;

				if (m_states[weightedWord.m_wordIndex] == "[END]" && weightedWord.m_probability > 0.f)
				{
					g_theConsole->AddLine(DevConsole::MARKOV_INFO, "Generate Text (Forced End):", 1.25f);
					WrapTextResult(result);
					m_conversationLog.push_back(result);
					SaveSession();
					m_lastUsedForwardMode = true;
					RunConversationLoop();
					return;
				}
			}

			// If [END] is not valid, randomly pick the next state and continue
			float randomValue = rng.SRollRandomFloatZeroToOne();
			cumulativeProbability = 0.f;

			for (const WeightedWord& weightedWord : nextWordCandidates)
			{
				cumulativeProbability += weightedWord.m_probability;
				if (randomValue <= cumulativeProbability)
				{
					std::string nextState = m_states[weightedWord.m_wordIndex];
					size_t spaceIndex = nextState.find_last_of(' ');
					if (spaceIndex != std::string::npos)
					{
						std::string nextWord = nextState.substr(spaceIndex + 1);
						if (!result.empty()) result += " "; // Add a space only if the result is not empty
						result += nextWord;

						stateQueue.pop_front();
						stateQueue.emplace_back(nextState.substr(spaceIndex + 1));
					}
					else
					{
						if (nextState == ".")
						{
							result += ".";
							stateQueue.pop_front();
							stateQueue.emplace_back(nextState);
						}
						else
						{
							// Add next state with space
							if (!result.empty()) result += " "; // Add a space only if the result is not empty
							result += nextState;
							stateQueue.pop_front();
							stateQueue.emplace_back(nextState);
						}
					}

					wordCount++;
					break;
				}
			}
		}
	}
}

void MarkovSystem::GenerateResponseBackward(int minResponseLength, int maxResponseLength)
{
	if (m_order != 1)
	{
		g_theConsole->AddLine(DevConsole::ERROR, "Backward generate only supports order 1 right now.");
		return;
	}

	UNUSED(minResponseLength);
	std::deque<std::string> stateQueue;
	stateQueue.emplace_back("[END]");

	RandomNumberGenerator rng;
	if (m_config.m_seedNumber == -1)
	{
		rng.SetSeed(GetRandomSeedFromTime());
	}
	else
	{
		rng.SetSeed(m_config.m_seedNumber);
	}

	int randomStartLine = rng.SRollRandomIntInRange(0, static_cast<int>(m_dataSet.size()) - 1);

	if (randomStartLine < static_cast<int>(m_dataSet.size()))
	{
		std::istringstream stream(m_dataSet[randomStartLine]);
		std::vector<std::string> words;
		std::string word;

		while (stream >> word)
		{
			words.emplace_back(word);
		}

		for (int j = static_cast<int>(words.size()) - 1; j >= 0 && static_cast<int>(stateQueue.size()) < m_order; j--)
		{
			if (words[j] != "[END]")
			{
				stateQueue.emplace_back(words[j]);
				DebuggerPrintf("First State(s) Selected (Backwards): [%s]\n", words[j].c_str());
			}
		}

		std::reverse(stateQueue.begin(), stateQueue.end());
	}

	std::string result;
	int wordCount = 0;

	while (wordCount < maxResponseLength)
	{
		if (stateQueue.size() < m_order)
		{
			g_theConsole->AddLine(DevConsole::ERROR, "StateQueue does not have enough elements!");
			return;
		}

		IntVec4<size_t> currentState = GetBackwardStateFromQueue(stateQueue);

		auto it = m_backwardTransitionMatrix.find(currentState);
		if (it == m_backwardTransitionMatrix.end())
		{
			g_theConsole->AddLine(DevConsole::ERROR, "Error: State not found in the Markov model!");
			return;
		}

		const std::vector<WeightedWord>& nextWordCandidates = it->second;
		float randomValue = rng.SRollRandomFloatZeroToOne();
		float cumulativeProbability = 0.f;
		bool isNextStateFound = false;

		for (const WeightedWord& weightedWord : nextWordCandidates)
		{
			cumulativeProbability += weightedWord.m_probability;

			if (randomValue <= cumulativeProbability)
			{
				std::string nextWord = m_states[weightedWord.m_wordIndex];
				DebuggerPrintf("Next State Selected: [%s]\n", nextWord.c_str());
				//g_theConsole->AddLine(DevConsole::MARKOV_INFO, "Next State: " + nextWord);

				if (nextWord == "[START]")
				{
					g_theConsole->AddLine(DevConsole::MARKOV_INFO, "Generated Text:", 1.25f);
					WrapTextResult(result);
					m_conversationLog.push_back(result);
					SaveSession();
					m_lastUsedForwardMode = false;
					RunConversationLoop();
					return;
				}

				stateQueue.pop_back();
				stateQueue.emplace_front(nextWord);

				if (!result.empty())
				{
					result = nextWord + " " + result;
				}
				else
				{
					result = nextWord;
				}
				isNextStateFound = true;
				wordCount++;
				break;
			}
		}

		if (!isNextStateFound && !nextWordCandidates.empty())
		{
			std::string fallbackWord = m_states[nextWordCandidates.front().m_wordIndex];
			DebuggerPrintf("Fallback State Selected: [%s]\n", fallbackWord.c_str());
			stateQueue.pop_back();
			stateQueue.emplace_front(fallbackWord);

			if (!result.empty())
			{
				result = fallbackWord + " " + result;
			}
			else
			{
				result = fallbackWord;
			}
			wordCount++;
		}
		else if (!isNextStateFound)
		{
			g_theConsole->AddLine(DevConsole::ERROR, "Generated Text (Incomplete):" + result);
			return;
		}
	}

	std::reverse(result.begin(), result.end());
	WrapTextResult(result);
	m_conversationLog.push_back(result);
	SaveSession();
	m_lastUsedForwardMode = false;
	RunConversationLoop();
}

void MarkovSystem::RunConversationLoop()
{
	RandomNumberGenerator rng;
	rng.SetSeed(GetRandomSeedFromTime());
	std::string prompt = rng.GetRandomElement(m_followUpPrompts);
	g_theConsole->AddLine(Rgba8::MAGENTA, prompt, 1.5f);
	g_theConsole->AddLine(DevConsole::INFO_MINOR, "Type: yes | no", 1.25f);
}

void MarkovSystem::DisplayAvailableStates()
{
	if (!m_forwardTransitionMatrix.empty())
	{
		for (const auto& pair : m_forwardTransitionMatrix)
		{
			const IntVec4<size_t>& state = pair.first;

			std::string stateStr = "[" + std::to_string(state.x) + ", " +
				std::to_string(state.y) + ", " +
				std::to_string(state.z) + ", " +
				std::to_string(state.w) + "]";

			g_theConsole->AddLine(DevConsole::INFO_MAJOR, "State: " + stateStr);

			for (const auto& transition : pair.second)
			{
				std::string nextWord = m_states[transition.m_wordIndex];
				DebuggerPrintf(" -> [%s] (Prob: %f)\n", nextWord.c_str(), transition.m_probability);
				g_theConsole->AddLine(DevConsole::INFO_MINOR, " -> " + nextWord + " (" + std::to_string(transition.m_probability) + ")");
			}
		}
	}
	else if (!m_backwardTransitionMatrix.empty())
	{
		for (const auto& pair : m_backwardTransitionMatrix)
		{
			const IntVec4<size_t>& state = pair.first;

			std::string stateStr = "[" + std::to_string(state.x) + ", " +
				std::to_string(state.y) + ", " +
				std::to_string(state.z) + ", " +
				std::to_string(state.w) + "]";

			g_theConsole->AddLine(DevConsole::INFO_MAJOR, "State: " + stateStr);

			for (const auto& transition : pair.second)
			{
				std::string nextWord = m_states[transition.m_wordIndex];
				DebuggerPrintf(" -> [%s] (Prob: %f)\n", nextWord.c_str(), transition.m_probability);
				g_theConsole->AddLine(DevConsole::INFO_MINOR, " -> " + nextWord + " (" + std::to_string(transition.m_probability) + ")");
			}
		}
	}
}

void MarkovSystem::WrapTextResult(const std::string& result, float fontScale /*= 1.25f*/)
{
	int consoleWidth = g_theWindow->GetClientDimensions().x;
	float lineHeight = g_theWindow->GetClientDimensions().y / g_theConsole->GetNumLines();
	float totalTextWidth = g_bitmapFont->GetTextWidth(lineHeight, result.c_str(), 1.f);
	float avgCharWidth = totalTextWidth / result.size();
	int maxCharsPerLine = static_cast<int>(consoleWidth / avgCharWidth);

	std::istringstream iss(result);
	std::string word;
	std::string currentLine;
	std::vector<std::string> wrappedLines;

	while (iss >> word)
	{
		if (currentLine.length() + word.length() + 1 > maxCharsPerLine)
		{
			wrappedLines.push_back(currentLine);
			currentLine = word;
		}
		else
		{
			if (!currentLine.empty())
			{
				currentLine += " ";
			}
			currentLine += word;
		}
	}

	if (!currentLine.empty())
	{
		wrappedLines.push_back(currentLine);
	}

	for (const std::string& line : wrappedLines)
	{
		g_theConsole->AddLine(DevConsole::RESPOND_TEXT, line, fontScale);
	}
}

void MarkovSystem::SetMarkovOrder(int markovOrderNumber)
{
	m_order = markovOrderNumber;
}

void MarkovSystem::SetResponseLength(int responseLength)
{
	m_minResponseLength = responseLength;
}

void MarkovSystem::SetNumResponses(int responseNumber)
{
	m_numResponses = responseNumber;
}

int MarkovSystem::GetMarkovOrder() const
{
	return m_order;
}

int MarkovSystem::GetMinimumResponseLength() const
{
	return m_minResponseLength;
}

int MarkovSystem::GetMaximumResponseLength() const
{
	return m_maxResponseLength;
}

int MarkovSystem::GetNumResponses() const
{
	return m_numResponses;
}

std::string MarkovSystem::GetSystemUsername()
{
#if defined(_WIN32) || defined(_WIN64)
	char* buffer = nullptr;
	size_t size = 0;
	if (_dupenv_s(&buffer, &size, "USERNAME") == 0 && buffer != nullptr)
	{
		std::string result(buffer);
		free(buffer);
		return result;
	}
#else
	const char* user = std::getenv("USER");
	if (user)
	{
		return std::string(user);
	}
#endif
	return "User";
}

std::string MarkovSystem::GetTimeGreeting()
{
	time_t now = time(nullptr);
	tm localTime;
#if defined(_WIN32)
	localtime_s(&localTime, &now);
#else
	localtime_r(&now, &localTime);
#endif

	int hour = localTime.tm_hour;
	std::vector<std::string> greetings = { "Hello", "Hey", "Hi", "Greetings", "Yo", "Howdy", "Hey there!"};
	std::string timeGreeting;
	if (hour < 12)
	{
		timeGreeting = "Good morning";
	}
	else if (hour < 18)
	{
		timeGreeting = "Good afternoon";
	}
	else
	{
		timeGreeting = "Good evening";
	}

	RandomNumberGenerator rng;
	rng.SetSeed(static_cast<unsigned int>(now));

	std::string chosenWord = rng.GetRandomElement(greetings);

	return Stringf("%s, %s", chosenWord.c_str(), timeGreeting.c_str());
}

void MarkovSystem::SaveUsername() const
{
	std::ofstream file("Data/user.cfg");
	if (file.is_open())
	{
		file << m_username << std::endl;
	}
}

void MarkovSystem::LoadUsername()
{
	std::ifstream file("Data/user.cfg");
	if (file.is_open())
	{
		std::getline(file, m_username);
	}

	if (m_username.empty())
	{
		m_username = GetSystemUsername(); // fall back to default
	}
}

void MarkovSystem::ClearAllData()
{
	m_dataSet.clear();
	m_states.clear();
	m_forwardTransitionMatrix.clear();
	m_backwardTransitionMatrix.clear();
}

void MarkovSystem::LoadDataSetByTopic(const std::string& topicNameInput)
{
	std::string path = "Data/DataSets/";
	std::string topicName = topicNameInput;
	std::transform(topicName.begin(), topicName.end(), topicName.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	if (topicName == "gamedev") path += "GameDev.txt";
	else if (topicName == "sports") path += "Sports.txt";
	else if (topicName == "economics") path += "Economics.txt";
	else if (topicName == "education") path += "Education.txt";
	else if (topicName == "climatechange") path += "ClimateChange.txt";
	else if (topicName == "healthcare") path += "Healthcare.txt";
	else path += "Other.txt";

	m_dataSet.clear();
	m_states.clear();
	m_forwardTransitionMatrix.clear();
	m_backwardTransitionMatrix.clear();

	LoadDataSet(path.c_str());
}

void MarkovSystem::SaveSession() const
{
	std::string path = "Data/Sessions/" + m_currentTopic + ".session";
	std::ofstream file(path);
	if (!file.is_open()) return;

	file << (m_lastUsedForwardMode ? "generateforward" : "generatebackward") << "\n";
	for (const std::string& line : m_conversationLog)
	{
		file << line << "\n";
	}
	file.close();
}

void MarkovSystem::LoadSessionForTopicPrompt()
{
	
}

void MarkovSystem::LoadSessionForTopic(const std::string& topic)
{
	m_currentTopic = topic;
	LoadDataSetByTopic(m_currentTopic);

	std::string path = "Data/Sessions/" + topic + ".session";
	std::ifstream file(path);
	if (!file.is_open()) return;

	std::string mode;
	std::getline(file, mode);
	m_lastUsedForwardMode = (mode == "generateforward");
	file.close();
}

//------------------------------------------------------------------------------------------------------------------------

void MarkovSystem::RegisterMarkovCommand(const std::string& commandName, EventCallbackFunction callback)
{
	g_theEventSystem->SubscribeEventCallbackFunction(commandName, callback);
	m_markovCommands.emplace_back(commandName);
}

void MarkovSystem::RegisterAllMarkovCommands()
{
	RegisterMarkovCommand("Topic", Command_LoadDataSet);
	RegisterMarkovCommand("Response", Command_Response);
	RegisterMarkovCommand("ReBuildMarkov", Command_ReBuildMarkov);
	RegisterMarkovCommand("GenerateForward", Command_GenerateForward);
	RegisterMarkovCommand("GenerateBackward", Command_GenerateBackward);
	RegisterMarkovCommand("DisplayStates", Command_DisplayStates);
	RegisterMarkovCommand("SetOrder", Command_SetOrder);
	RegisterMarkovCommand("SetResponseLength", Command_SetResponseLength);
	RegisterMarkovCommand("SetResponseNumber", Command_SetResponseNumber);
	RegisterMarkovCommand("SetUsername", Command_SetUsername);
	RegisterMarkovCommand("ResetUsername", Command_ResetUsername);
}
