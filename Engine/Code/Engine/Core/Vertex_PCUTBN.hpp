#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"

struct Vertex_PCUTBN
{
	Vertex_PCUTBN() = default;
	Vertex_PCUTBN(Vec3 const& position, Rgba8 const& color = Rgba8(225, 255, 255,255), Vec2 const& uvTexCoords = Vec2(0.f, 0.f), Vec3 const& tangent = Vec3(0.f, 0.f, 0.f), Vec3 const& bitangent = Vec3(0.f, 0.f, 0.f), Vec3 const& normal = Vec3(0.f, 0.f, 0.f));
	Vertex_PCUTBN(float px, float py, float pz, unsigned char r, unsigned char g, unsigned char b, unsigned char a, float u, float v, float tx, float ty, float tz, float bx, float by, float bz, float nx, float ny, float nz);
	
	Vec3 m_position = Vec3::ZERO;
	Rgba8 m_color = Rgba8::WHITE;
	Vec2 m_uvTexCoords = Vec2::ZERO;
	Vec3 m_tangent = Vec3::ZERO;
	Vec3 m_bitangent = Vec3::ZERO;
	Vec3 m_normal = Vec3::ZERO;

	bool operator==(const Vertex_PCUTBN& other) const
	{
		return (m_position == other.m_position) &&
			   (m_color == other.m_color) &&
			   (m_uvTexCoords == other.m_uvTexCoords) &&
			   (m_tangent == other.m_tangent) &&
			   (m_bitangent == other.m_bitangent) &&
			   (m_normal == other.m_normal);
	}
};

// Hash function for Vertex to use in unordered_map or unordered_set
namespace std
{
	template <>
	struct hash<Vertex_PCUTBN>
	{
		size_t operator()(const Vertex_PCUTBN& vert) const
		{
			// Hash individual components
			size_t h1 = hash<Vec3>()(vert.m_position);
			size_t h2 = hash<Vec3>()(vert.m_normal);
			size_t h3 = hash<Vec3>()(vert.m_tangent);
			size_t h4 = hash<Vec3>()(vert.m_bitangent);
			size_t h5 = hash<Vec2>()(vert.m_uvTexCoords);
			size_t h6 = hash<Rgba8>()(vert.m_color);

			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5); // Combine hashes
		}
	};
}