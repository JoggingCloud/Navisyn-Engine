#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

 struct Vertex_PCU
{
public:
	Vertex_PCU() = default;
	explicit Vertex_PCU(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	Vertex_PCU(Vec3 positon, Rgba8 color, Vec2 uvTexCoords);

	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;
};