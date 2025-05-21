#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>
#include <algorithm>
#include <cctype>
#include <sstream>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


Strings SplitStringWithQuotes(std::string const& originalString, char delimiterToSplitOn, bool removeInsideQuotes)
{
	Strings result;
	bool insideQuotes = false;
	std::string currentToken;

	for (char ch : originalString)
	{
		if (ch == '"')
		{
			// Toggle the insideQuotes flag
			insideQuotes = !insideQuotes;
			if (!removeInsideQuotes)
			{
				// Keep the quote if we are not removing them
				currentToken += ch;
			}
		}
		else if (ch == delimiterToSplitOn && !insideQuotes)
		{
			// Add the current token to the result and clear for the next one
			if (!currentToken.empty())
			{
				result.emplace_back(currentToken);
				currentToken.clear();
			}
		}
		else
		{
			// Add character to the current token
			currentToken += ch;
		}
	}

	// Add the last token if it exists
	if (!currentToken.empty())
	{
		result.emplace_back(currentToken);
	}

	// Trim quotes from tokens if they were inside quotes
	if (removeInsideQuotes)
	{
		for (std::string& token : result)
		{
			if (token.size() >= 2 && token.front() == '"' && token.back() == '"')
			{
				token = token.substr(1, token.size() - 2);  // Remove the leading and trailing quotes
			}
		}
	}

	return result;
}

Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn, bool removeEmpty)
{
	Strings result;

	// Start position for the first substring
	size_t startPos = 0;

	// Find the position of the first delimiter
	size_t delimiterPos = originalString.find(delimiterToSplitOn);

	// Loop until no more delimiters are found
	while (delimiterPos != std::string::npos)
	{
		// Extract the substring between startPos and delimiterPos
		std::string substring = originalString.substr(startPos, delimiterPos - startPos);

		// Add the substring to the result vector if it's not empty or if we don't want to remove empty strings
		if (!removeEmpty || !substring.empty())
		{
			// Add the substring to the result vector
			result.emplace_back(substring);
		}

		// Move the startPos to the next character after the delimiter
		startPos = delimiterPos + 1;

		// Find the position of the next delimiter
		delimiterPos = originalString.find(delimiterToSplitOn, startPos);
	}

	// Add the last substring (after the last delimiter)
	std::string lastSubstring = originalString.substr(startPos);
	if (!removeEmpty || !lastSubstring.empty())
	{
		result.emplace_back(lastSubstring);
	}

	return result;
}

Strings SplitStringOnDelimiter(std::string const& originalString, const std::string& delimiterToSplitOn, bool removeEmpty)
{
	Strings result;

	// Start position for the first substring
	size_t startPos = 0;

	// Find the position of the first delimiter
	size_t delimiterPos;

	// Loop until no more delimiters are found
	while ((delimiterPos = originalString.find(delimiterToSplitOn, startPos)) != std::string::npos)
	{
		std::string substring = originalString.substr(startPos, delimiterPos - startPos);
		if (!removeEmpty || !substring.empty())
		{
			result.emplace_back(substring);
		}
		startPos = delimiterPos + delimiterToSplitOn.length();
	}

	// Add the last substring (after the last delimiter)
	std::string lastSubstring = originalString.substr(startPos);
	if (!removeEmpty || !lastSubstring.empty())
	{
		result.emplace_back(lastSubstring);
	}

	return result;
}


int SplitStringOnDelimiter(Strings& out_splitString, const std::string& originalString, const std::string& delimiterToSplitOn)
{
	// Clear the output vector ensure it starts empty 
	out_splitString.clear();

	// Start position for the first substring
	size_t startPos = 0;

	// Find the position of the first delimiter
	size_t delimiterPos;

	// Loop until no more delimiters are found
	while ((delimiterPos = originalString.find(delimiterToSplitOn, startPos)) != std::string::npos)
	{
		std::string substring = originalString.substr(startPos, delimiterPos - startPos);
		if (!substring.empty())
		{
			out_splitString.emplace_back(substring);
		}
		startPos = delimiterPos + delimiterToSplitOn.length();
	}

	// Handle the last substring (after the final delimiter)
	if (startPos < originalString.size()) // Make sure we still have something to push
	{
		std::string lastSubstring = originalString.substr(startPos);
		if (!lastSubstring.empty())
		{
			out_splitString.emplace_back(lastSubstring);
		}
	}

	// Return the number of resulting substrings 
	return static_cast<int>(out_splitString.size());
}

