#pragma once
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

class GPUMesh
{
public:
	GPUMesh();
	GPUMesh(const CPUMesh* cpuMesh);
	virtual ~GPUMesh();

	void Create(const CPUMesh* cpuMesh);
	void Render() const;

public:
	IndexBuffer* m_indexBuffer = nullptr;
	VertexBuffer* m_vertexBuffer = nullptr;
};