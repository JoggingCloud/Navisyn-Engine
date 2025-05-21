#include "Engine/Core/BufferWriter.hpp"

BufferWriter::BufferWriter(std::vector<uint8_t>& buffer)
	: m_buffer(&buffer)
{
}

void BufferWriter::AppendStringZeroTerminated(const std::string& string)
{
	m_buffer->insert(m_buffer->end(), string.begin(), string.end());
	m_buffer->emplace_back(static_cast<uint8_t>(0)); // Null terminator
}

void BufferWriter::AppendLengthPrefixed(const std::string& string)
{
	AppendUInt32(static_cast<uint32_t>(string.size()));
	m_buffer->insert(m_buffer->end(), string.begin(), string.end());
}

void BufferWriter::AppendVec2(const Vec2& vector)
{
	AppendFloat(vector.x);
	AppendFloat(vector.y);
}

void BufferWriter::AppendVec3(const Vec3& vector)
{
	AppendFloat(vector.x);
	AppendFloat(vector.y);
	AppendFloat(vector.z);
}

void BufferWriter::AppendVec4(const Vec4& vector)
{
	AppendFloat(vector.x);
	AppendFloat(vector.y);
	AppendFloat(vector.z);
	AppendFloat(vector.w);
}

void BufferWriter::AppendIntVec2(const IntVec2& vector)
{
	AppendInt(vector.x);
	AppendInt(vector.y);
}

void BufferWriter::AppendIntVec3(const IntVec3& vector)
{
	AppendInt(vector.x);
	AppendInt(vector.y);
	AppendInt(vector.z);
}

void BufferWriter::AppendAABB2(const AABB2& aabb)
{
	AppendVec2(aabb.m_mins);
	AppendVec2(aabb.m_maxs);
}

void BufferWriter::AppendAABB3(const AABB3& aabb)
{
	AppendVec3(aabb.m_mins);
	AppendVec3(aabb.m_maxs);
}

void BufferWriter::AppendOBB2(const OBB2& obb)
{
	AppendVec2(obb.m_center);
	AppendVec2(obb.m_halfDimensions);
	AppendVec2(obb.m_iBasisNormal);
}

void BufferWriter::AppendOBB3(const OBB3& obb)
{
	AppendVec3(obb.m_center);
	AppendVec3(obb.m_halfDimensions);
	AppendVec3(obb.m_iBasisNormal);
	AppendVec3(obb.m_jBasisNormal);
	AppendVec3(obb.m_kBasisNormal);
}

void BufferWriter::AppendPlane2(const Plane2D& plane)
{
	AppendVec2(plane.m_normal);
	AppendFloat(plane.m_distance);
}

void BufferWriter::AppendPlane3(const Plane3D& plane)
{
	AppendVec3(plane.m_normal);
	AppendFloat(plane.m_distance);
}

void BufferWriter::AppendVertexPCU(const Vertex_PCU& pcu)
{
	AppendVec3(pcu.m_position);
	AppendRgba8(pcu.m_color);
	AppendVec2(pcu.m_uvTexCoords);
}

void BufferWriter::AppendVertexPCUTBN(const Vertex_PCUTBN& pcutbn)
{
	AppendVec3(pcutbn.m_position);
	AppendRgba8(pcutbn.m_color);
	AppendVec2(pcutbn.m_uvTexCoords);
	AppendVec3(pcutbn.m_tangent);
	AppendVec3(pcutbn.m_bitangent);
	AppendVec3(pcutbn.m_normal);
}

void BufferWriter::AppendRgba8(const Rgba8& color)
{
	AppendByte(color.r);
	AppendByte(color.g);
	AppendByte(color.b);
	AppendByte(color.a);
}

void BufferWriter::OverwriteUInt32At(uint32_t value, size_t index)
{
	if (index + sizeof(uint32_t) > m_buffer->size()) return;

	uint8_t const* data = reinterpret_cast<uint8_t const*>(&value);
	std::copy(data, data + sizeof(uint32_t), m_buffer->begin() + index);
}

void BufferWriter::SetEndianMode(Endianness endian)
{
	m_endian = endian;
}

Endianness BufferWriter::GetNativeEndianness()
{
	uint32_t value = 1;
	return (*reinterpret_cast<uint8_t*>(&value) == 1) ? Endianness::LITTLE : Endianness::BIG;
}

