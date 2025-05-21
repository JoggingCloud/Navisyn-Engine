#include "Engine/Utilities/Prop.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include <filesystem>
#include <string>

extern Renderer* g_theRenderer;

Prop::Prop(Vec3 const& startPos, EulerAngles const& startOrientation)
	: m_position(startPos), m_orientation(startOrientation)
{
}

Prop::~Prop()
{
	SafeDelete(m_indexBuffer);
	SafeDelete(m_vertexBuffer);
	SafeDelete(m_material);

	m_tbnVertexes.clear();
	m_vertexes.clear();
	m_indexes.clear();
}

void Prop::CreateGrid(const int& gridSize /*= 100*/, const int& thickLineSpacing /*= 5*/, const float& lineWidth /*= 0.1f*/)
{
	int halfGridSize = gridSize / 2;

	// Iterate over Y-Axis lines 
	for (int y = -halfGridSize; y <= halfGridSize; y++)
	{
		Rgba8 color = Rgba8::LIGHT_GRAY;
		if (y % thickLineSpacing == 0)
		{
			// Thicker and brighter lines
			if (y == 0 || y == halfGridSize || y == -halfGridSize)
			{
				color = Rgba8::GREEN;
			}
			else
			{
				color = Rgba8::DARK_GREEN;
			}

			AddLine(Vec3(static_cast<float>(y), static_cast<float>(-halfGridSize), 0.f), Vec3(static_cast<float>(y), static_cast<float>(halfGridSize), 0.f), color, lineWidth);
		}
		else
		{
			// Regular lines
			AddLine(Vec3(static_cast<float>(y), static_cast<float>(-halfGridSize), 0.f), Vec3(static_cast<float>(y), static_cast<float>(halfGridSize), 0.f), Rgba8::LIGHT_GRAY, lineWidth * 0.5f);
		}
	}

	// Iterate over X-Axis lines
	for (int x = -halfGridSize; x <= halfGridSize; x++)
	{
		Rgba8 color = Rgba8::LIGHT_GRAY;
		if (x % thickLineSpacing == 0)
		{
			// Thicker and brighter lines
			if (x == 0 || x == halfGridSize || x == -halfGridSize)
			{
				color = Rgba8::RED;
			}
			else
			{
				color = Rgba8::DARK_RED;
			}

			AddLine(Vec3(static_cast<float>(-halfGridSize), static_cast<float>(x), 0.f), Vec3(static_cast<float>(halfGridSize), static_cast<float>(x), 0.f), color, lineWidth);
		}
		else
		{
			// Regular lines
			AddLine(Vec3(static_cast<float>(-halfGridSize), static_cast<float>(x), 0.f), Vec3(static_cast<float>(halfGridSize), static_cast<float>(x), 0.f), Rgba8::LIGHT_GRAY, lineWidth * 0.5f);
		}
	}
}

void Prop::AddLine(const Vec3& bottomLeft, const Vec3& topRight, const Rgba8& color, float thickness)
{
	Vec3 center = (bottomLeft + topRight) * 0.5f;
	Vec3 scale = (topRight - bottomLeft);

	// Adjust scale to account for thickness along the x-axis and y-axis
	if (scale.x == 0.f)
	{
		scale.x = thickness;
	}
	else if (scale.y == 0.f)
	{
		scale.y = thickness;
	}
	scale.z = thickness;

	// Add the line cube with adjusted scale
	AddVertsForAABB3D(m_vertexes, AABB3(center + scale * 0.5f, center - scale * 0.5f), color, AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f)));
}

void Prop::CreateCube(Vec3 const& halfSize)
{
	Vec3 mins = m_position - halfSize;
	Vec3 maxs = m_position + halfSize;

	m_box = AABB3(mins, maxs);

	AddVertsForAABB3D(m_tbnVertexes, m_indexes, m_box, m_color);
}

