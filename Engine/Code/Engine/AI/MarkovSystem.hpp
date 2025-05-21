#pragma once
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntVec4.hpp"

constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

struct WeightedWord
{
	int m_wordIndex;
	float m_probability;

	WeightedWord(int nextWordIndex, float probability)
		: m_wordIndex(nextWordIndex), m_probability(probability) {}
};

struct MarkovConfig
{
	std::string m_defaultDatasetPath; // Path to a default text file or xml file
	int m_seedNumber = 12346;		// Seed number for sentence generation
	int m_markovOrder = 1;			// Default Markov order (e.g., first-order, second-order)
	int m_responseLength = 10;		// Default number of words in generated response
	int m_memorySize = 5;			// Size of memory buffer for conversation history

	bool m_enableForward = false;
	bool m_enableBackward = false;
};

class MarkovSystem
{
public:
	MarkovSystem() = default;
	~MarkovSystem();

	MarkovSystem(MarkovConfig const& config);
	void Startup();
	void ShutDown();
	void BeginFrame();
	void EndFrame();

	void LoadDataSet(const std::string& dataSetFilePath);
	void ParseLoadedDataSet();
	void ForwardParsing();
	void BackwardParsing();

	size_t GetHashedWordIndex(const std::string& word);
	void CreateTransitionBetweenWords(const std::deque<std::string>& stateQueue, size_t toStateIndex, std::unordered_map<IntVec4<size_t>, std::vector<WeightedWord>>& matrix, bool isBackward = false);
	IntVec4<size_t> GetForwardStateFromQueue(const std::deque<std::string>& stateQueue);
	IntVec4<size_t> GetBackwardStateFromQueue(const std::deque<std::string>& stateQueue);
	void NormalizeTransitionMatrix(std::unordered_map<IntVec4<size_t>, std::vector<WeightedWord>>& matrix);

	// Utility function for debugging purposes only
	void PrintTransitions(const std::string& stateString, const std::vector<WeightedWord>& transitions);
	std::string BuildStateString(const std::vector<std::string>& words);

	// Initialize the System
	static bool Command_StartupMarkov(EventArgs& args);

	// ReBuild Markov
	static bool Command_ReBuildMarkov(EventArgs& args);

	// Generate Markov Forward
	static bool Command_GenerateForward(EventArgs& args);

	// Generate Markov Backward
	static bool Command_GenerateBackward(EventArgs& args);

	// Set Order for Markov
	static bool Command_SetOrder(EventArgs& args);

	// Set Length of Markov response
	static bool Command_SetResponseLength(EventArgs& args);

	// Set number of Markov responses
	static bool Command_SetResponseNumber(EventArgs& args);

	// Print available states that the user can use as a seed
	static bool Command_DisplayStates(EventArgs& args);

	// Set username
	static bool Command_SetUsername(EventArgs& args);

	// Reset username
	static bool Command_ResetUsername(EventArgs& args);

	// Load DataSet
	static bool Command_LoadDataSet(EventArgs& args);

	// Response to system
	static bool Command_Response(EventArgs& args);

public:
	void Build(int order);
	void GenerateResponseForward(int minResponseLength, int maxResponseLength);
	void GenerateResponseBackward(int minResponseLength, int maxResponseLength);

	void RunConversationLoop();

	void DisplayAvailableStates();
	void WrapTextResult(const std::string& result, float fontScale = 1.25f);

	void SetMarkovOrder(int markovOrderNumber);
	void SetResponseLength(int responseLength);
	void SetNumResponses(int responseNumber);

	int GetMarkovOrder() const;
	int GetMinimumResponseLength() const;
	int GetMaximumResponseLength() const;
	int GetNumResponses() const;

	std::string GetSystemUsername();
	std::string GetTimeGreeting();

	void SaveUsername() const;
	void LoadUsername();
	void ClearAllData();
	void LoadDataSetByTopic(const std::string& topicNameInput);

	void SaveSession() const;
	void LoadSessionForTopicPrompt();
	void LoadSessionForTopic(const std::string& topic);

protected:
	MarkovConfig m_config;

private:
	std::vector<std::string> m_dataSet; // Store the data set we loaded via text file
	std::vector<std::string> m_states; // Store the strings in a list just for reference
	
	std::vector<std::string> m_conversationLog;
	std::string m_username;
	std::string m_combinedText;
	std::string m_currentTopic;

	// Each value (std::vector<WeightedWord>) stores all possible next states with their probabilities 
	std::unordered_map<IntVec4<size_t>, std::vector<WeightedWord>> m_forwardTransitionMatrix; // Each key represents a state (a sequence of words) having a int it will be a much faster look up.
	std::unordered_map<IntVec4<size_t>, std::vector<WeightedWord>> m_backwardTransitionMatrix; // Each key represents a state (a sequence of words) having a int it will be a much faster look up.

	int m_order = 1;
	int m_minResponseLength = 10;
	int m_maxResponseLength = 50;
	int m_numResponses = 1;

	bool m_lastUsedForwardMode = true;

private:
	void RegisterMarkovCommand(const std::string& commandName, EventCallbackFunction callback);
	void RegisterAllMarkovCommands();

	std::vector<std::string> m_markovCommands;
	std::vector<std::string> m_followUpPrompts = { "Would you like to hear more?",
												   "Should I continue?",
												   "Want to explore that further?",
												   "Interested in another thought?",
												   "Shall I keep going?" 
												  };
};
