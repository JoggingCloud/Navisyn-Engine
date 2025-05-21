#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

extern RandomNumberGenerator g_rng;

FloatRange::~FloatRange()
{

}

FloatRange::FloatRange()
	: m_min(0.0f), m_max(0.0f)
{

}

FloatRange::FloatRange(float min, float max)
	: m_min(min), m_max(max)
{

}

float FloatRange::GetRandomValueInRange() const
{
	float randomFactor = g_rng.SRollRandomFloatZeroToOne();
	return m_min + randomFactor * (m_max - m_min);
}

bool FloatRange::IsOnRange(float value) const
{
	return (value >= m_min) && (value <= m_max);
}

bool FloatRange::IsOverlapingWith(const FloatRange& compare) const
{
	return (compare.IsOnRange(m_min) || compare.IsOnRange(m_max) || IsOnRange(compare.m_min) || IsOnRange(compare.m_max));
}

void FloatRange::SetFromText(char const* text)
{
	Strings parts = SplitStringOnDelimiter(text, '~');
	if (parts.size() == 2)
	{
		m_min = std::stof(parts[0]);
		m_max = std::stof(parts[1]);
	}
	else
	{
		m_min = m_max = 0.f;
	}
}

void FloatRange::operator=(const FloatRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

bool FloatRange::operator!=(const FloatRange& compare) const
{
	return (m_min != compare.m_min) || (m_max != compare.m_max);
}

bool FloatRange::operator==(const FloatRange& compare) const
{
	return (m_min == compare.m_min) && (m_max == compare.m_max);
}

// Static const objects initialization
const FloatRange FloatRange::ZERO(0.0f, 0.0f);
const FloatRange FloatRange::ONE(1.0f, 1.0f);
const FloatRange FloatRange::ZERO_TO_ONE(0.0f, 1.0f);