void Prop::CreateCylinder(Vec3 const& startPos, Vec3 const& endPos, float radius, int slices)
{
	m_cylinderStartPos = startPos;
	m_cylinderEndPos = endPos;
	m_cylinderRadius = radius;

	m_cylinderCenter = (startPos + endPos) * 0.5f;

	AddVertsForZCylinder3D(m_tbnVertexes, m_indexes, m_cylinderStartPos, m_cylinderEndPos, m_cylinderRadius, slices, m_color);
}

void Prop::CreatePrism(Vec3 const& centerPosition, float length, float width, float height)
{
	// Compute the half-extents
	Vec3 halfSize = Vec3(length * 0.5f, width * 0.5f, height * 0.5f);

	// Calculate the bounds
	Vec3 mins = centerPosition - halfSize;
	Vec3 maxs = centerPosition + halfSize;

	// Store bounds as AABB3 for intersection checks
	m_box = AABB3(mins, maxs);

	AddVertsForPrism(m_tbnVertexes, m_indexes, centerPosition, length, width, height, m_color);
}

void Prop::CreatePyramid(Vec3 const& centerPosition, float length, float width, float height)
{
    // Compute the half-extents for the base
    Vec3 halfBaseSize = Vec3(length * 0.5f, width * 0.5f, 0.f);

    // Calculate the base bounds
    Vec3 baseMins = centerPosition - halfBaseSize;
    Vec3 baseMaxs = centerPosition + halfBaseSize;

    // Apex is directly above the center of the base
    Vec3 apex = centerPosition + Vec3(0.f, 0.f, height);

    // Store bounds as AABB3 for intersection checks
    m_box = AABB3(baseMins, baseMaxs + Vec3(0.f, 0.f, height));

	AddVertsForPyramid(m_tbnVertexes, m_indexes, centerPosition, length, width, height, m_color);
}

void Prop::AddZArrow(const Vec3& start, const Vec3& end, const float& cylinderRadius, const float& coneRadius, const int& cylinderSlices, const int& coneSlices)
{
	AddVertsForArrow3D(m_vertexes, start, end, 0.75f, cylinderRadius, coneRadius, Rgba8::BLUE, Rgba8::BLUE, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), cylinderSlices, coneSlices);
}

void Prop::AddXArrow(const Vec3& start, const Vec3& end, const float& cylinderRadius, const float& coneRadius, const int& cylinderSlices, const int& coneSlices)
{
	AddVertsForArrow3D(m_vertexes, start, end, 0.75f, cylinderRadius, coneRadius, Rgba8::RED, Rgba8::RED, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), cylinderSlices, coneSlices);
}

void Prop::AddYArrow(const Vec3& start, const Vec3& end, const float& cylinderRadius, const float& coneRadius, const int& cylinderSlices, const int& coneSlices)
{
	AddVertsForArrow3D(m_vertexes, start, end, 0.75f, cylinderRadius, coneRadius, Rgba8::GREEN, Rgba8::GREEN, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), cylinderSlices, coneSlices);
}

void Prop::Render() const
{
	RenderPropPCUTBN();
	RenderPropPCU();
}

void Prop::RenderPropPCUTBN() const
{
	if (!m_tbnVertexes.empty() && !m_indexes.empty())
	{
		g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants(Mat44(), m_color);
		g_theRenderer->BindTexture(0, nullptr);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->DrawVertexBufferIndex(m_vertexBuffer, m_indexBuffer, VertexType::Vertex_PCUTBN, static_cast<int>(m_indexes.size()));
	}
}

void Prop::RenderPropPCU() const
{
	if(!m_vertexes.empty())
	{
		g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants(Mat44(), m_color);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(0, nullptr);
		g_theRenderer->DrawVertexArray(static_cast<int>(m_vertexes.size()), m_vertexes.data());
	}
}

void Prop::RenderPropVBO() const
{
	if (!m_vertexes.empty() && m_vertexBuffer)
	{
		g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetModelConstants(Mat44(), m_color);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(0, nullptr);
		g_theRenderer->DrawVertexBuffer(m_vertexBuffer, VertexType::Vertex_PCU, static_cast<int>(m_vertexes.size()));
	}
}

