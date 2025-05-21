#include "Engine/Renderer/ObjLoader.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <algorithm>
#include <unordered_map>

struct Triangle
{
	Triangle(int v0, int v1, int v2, int t0, int t1, int t2, int n0, int n1, int n2, const Rgba8& color = Rgba8::WHITE)
		: m_color(color)
	{
		m_vertexPositionIndex[0] = v0;
		m_vertexPositionIndex[1] = v1;
		m_vertexPositionIndex[2] = v2;

		m_vertexTextureCoordinateIndex[0] = t0;
		m_vertexTextureCoordinateIndex[1] = t1;
		m_vertexTextureCoordinateIndex[2] = t2;

		m_vertexNormalIndex[0] = n0;
		m_vertexNormalIndex[1] = n1;
		m_vertexNormalIndex[2] = n2;
	}
	int m_vertexPositionIndex[3]{-1, -1, -1};
	Rgba8 m_color;
	int m_vertexTextureCoordinateIndex[3]{-1, -1, -1};
	int m_vertexNormalIndex[3]{-1, -1, -1};
};

std::vector<Triangle> triangles;

struct VertexHash {
	size_t operator()(const Vertex& vertex) const {
		size_t h1 = std::hash<int>{}(vertex.m_v);
		size_t h2 = std::hash<int>{}(vertex.m_vt);
		size_t h3 = std::hash<int>{}(vertex.m_vn);

		// Combine hashes
		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

struct VertexEqual {
	bool operator()(const Vertex& lhs, const Vertex& rhs) const {
		return lhs.m_v == rhs.m_v && lhs.m_vt == rhs.m_vt && lhs.m_vn == rhs.m_vn;
	}
};

bool ObjLoader::Load(const std::string& fileName, std::vector<Vertex_PCUTBN>& outVertexes, std::vector<unsigned int>& outIndexes, bool& outHasNormals, bool& outHasUVs, const Mat44& transform /*= Mat44()*/)
{
	std::vector<Vec3> posArr;
	std::vector<Vec2> uvArr;
	std::vector<Vec3> normArr;
	std::vector<Face> faceArr;
	std::unordered_map<std::string, Rgba8> materialLibrary;

	std::string strr;
	std::string filePath = fileName;
	std::string objDirectory = filePath.substr(0, filePath.find_last_of("\\/"));
	FileUtils file;
	file.FileReadToString(strr, filePath);

	Strings st;

	std::string currentFaceMaterial;

	int numLines = SplitStringOnDelimiter(st, strr, "\r\n");
	if (numLines == 1) // Meaning no valid split occurred (there's only one line or something went wrong in the splitting process)
	{
		int numSubStrings = SplitStringOnDelimiter(st, strr, "\n");
		if (numSubStrings <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < st.size(); i++)
		{
			if (st[i].empty() || st[i][0] == '#')
			{
				continue;
			}

			// Parse materials first
			ParsingMaterialFile(st[i], materialLibrary, objDirectory);

			// Parse vertex positions, UVs, and normals
			ParsingVertexPos(st[i], posArr);
			ParsingVertexUVs(st[i], uvArr, outHasUVs);
			ParsingVertexNormals(st[i], normArr, outHasNormals);

			// Check if a material is being used for faces
			if (st[i].substr(0, 6) == "usemtl") 
			{
				currentFaceMaterial = SplitStringOnDelimiter(st[i], ' ')[1];
			}

			// Parse faces and apply the correct material
			if (currentFaceMaterial.size() > 0) 
			{
				ParsingFaces(st[i], faceArr, materialLibrary[currentFaceMaterial]);
			}
			else 
			{
				ParsingFaces(st[i], faceArr);
			}
		}
	}
	else
	{
		for (int i = 0; i < st.size(); i++)
		{
			if (st[i].empty() || st[i][0] == '#')
			{
				continue;
			}

			// Parse materials first
			ParsingMaterialFile(st[i], materialLibrary, objDirectory);

			// Parse vertex positions, UVs, and normals
			ParsingVertexPos(st[i], posArr);
			ParsingVertexUVs(st[i], uvArr, outHasUVs);
			ParsingVertexNormals(st[i], normArr, outHasNormals);

			// Check if a material is being used for faces
			if (st[i].substr(0, 6) == "usemtl") 
			{
				currentFaceMaterial = SplitStringOnDelimiter(st[i], ' ')[1];
			}

			// Parse faces and apply the correct material
			if (currentFaceMaterial.size() > 0) 
			{
				ParsingFaces(st[i], faceArr, materialLibrary[currentFaceMaterial]);
			}
			else 
			{
				ParsingFaces(st[i], faceArr);
			}
		}
	}

	GenerateVerticesAndIndices(faceArr, posArr, uvArr, normArr, outVertexes, outIndexes);

	TransformVertexArray3D(outVertexes, transform, outHasNormals);

	return true;
}

void ObjLoader::ParsingVertexPos(std::string line, std::vector<Vec3>& positions)
{
	if (line[0] == 'v' && line[1] == ' ')
	{
		Strings positionStrings = SplitStringOnDelimiter(line, ' ');

		float x = (float)std::atof(positionStrings[1].c_str());
		float y = (float)std::atof(positionStrings[2].c_str());
		float z = (float)std::atof(positionStrings[3].c_str());

		positions.emplace_back(Vec3(x, y, z));
	}
}

void ObjLoader::ParsingVertexUVs(std::string line, std::vector<Vec2>& uvs, bool& outHasUVs)
{
	if (line[0] == 'v' && line[1] == 't')
	{
		outHasUVs = true;

		Strings uv = SplitStringOnDelimiter(line, ' ');

		float u = (float)std::atof(uv[1].c_str());
		float v = (float)std::atof(uv[2].c_str());

		uvs.emplace_back(Vec2(u, v));
	}
}

void ObjLoader::ParsingVertexNormals(std::string line, std::vector<Vec3>& normals, bool& outHasNormals)
{
	if (line[0] == 'v' && line[1] == 'n')
	{
		outHasNormals = true;

		Strings normal = SplitStringOnDelimiter(line, ' ');

		float normX = (float)std::atof(normal[1].c_str());
		float normY = (float)std::atof(normal[2].c_str());
		float normZ = (float)std::atof(normal[3].c_str());

		normals.emplace_back(Vec3(normX, normY, normZ));
	}
}

void ObjLoader::ParsingFaces(std::string line, std::vector<Face>& outFaces, Rgba8 faceColor)
{
	if (line[0] == 'f')
	{
		Strings faces = SplitStringOnDelimiter(line, ' ');
		Face face;

		// Iterate over each face vertex definition (skipping the 'f' at index 0)
		for (int j = 1; j < faces.size(); j++)
		{
			Strings faceInfo;
			Vertex vert;

			// Check if it's a `v//vn` format (i.e. vertex and normal but no texture coordinate)
			faceInfo = SplitStringOnDelimiter(faces[j], "//");
			if (faceInfo.size() == 2)
			{
				vert.m_v = std::atoi(faceInfo[0].c_str());
				vert.m_vt = -1; // No texture coordinate
				vert.m_vn = std::atoi(faceInfo[1].c_str());
				face.m_vertices.emplace_back(vert);
				continue;
			}

			// Otherwise, assume it's a `v/vt/vn` or `v/vt` or `v` format (split on '/')
			faceInfo = SplitStringOnDelimiter(faces[j], '/');
			if (faceInfo.size() == 1)
			{
				vert.m_v = std::atoi(faceInfo[0].c_str());
				vert.m_vt = -1; // No texture coordinate
				vert.m_vn = -1; // No normal
			}
			else if (faceInfo.size() == 2)
			{
				vert.m_v = std::atoi(faceInfo[0].c_str());
				vert.m_vt = std::atoi(faceInfo[1].c_str());
				vert.m_vn = -1; // No normal
			}
			else if (faceInfo.size() == 3)
			{
				vert.m_v = std::atoi(faceInfo[0].c_str());
				vert.m_vt = std::atoi(faceInfo[1].c_str());
				vert.m_vn = std::atoi(faceInfo[2].c_str());
			}
			face.m_vertices.emplace_back(vert);
		}
		face.m_color = faceColor;
		outFaces.emplace_back(face);
	}
}

void ObjLoader::ParsingMaterialFile(std::string line, std::unordered_map<std::string, Rgba8>& outMaterialLib, const std::string& fileDirectory)
{
	std::string matrialFile = line.substr(0, 6);

	if (matrialFile == "mtllib")
	{
		Strings materialInfo;

		std::string sug = line;

		materialInfo = SplitStringOnDelimiter(sug, ' ');

		Strings materialStrings;

		std::string materialString;

		std::string materialFilePath = fileDirectory + "/" + materialInfo[1];

		FileUtils file;
		file.FileReadToString(materialString, materialFilePath);

		std::replace(materialString.begin(), materialString.end(), '\r', '\n');
		
		int materialSplit = SplitStringOnDelimiter(materialStrings, materialString, "\n");
		
		std::string materialName;

		if (materialSplit == 1)
		{
			for (int j = 0; j < materialStrings.size(); j++)
			{
				if (materialStrings[j].substr(0, 6) == "newmtl")
				{
					Strings newMaterial;

					newMaterial = SplitStringOnDelimiter(materialStrings[j], ' ');

					materialName = newMaterial[1];
				}

				if (materialStrings[j].substr(0, 2) == "Kd")
				{
					Strings materialDiffuseColor;

					materialDiffuseColor = SplitStringOnDelimiter(materialStrings[j], ' ');

					Rgba8 color;

					float r = (float)std::atof(materialDiffuseColor[1].c_str());
					float g = (float)std::atof(materialDiffuseColor[2].c_str());
					float b = (float)std::atof(materialDiffuseColor[3].c_str());

					color.r = unsigned char(r * 255);
					color.g = unsigned char(g * 255);
					color.b = unsigned char(b * 255);

					outMaterialLib[materialName] = color;
				}
			}
		}
		else
		{
			for (int j = 0; j < materialStrings.size(); j++)
			{
				if (materialStrings[j].substr(0, 6) == "newmtl")
				{
					Strings newMaterial;

					newMaterial = SplitStringOnDelimiter(materialStrings[j], ' ');

					materialName = newMaterial[1];
				}

				if (materialStrings[j].substr(0, 2) == "Kd")
				{
					Strings materialDiffuseColor;

					materialDiffuseColor = SplitStringOnDelimiter(materialStrings[j], ' ');

					Rgba8 color;

					float r = (float)std::atof(materialDiffuseColor[1].c_str());
					float g = (float)std::atof(materialDiffuseColor[2].c_str());
					float b = (float)std::atof(materialDiffuseColor[3].c_str());

					color.r = unsigned char(r * 255);
					color.g = unsigned char(g * 255);
					color.b = unsigned char(b * 255);

					outMaterialLib[materialName] = color;
				}
			}
		}
	}
}

void ObjLoader::GenerateVerticesAndIndices(std::vector<Face> inFaces, std::vector<Vec3> positions, std::vector<Vec2> uvs, std::vector<Vec3> normals, std::vector<Vertex_PCUTBN>& outVertices, std::vector<unsigned int>& outIndices)
{
	int index = 0;

	for (int i = 0; i < inFaces.size(); i++)
	{
		Rgba8 faceColor = inFaces[i].m_color;

		for (int j = 0; j < inFaces[i].m_vertices.size(); j++)
		{
			Vertex faceVertex = inFaces[i].m_vertices[j];

			Vertex_PCUTBN vertex;
			vertex.m_position = positions[faceVertex.m_v - 1];
			vertex.m_color = faceColor;

			if (uvs.size() == 0)
			{
				vertex.m_uvTexCoords = Vec2::ZERO;
			}
			else
			{
				vertex.m_uvTexCoords = uvs[faceVertex.m_vt - 1];
			}

			if (normals.size() == 0)
			{
				vertex.m_normal = Vec3::ZERO;
			}
			else
			{
				vertex.m_normal = normals[faceVertex.m_vn - 1];
			}

			outVertices.emplace_back(vertex);
		}

		for (int j = 1; j < inFaces[i].m_vertices.size() - 1; j++)
		{
			unsigned int i0 = index + 0;
			unsigned int i1 = index + j;
			unsigned int i2 = index + j + 1;

			outIndices.emplace_back(i0);
			outIndices.emplace_back(i1);
			outIndices.emplace_back(i2);
		}

		index += (int)inFaces[i].m_vertices.size();
	}
}