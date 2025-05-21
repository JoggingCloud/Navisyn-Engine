#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Mat44.hpp"

extern Renderer* g_theRenderer;

void DebugDrawLine(Vec2 startPoint, Vec2 endPoint, float thickness, Rgba8 color)
{
	constexpr int verts = 3;
	constexpr int numLineVert = 2 * verts;
	Vec2 direction = (endPoint - startPoint).GetNormalized();
	direction.SetLength(thickness * 0.5f);
	Vec2 left = direction.GetRotated90Degrees();

	Vec2 topLeft = (endPoint + direction) + left;
	Vec2 topRight = (endPoint + direction) - left;

	Vec2 bottomLeft = (startPoint - direction) + left;
	Vec2 bottomRight = (startPoint - direction) - left;

	Vertex_PCU vertices[numLineVert];
	vertices[0] = Vertex_PCU(topRight.x, topRight.y, color.r, color.g, color.b, color.a);
	vertices[1] = Vertex_PCU(topLeft.x, topLeft.y, color.r, color.g, color.b, color.a);

	vertices[2] = Vertex_PCU(bottomLeft.x, bottomLeft.y, color.r, color.g, color.b, color.a);
	vertices[3] = Vertex_PCU(bottomLeft.x, bottomLeft.y, color.r, color.g, color.b, color.a);

	vertices[4] = Vertex_PCU(bottomRight.x, bottomRight.y, color.r, color.g, color.b, color.a);
	vertices[5] = Vertex_PCU(topRight.x, topRight.y, color.r, color.g, color.b, color.a);

	Mat44 modelMatrix = Mat44();

	g_theRenderer->SetModelConstants(modelMatrix, Rgba8::WHITE);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray(numLineVert, vertices);
}

void DebugDraw2DRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;

	constexpr int NUM_SIDES = 64;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);

	Vertex_PCU verts[NUM_VERTS];
	for (int sideNum = 0; sideNum < NUM_SIDES; sideNum++)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * (sideNum + 1);

		float cosStart = CosDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);

		float sinStart = SinDegrees(startDegrees);
		float sinEnd = SinDegrees(endDegrees);

		// Computing the inner and out length of the triangles 
		Vec3 innerStartPos = Vec3(center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.f);
		Vec3 innerEndPos = Vec3(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);

		Vec3 outerStartPos = Vec3(center.x + outerRadius * cosStart, center.y + outerRadius * sinStart, 0.f);
		Vec3 outerEndPos = Vec3(center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f);

		// Trapezoid is made of two triangles; ABC & DEF
		// A is innner end; B is inner Start; C is outer start
		// D is inner end; E is out start; F is outer end 
		int vertIndexA = 6 * sideNum;
		int vertIndexB = vertIndexA + 1;
		int vertIndexC = vertIndexB + 1;
		int	vertIndexD = vertIndexC + 1;
		int	vertIndexE = vertIndexD + 1;
		int	vertIndexF = vertIndexE + 1;

		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexC].m_position = outerStartPos;
		verts[vertIndexA].m_color = color;
		verts[vertIndexB].m_color = color;
		verts[vertIndexC].m_color = color;

		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexD].m_color = color;
		verts[vertIndexE].m_color = color;
		verts[vertIndexF].m_color = color;
	}

	Mat44 modelMatrix = Mat44(); 

	g_theRenderer->SetModelConstants(modelMatrix, Rgba8::WHITE);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray(NUM_VERTS, verts);
}
