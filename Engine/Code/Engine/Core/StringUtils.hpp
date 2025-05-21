#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <deque>

typedef std::vector< std::string >	Strings;

Strings SplitStringWithQuotes(std::string const& originalString, char delimiterToSplitOn, bool removeInsideQuotes = false);
Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn, bool removeEmpty = true);
Strings SplitStringOnDelimiter(std::string const& originalString, const std::string& delimiterToSplitOn, bool removeEmpty = true);
int SplitStringOnDelimiter(Strings& out_splitString, const std::string& originalString, const std::string& delimiterToSplitOn);

void TrimString(std::string& originalString, char delimiterToTrim);

//-----------------------------------------------------------------------------------------------
std::string Join(std::vector<std::string> const& words, std::string const& delimiter);
std::string Join(std::deque<std::string> const& words, std::string const& delimiter);
std::string ToLower(std::string const& originalString);
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

//-----------------------------------------------------------------------------------------------

class HashedCaseInsensitiveString
{
public:
	// Constructors
	HashedCaseInsensitiveString() = default;
	HashedCaseInsensitiveString(const HashedCaseInsensitiveString& other);  // Copy constructor
	HashedCaseInsensitiveString(const char* text);  // C-string constructor
	HashedCaseInsensitiveString(const std::string& text);  // std::string constructor

	// Accessors
	unsigned int GetHash() const { return m_caseInsensitiveHash; }
	const std::string& GetOriginalString() const { return m_originalString; }
	const char* c_str() const { return m_originalString.c_str(); }

	// Operators
	bool operator<(const HashedCaseInsensitiveString& compare) const;
	bool operator==(const HashedCaseInsensitiveString& compare) const;
	bool operator!=(const HashedCaseInsensitiveString& compare) const;
	bool operator==(const char* text) const;
	bool operator!=(const char* text) const;
	bool operator==(const std::string& text) const;
	bool operator!=(const std::string& text) const;

	void operator=(const HashedCaseInsensitiveString& assignFrom);
	void operator=(const char* text);
	void operator=(const std::string& text);

private:
	std::string m_originalString;
	unsigned int m_caseInsensitiveHash = 0;

	// Hashing function
	static unsigned int GenerateCaseInsensitiveHash(std::string const& text);
	static unsigned int GenerateCaseInsensitiveHash(const char* text);
};