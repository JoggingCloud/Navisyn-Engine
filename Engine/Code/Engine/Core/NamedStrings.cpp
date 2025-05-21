#include "Engine/Core/NamedStrings.hpp"
#include <sstream>

NamedStrings::~NamedStrings()
{

}

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element, bool isAdditional)
{
	const XmlAttribute* attribute = element.FirstAttribute();

	while (attribute != nullptr)
	{
		if (isAdditional)
		{
			auto it = m_keyValuePairs.find(attribute->Name());
			if (it != m_keyValuePairs.end())
			{
				// Attribute already exists, decide whether to append or override.
				it->second += "," + std::string(attribute->Value());
			}
			else
			{
				// Attribute does not exist, add it.
				m_keyValuePairs[attribute->Name()] = attribute->Value();
			}
		}
		else
		{
			// Default behavior for adding new attributes.
			m_keyValuePairs[attribute->Name()] = attribute->Value();
		}
		attribute = attribute->Next();
	}
}

void NamedStrings::ParseFromInput(const std::string& rawInput)
{
	std::istringstream stream(rawInput);
	std::string pair;

	while (std::getline(stream, pair, ' '))
	{
		size_t separator = pair.find('=');
		if (separator != std::string::npos)
		{
			std::string key = pair.substr(0, separator);
			std::string value = pair.substr(separator + 1);
			SetValue(key, value);
		}
	}
}

void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		return it->second;
	}
	return defaultValue;
}

bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		if (it->second == "true")
		{
			return true;
		}

		if (it->second == "false")
		{
			return false;
		}
	}
	return defaultValue;
}

int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		return std::stoi(it->second);
	}
	return defaultValue;
}

float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		return std::stof(it->second);
	}
	return defaultValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	if (it != m_keyValuePairs.end())
	{
		return it->second.c_str();
	}
	return defaultValue;
}

Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);

	Rgba8 result = defaultValue;

	if (it != m_keyValuePairs.end())
	{
		result.SetFromText(it->second.c_str());
	}
	return result;
}

Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);

	Vec2 result = defaultValue;

	if (it != m_keyValuePairs.end())
	{
		result.SetFromText(it->second.c_str());
	}
	return result;
}

Vec3 NamedStrings::GetValue(std::string const& keyName, Vec3 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);

	Vec3 result = defaultValue;

	if (it != m_keyValuePairs.end())
	{
		result.SetFromText(it->second.c_str());
	}

	return result;
}

IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);
	
	IntVec2 result = defaultValue;

	if (it != m_keyValuePairs.end())
	{
		result.SetFromText(it->second.c_str());
	}
	return result;
}

bool NamedStrings::HasArgument(std::string const& keyName)
{
	std::map<std::string, std::string>::const_iterator it = m_keyValuePairs.find(keyName);

	if (it != m_keyValuePairs.end())
	{
		return true;
	}
	return false;
}
