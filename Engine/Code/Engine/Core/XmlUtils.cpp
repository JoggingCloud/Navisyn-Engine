#include "Engine/Core/XmlUtils.hpp"

int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	int result = defaultValue;
	if (element.Attribute(attributeName))
	{
		char const* value = element.Attribute(attributeName);
		result = std::stoi(value);
	}
	return result;
}

char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	char result = defaultValue;
	if (element.Attribute(attributeName))
	{
		char const* value = element.Attribute(attributeName);
		result = value[0];
	}
	return result;
}

bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	bool result = defaultValue;
	if (element.Attribute(attributeName))
	{
		char const* value = element.Attribute(attributeName);
		if (static_cast<std::string>((value)) == "true")
		{
			result = true;
		}

		if (static_cast<std::string>((value)) == "false")
		{
			result = false;
		}
	}
	return result;
}

float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	float result = defaultValue;
	if (element.Attribute(attributeName))
	{
		char const* value = element.Attribute(attributeName);
		result = static_cast<float>(std::stof(value));
	}
	return result;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	std::string result = defaultValue;
	if (element.Attribute(attributeName))
	{
		result = element.Attribute(attributeName);
	}
	return result;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	std::string result = defaultValue;
	if (element.Attribute(attributeName))
	{
		result = element.Attribute(attributeName);
	}
	return result;
}

Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	Rgba8 result = defaultValue;
	if (element.Attribute(attributeName))
	{
		result.SetFromText(element.Attribute(attributeName));
	}
	return result;
}

Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	Vec2 result = defaultValue;
	if (element.Attribute(attributeName))
	{
		result.SetFromText(element.Attribute(attributeName));
	}
	return result;
}

IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	IntVec2 result = defaultValue;
	if (element.Attribute(attributeName))
	{
		result.SetFromText(element.Attribute(attributeName));
	}
	return result;
}

Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	Vec3 result = defaultValue;
	if (element.Attribute(attributeName))
	{
		result.SetFromText(element.Attribute(attributeName));
	}
	return result;
}

EulerAngles ParseXmlAttribute(XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue)
{
	EulerAngles result = defaultValue;
	if (element.Attribute(attributeName))
	{
		result.SetFromText(element.Attribute(attributeName));
	}
	return result;
}

Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues)
{
	Strings result = defaultValues;
	if (element.Attribute(attributeName))
	{
		result = SplitStringOnDelimiter(element.Attribute(attributeName), ',');
	}
	return result;
}

FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange defaultValue)
{
	FloatRange result = defaultValue;
	if (element.Attribute(attributeName))
	{
		result.SetFromText(element.Attribute(attributeName));
	}
	return result;
}