void TrimString(std::string& originalString, char delimiterToTrim)
{
	if (!originalString.empty() && originalString.front() == delimiterToTrim)
	{
		originalString.erase(0, 1); // Remove the first character
	}
	if (!originalString.empty() && originalString.back() == delimiterToTrim)
	{
		originalString.pop_back(); // Remove the last character
	}
}

std::string Join(std::vector<std::string> const& words, std::string const& delimiter)
{
	std::ostringstream joined;
	for (size_t i = 0; i < words.size(); i++)
	{
		if (i > 0)
		{
			joined << delimiter; // Add delimiter (e.g., space) between words
		}
		joined << words[i]; // Append word
	}
	return joined.str();
}

std::string Join(std::deque<std::string> const& words, std::string const& delimiter)
{
	std::ostringstream joined;
	for (size_t i = 0; i < words.size(); i++)
	{
		if (i > 0)
		{
			joined << delimiter; // Add delimiter (e.g., space) between words
		}
		joined << words[i]; // Append word
	}
	return joined.str();
}

std::string ToLower(std::string const& originalString)
{
	std::string lowerCaseString = originalString;
	std::transform(lowerCaseString.begin(), lowerCaseString.end(), lowerCaseString.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
	return lowerCaseString;
}

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

//-----------------------------------------------------------------------------------------------

HashedCaseInsensitiveString::HashedCaseInsensitiveString(const HashedCaseInsensitiveString& other)
	: m_originalString(other.m_originalString), m_caseInsensitiveHash(other.m_caseInsensitiveHash)
{
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString(const char* text)
	: m_originalString(text) , m_caseInsensitiveHash(GenerateCaseInsensitiveHash(text))
{
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString(const std::string& text)
	: HashedCaseInsensitiveString(text.c_str())
{
}

bool HashedCaseInsensitiveString::operator<(const HashedCaseInsensitiveString& compare) const
{
// 	if (m_originalString < compare.m_originalString) return true;
// 	if (m_originalString > compare.m_originalString) return false;
// 	return _stricmp(m_originalString.c_str(), compare.m_originalString.c_str()) < 0;
	return _stricmp(m_originalString.c_str(), compare.m_originalString.c_str()) < 0;
}

bool HashedCaseInsensitiveString::operator==(const HashedCaseInsensitiveString& compare) const
{
	if (m_originalString != compare.m_originalString) return false;
	return _stricmp(m_originalString.c_str(), compare.m_originalString.c_str()) == 0;
}

bool HashedCaseInsensitiveString::operator!=(const HashedCaseInsensitiveString& compare) const
{
	return!(*this == compare);
}

bool HashedCaseInsensitiveString::operator==(const char* text) const
{
	return _stricmp(m_originalString.c_str(), text) == 0;
}

bool HashedCaseInsensitiveString::operator!=(const char* text) const
{
	return !(*this == text);
}

bool HashedCaseInsensitiveString::operator==(const std::string& text) const
{
	return *this == text.c_str();
}

bool HashedCaseInsensitiveString::operator!=(const std::string& text) const
{
	return *this == text;
}

void HashedCaseInsensitiveString::operator=(const HashedCaseInsensitiveString& assignFrom)
{
	if (this != &assignFrom)
	{
		m_originalString = assignFrom.m_originalString;
		m_caseInsensitiveHash = assignFrom.m_caseInsensitiveHash;
	}
}

void HashedCaseInsensitiveString::operator=(const char* text)
{
	m_originalString = text;
	m_caseInsensitiveHash = GenerateCaseInsensitiveHash(text);
}

void HashedCaseInsensitiveString::operator=(const std::string& text)
{
	*this = text.c_str();
}

unsigned int HashedCaseInsensitiveString::GenerateCaseInsensitiveHash(std::string const& text)
{
	return GenerateCaseInsensitiveHash(text.c_str());
}

unsigned int HashedCaseInsensitiveString::GenerateCaseInsensitiveHash(const char* text)
{
	unsigned int hash = 0;
	char const* scan = text;
	while (*scan != '\0')
	{
		hash *= 31;
		hash += static_cast<unsigned int>(std::tolower(*scan));
		scan++;
	}
	return hash;
}
