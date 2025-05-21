#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/ObjLoader.hpp"

CPUMesh::CPUMesh()
{
}

CPUMesh::CPUMesh(const std::string& objFilename, const Mat44& transform)
{
	ObjLoader::Load(objFilename, m_vertexes, m_indexes, m_hasNormals, m_hasUVs, transform);
}

CPUMesh::~CPUMesh()
{
	m_vertexes.clear();
	m_indexes.clear();
}

void CPUMesh::Load(const std::string& objFilename, const Mat44& transform)
{
	ObjLoader::Load(objFilename, m_vertexes, m_indexes, m_hasNormals, m_hasUVs, transform);
}

std::vector<Vertex_PCUTBN> CPUMesh::GetVertexCount() const
{
	return m_vertexes;
}

std::vector<unsigned int> CPUMesh::GetIndexCount() const
{
	return m_indexes;
}
