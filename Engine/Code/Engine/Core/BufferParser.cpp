#include "Engine/Core/BufferParser.hpp"

BufferParser::BufferParser(const void* data, size_t size)
	: m_data(reinterpret_cast<const uint8_t*>(data)), m_size(size), m_cursor(0)
{
}

BufferParser::BufferParser(const std::vector<uint8_t>& buffer)
	: BufferParser(buffer.data(), buffer.size())
{
}

void BufferParser::ParseStringZeroTerminated(std::string& outString)
{
	outString.clear();

	while (m_cursor < m_size)
	{
		char c = ParsePrimitive<char>();
		if (c == '\0') 
		{
			break;
		}
		outString += c;
	}
}

void BufferParser::ParseStringAfter32BitLength(std::string& outString)
{
	uint32_t strLen = ParsePrimitive<uint32_t>();

	if (m_cursor + strLen > m_size) {
		throw std::out_of_range("String length exceeds buffer bounds");
	}

	outString = std::string(reinterpret_cast<const char*>(m_data + m_cursor), strLen);
	m_cursor += strLen;
}

void BufferParser::JumpTo(size_t absoluteOffset)
{
	if (absoluteOffset > m_size)
	{
		throw std::out_of_range("JumpTo offset is out of bounds");
	}
	m_cursor = absoluteOffset;
}

bool BufferParser::IsAtEnd() const
{
	return m_cursor >= m_size;
}

size_t BufferParser::GetCurrentOffset() const
{
	return m_cursor;
}
