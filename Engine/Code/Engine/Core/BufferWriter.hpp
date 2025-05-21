#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/IntVec4.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <string>
#include <vector>
#include <type_traits>

enum class Endianness
{
	LITTLE,
	BIG,
	NATIVE
};

class BufferWriter
{
public:
	BufferWriter(std::vector<uint8_t>& buffer);
	~BufferWriter() = default;

	// Append functions
	template <typename T>
	void AppendPrimitive(T value, Endianness endian = Endianness::NATIVE);

	void AppendByte(uint8_t value)    { AppendPrimitive(value); }
	void AppendBool(bool value)		  { AppendPrimitive(value); }
	void AppendChar(char value)       { AppendPrimitive(value); }
	void AppendUShort(uint16_t value) { AppendPrimitive(value); }
	void AppendUInt32(uint32_t value) { AppendPrimitive(value); }
	void AppendUInt64(uint64_t value) { AppendPrimitive(value); }
	void AppendInt(int32_t value)     { AppendPrimitive(value); }
	void AppendInt64(int64_t value)   { AppendPrimitive(value); }
	void AppendFloat(float value)     { AppendPrimitive(value); }
	void AppendDouble(double value)   { AppendPrimitive(value); }

	void AppendStringZeroTerminated(const std::string& string);
	void AppendLengthPrefixed(const std::string& string);

	void AppendVec2(const Vec2& vector);
	void AppendVec3(const Vec3& vector);
	void AppendVec4(const Vec4& vector);
	
	void AppendIntVec2(const IntVec2& vector);
	void AppendIntVec3(const IntVec3& vector);

	template <typename T>
	void AppendIntVec4(const IntVec4<T>& vector);

	void AppendAABB2(const AABB2& aabb);
	void AppendAABB3(const AABB3& aabb);

	void AppendOBB2(const OBB2& obb);
	void AppendOBB3(const OBB3& obb);

	void AppendPlane2(const Plane2D& plane);
	void AppendPlane3(const Plane3D& plane);

	void AppendVertexPCU(const Vertex_PCU& pcu);
	void AppendVertexPCUTBN(const Vertex_PCUTBN& pcutbn);

	void AppendRgba8(const Rgba8& color);

	void OverwriteUInt32At(uint32_t value, size_t index);

	void SetEndianMode(Endianness endian);
	Endianness GetNativeEndianness();

	template <typename T>
	T SwapEndian(T value, Endianness desiredEndian);

private:
	std::vector<uint8_t>* m_buffer = nullptr;
	Endianness m_endian;
};

template <typename T>
void BufferWriter::AppendPrimitive(T value, Endianness endian)
{
	// std::is_trivially_copyable returns a boolean value true is T is trivially copyable tpe, otherwise returns false
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable"); 
	value = SwapEndian(value, endian);

	uint8_t const* data = reinterpret_cast<uint8_t const*>(&value);
	m_buffer->insert(m_buffer->end(), data, data + sizeof(T));
}

template <typename T>
void BufferWriter::AppendIntVec4(const IntVec4<T>& vector)
{
	AppendPrimitive(vector.x);
	AppendPrimitive(vector.y);
	AppendPrimitive(vector.z);
	AppendPrimitive(vector.w);
}

template <typename T>
T BufferWriter::SwapEndian(T value, Endianness desiredEndian)
{
	if (desiredEndian == Endianness::NATIVE || desiredEndian == GetNativeEndianness()) { return value; }

	T swapped = 0;
	uint8_t* destination = reinterpret_cast<uint8_t*>(&swapped);
	uint8_t const* source = reinterpret_cast<uint8_t const*>(&value);
	for (size_t i = 0; i < sizeof(T); i++) 
	{
		destination[i] = source[sizeof(T) - 1 - i];
	}
	return swapped;
}
