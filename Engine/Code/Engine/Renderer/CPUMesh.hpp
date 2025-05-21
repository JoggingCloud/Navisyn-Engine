#pragma once
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>
#include <string>

class CPUMesh
{
public:
	CPUMesh();
	CPUMesh(const std::string& objFilename, const Mat44& transform);
	virtual ~CPUMesh();

	void Load(const std::string& objFilename, const Mat44& transform);
	std::vector<Vertex_PCUTBN> GetVertexCount() const;
	std::vector<unsigned int> GetIndexCount() const;

public:
	std::vector<unsigned int> m_indexes;
	std::vector<Vertex_PCUTBN> m_vertexes;

	bool m_hasNormals = false;
	bool m_hasUVs = false;
};