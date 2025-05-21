#pragma once
#include <vector>
#include <stdexcept>

class BufferParser
{
public:
	BufferParser(const void* data, size_t size);
	BufferParser(const std::vector<uint8_t>& buffer);
	~BufferParser() = default;

	void ParseStringZeroTerminated(std::string& outString);
	void ParseStringAfter32BitLength(std::string& outString);

	template <typename T>
	T ParsePrimitive();

	void JumpTo(size_t absoluteOffset);

	bool IsAtEnd() const;
	size_t GetCurrentOffset() const;

private:
	const uint8_t* m_data = nullptr;
	size_t m_size = 0;
	size_t m_cursor = 0;
};

template<typename T>
T BufferParser::ParsePrimitive()
{
	if (m_cursor + sizeof(T) > m_size)
	{
		throw std::out_of_range("Attempt to read beyond buffer size");
	}

	T value;
	std::memcpy(&value, m_data + m_cursor, sizeof(T));
	m_cursor += sizeof(T);
	return value;
}
