#include "Engine/Core/Vertex_PCU.hpp"

 Vertex_PCU::Vertex_PCU(float x, float y, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	 //Structers that needs constructer functions
	: m_position(x, y, 0.f)
 	, m_color(r, g, b, a)
 	, m_uvTexCoords(0.f, 0.f)
 {
 }

 Vertex_PCU::Vertex_PCU(Vec3 positon, Rgba8 color, Vec2 uvTexCoords)
	 : m_position (positon)
	 , m_color (color)
	 , m_uvTexCoords (uvTexCoords)
 {
 }
