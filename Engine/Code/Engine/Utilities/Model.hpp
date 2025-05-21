#pragma once
#include <vector>
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Material;

class Model
{
public:
	Model() = default;
	~Model();

	bool Load(const std::string& filename);
	bool LoadXml(const std::string& filename);
	bool LoadObj(const std::string& filename, const Mat44& transform = Mat44());

	void Update();
	void Render() const;
	void RenderLoadedModel() const;

public:
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;

public:
	Mat44 GetModelMatrix() const;

	std::string m_objFilename;

	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh* m_gpuMesh = nullptr;
	Material* m_material = nullptr;
	Rgba8 m_color = Rgba8::WHITE;

	std::vector<Vertex_PCU> m_debugVertexes;
	VertexBuffer* m_debugVertexBuffer = nullptr;
};