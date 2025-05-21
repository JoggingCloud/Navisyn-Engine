#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"

extern Renderer*	g_theRenderer;

GPUMesh::GPUMesh()
{
}

GPUMesh::GPUMesh(const CPUMesh* cpuMesh)
{
	 UNUSED(cpuMesh);
	//Create(cpuMesh);
}

GPUMesh::~GPUMesh()
{
	if (m_indexBuffer)
	{
		delete m_indexBuffer;
		m_indexBuffer = nullptr;
	}

	if (m_vertexBuffer)
	{
		delete m_vertexBuffer;
		m_vertexBuffer = nullptr;
	}
}

void GPUMesh::Create(const CPUMesh* cpuMesh)
{
	// Create vertex buffer on GPU

	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(cpuMesh->m_vertexes.size());
	g_theRenderer->CopyCPUToGPU(cpuMesh->m_vertexes.data(), cpuMesh->m_vertexes.size() * sizeof(Vertex_PCUTBN), m_vertexBuffer);

	// Create index buffer on GPU
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(cpuMesh->m_indexes.size());
	g_theRenderer->CopyCPUToGPU(cpuMesh->m_indexes.data(), cpuMesh->m_indexes.size() * sizeof(unsigned int), m_indexBuffer);
}

void GPUMesh::Render() const
{
	//g_theRenderer->DrawVertexBufferIndex(m_vertexBuffer, m_indexBuffer, VertexType::Vertex_PCUTBN, static_cast<int>());
}