void Prop::Update()
{
}

Mat44 Prop::GetModelMatrix() const
{
	Mat44 translation = Mat44::CreateTranslation3D(m_position);
	Mat44 orientation = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();

	translation.Append(orientation);
	return translation;
}

void Prop::CreateIndexBuffer()
{
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indexes.size());
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), m_indexes.size() * sizeof(unsigned int), m_indexBuffer);
}

void Prop::CreateVertexPCUBuffer()
{
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(m_vertexes.size());
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), m_vertexes.size() * sizeof(Vertex_PCU), m_vertexBuffer);
}

void Prop::CreateBuffers()
{
	// Create vertex buffer on GPU
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(m_tbnVertexes.size());
	g_theRenderer->CopyCPUToGPU(m_tbnVertexes.data(), m_tbnVertexes.size() * sizeof(Vertex_PCUTBN), m_vertexBuffer);

	// Create index buffer on GPU
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indexes.size());
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), m_indexes.size() * sizeof(unsigned int), m_indexBuffer);
}

bool Prop::IsTriangleBlockedByCylinder3D(Vec3 const& v0, Vec3 const& v1, Vec3 const& v2, float radius) const
{
	float expandedRadius = m_cylinderRadius + radius;

	return IsPointInsideZCylinder(v0, Vec2(m_cylinderCenter.x, m_cylinderCenter.y), FloatRange(m_cylinderEndPos.z, m_cylinderStartPos.z), expandedRadius) ||
		   IsPointInsideZCylinder(v1, Vec2(m_cylinderCenter.x, m_cylinderCenter.y), FloatRange(m_cylinderEndPos.z, m_cylinderStartPos.z), expandedRadius) ||
		   IsPointInsideZCylinder(v2, Vec2(m_cylinderCenter.x, m_cylinderCenter.y), FloatRange(m_cylinderEndPos.z, m_cylinderStartPos.z), expandedRadius) ||
		   IsEdgeIntersectingCylinder(v0, v1) ||
		   IsEdgeIntersectingCylinder(v1, v2) ||
		   IsEdgeIntersectingCylinder(v2, v0);
}

bool Prop::IsTriangleBlockedByAABB3D(Vec3 const& v0, Vec3 const& v1, Vec3 const& v2, float radius) const
{
	AABB3 expandedBox = m_box;
	expandedBox.m_mins -= Vec3(radius, radius, radius);
	expandedBox.m_maxs += Vec3(radius, radius, radius);

	return expandedBox.IsPointInside(v0) || expandedBox.IsPointInside(v1) || expandedBox.IsPointInside(v2) ||
		IsEdgeIntersectingAABB3D(v0, v1) || IsEdgeIntersectingAABB3D(v1, v2) ||
		IsEdgeIntersectingAABB3D(v2, v0);
}

bool Prop::IsEdgeIntersectingAABB3D(Vec3 const& v0, Vec3 const& v1) const
{
	Vec3 edgeDirection = (v1 - v0).GetNormalized();
	float edgeLength = GetDistance3D(v0, v1);

	RaycastResult3D result = RaycastVsAABB3D(v0, edgeDirection, edgeLength, m_box);
	return result.m_didImpact;
}

bool Prop::IsEdgeIntersectingCylinder(Vec3 const& v0, Vec3 const& v1) const
{
	Vec3 edgeDirection = (v1 - v0).GetNormalized();
	float edgeLength = GetDistance3D(v0, v1);

	RaycastResult3D result = RaycastVsZCylinder(v0, edgeDirection, edgeLength, Vec2(m_cylinderCenter.x, m_cylinderCenter.y), FloatRange(m_cylinderEndPos.z, m_cylinderStartPos.z), m_cylinderRadius);
	return result.m_didImpact;
}

bool Prop::IsPointInside(Vec3 const& point) const
{
	return m_box.IsPointInside(point);
}
