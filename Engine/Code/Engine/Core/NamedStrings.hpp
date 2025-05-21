#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include <map>
#include <string>

class NamedStrings
{
public:
	NamedStrings() = default;
	~NamedStrings();

	void			PopulateFromXmlElementAttributes(XmlElement const& element, bool isAdditional);
	void			ParseFromInput(const std::string& rawInput);
	void			SetValue(std::string const& keyName, std::string const& newValue);
	std::string		GetValue(std::string const& keyName, std::string const& defaultValue) const;
	bool			GetValue(std::string const& keyName, bool defaultValue) const;
	int			    GetValue(std::string const& keyName, int defaultValue) const;
	float			GetValue(std::string const& keyName, float defaultValue) const;
	std::string		GetValue(std::string const& keyName, char const* defaultValue) const;
	Rgba8			GetValue(std::string const& keyName, Rgba8 const& defaultValue) const;
	Vec2			GetValue(std::string const& keyName, Vec2 const& defaultValue) const;
	Vec3			GetValue(std::string const& keyName, Vec3 const& defaultValue) const;
	IntVec2		    GetValue(std::string const& keyName, IntVec2 const& defaultValue) const;
	bool            HasArgument(std::string const& keyName);

public:
	std::map< std::string, std::string >	m_keyValuePairs;

};