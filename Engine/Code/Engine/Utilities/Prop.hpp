#pragma once
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include <vector>

struct RaycastResult3D;

class Prop
{
public:
	Prop(Vec3 const& startPos, EulerAngles const& startOrientation);
	Prop() = default;
	~Prop();

	void CreateGrid(const int& gridSize = 100, const int& thickLineSpacing = 5, const float& lineWidth = 0.1f);
	void CreateCube(Vec3 const& halfSize);
	void CreateCylinder(Vec3 const& startPos, Vec3 const& endPos, float radius, int slices);
	void CreatePrism(Vec3 const& centerPosition, float length, float width, float height);
	void CreatePyramid(Vec3 const& centerPosition, float length, float width, float height);
	
	void AddLine(const Vec3& bottomLeft, const Vec3& topRight, const Rgba8& color, float thickness);
	void AddZArrow(const Vec3& start, const Vec3& end, const float& cylinderRadius = 0.05f, const float& coneRadius = 0.1f, const int& cylinderSlices = 32, const int& coneSlices = 32);
	void AddXArrow(const Vec3& start, const Vec3& end, const float& cylinderRadius = 0.05f, const float& coneRadius = 0.1f, const int& cylinderSlices = 32, const int& coneSlices = 32);
	void AddYArrow(const Vec3& start, const Vec3& end, const float& cylinderRadius = 0.05f, const float& coneRadius = 0.1f, const int& cylinderSlices = 32, const int& coneSlices = 32);
	
	void Render() const;
	void RenderPropPCUTBN() const;
	void RenderPropPCU() const;
	void RenderPropVBO() const;
	void Update();

	Mat44 GetModelMatrix() const;	

	void CreateIndexBuffer();
	void CreateVertexPCUBuffer();
	void CreateBuffers();

public:
	bool IsTriangleBlockedByCylinder3D(Vec3 const& v0, Vec3 const& v1, Vec3 const& v2, float radius) const;
	bool IsTriangleBlockedByAABB3D(Vec3 const& v0, Vec3 const& v1, Vec3 const& v2, float radius) const;
	bool IsEdgeIntersectingAABB3D(Vec3 const& v0, Vec3 const& v1) const;
	bool IsEdgeIntersectingCylinder(Vec3 const& v0, Vec3 const& v1) const;
	bool IsPointInside(Vec3 const& point) const;

public:
	Material* m_material = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;

public:
	std::vector<Vertex_PCU> m_vertexes;
	std::vector<Vertex_PCUTBN> m_tbnVertexes;
	std::vector<unsigned int> m_indexes;
	Texture* m_texture = nullptr;

public:
	AABB3 m_box;
	Vec3 m_cylinderCenter = Vec3::ZERO;
	Vec3 m_cylinderStartPos = Vec3::ZERO;
	Vec3 m_cylinderEndPos = Vec3::ZERO;
	float m_cylinderRadius = 0.f;

public:
	Rgba8 m_color = Rgba8::WHITE;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
};