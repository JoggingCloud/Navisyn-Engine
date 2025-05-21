#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Mat44.hpp"

struct Vertex
{
	int m_v = -1;
	int m_vt = -1;
	int m_vn = -1;
};

struct Face
{
	std::vector<Vertex> m_vertices;
	Rgba8 m_color;
};

class ObjLoader
{
public:
	ObjLoader() = default;
	~ObjLoader() = default;

	static bool Load(const std::string& fileName, std::vector<Vertex_PCUTBN>& outVertexes, std::vector<unsigned int>& outIndexes, bool& outHasNormals, bool& outHasUVs, const Mat44& transform = Mat44());
	static void ParsingVertexPos(std::string line, std::vector<Vec3>& positions);
	static void ParsingVertexUVs(std::string line, std::vector<Vec2>& uvs, bool& outHasUVs);
	static void ParsingVertexNormals(std::string line, std::vector<Vec3>& normals, bool& outHasNormals);
	static void ParsingFaces(std::string line, std::vector<Face>& outFaces, Rgba8 faceColor = Rgba8::WHITE);
	static void ParsingMaterialFile(std::string line, std::unordered_map<std::string, Rgba8>& outMaterialLib, const std::string& fileDirectory);
	static void GenerateVerticesAndIndices(std::vector<Face> inFaces, std::vector<Vec3> positions, std::vector<Vec2> uvs, std::vector<Vec3> normals, std::vector<Vertex_PCUTBN>& outVertices, std::vector<unsigned int>& outIndices);
};