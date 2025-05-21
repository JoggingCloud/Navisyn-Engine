#pragma once
#include <string>
#include "Engine/Renderer/Renderer.hpp"

class Shader;

class Material
{
public:
	Material();
	~Material();

	bool Load(const std::string& xmlFilename);

	std::string m_materialName;
	std::string m_shaderName;
	std::string m_vertexTypeName;
	std::string m_diffuseTextureName;
	std::string m_normalTextureName;
	std::string m_specGlossEmitTextureName;

	Shader* m_shader = nullptr;
	VertexType m_vertexType = VertexType::Vertex_PCUTBN;
	Texture* m_diffuseTexture = nullptr;
	Texture* m_normalTexture = nullptr;
	Texture* m_specGlossEmitTexture = nullptr;
	Rgba8 m_color = Rgba8::WHITE;

};