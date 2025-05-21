#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Shader.hpp"

extern Renderer* g_theRenderer;

Material::Material()
{
}

Material::~Material()
{
	if (m_shader)
	{
		delete m_shader;
		m_shader = nullptr;
	}
}

bool Material::Load(const std::string& xmlFilename)
{
	tinyxml2::XMLDocument document;
	if (document.LoadFile(xmlFilename.c_str()) == tinyxml2::XML_SUCCESS)
	{
		// Get the root element 
		tinyxml2::XMLElement* rootElement = document.RootElement();
		GUARANTEE_OR_DIE(rootElement, "XML couldn't be loaded");

		m_materialName = ParseXmlAttribute(*rootElement, "name", std::string());
		m_shaderName = ParseXmlAttribute(*rootElement, "shader", std::string());
		m_vertexTypeName = ParseXmlAttribute(*rootElement, "vertexType", std::string());
		m_diffuseTextureName = ParseXmlAttribute(*rootElement, "diffuseTexture", std::string());
		m_normalTextureName = ParseXmlAttribute(*rootElement, "normalTexture", std::string());
		m_specGlossEmitTextureName = ParseXmlAttribute(*rootElement, "specGlossEmitTexture", std::string());
		m_color = ParseXmlAttribute(*rootElement, "color", Rgba8(255, 255, 255));

		if (m_vertexTypeName == "Vertex_PCUTBN")
		{
			m_vertexType = VertexType::Vertex_PCUTBN;
		}
		else if (m_vertexTypeName == "Vertex_PCU")
		{
			m_vertexType = VertexType::Vertex_PCU;
		}

		m_shader = g_theRenderer->CreateOrGetShader(m_shaderName.c_str(), m_vertexType);
		m_diffuseTexture = g_theRenderer->CreateOrGetTextureFromFile(m_diffuseTextureName.c_str());
		m_normalTexture = g_theRenderer->CreateOrGetTextureFromFile(m_normalTextureName.c_str());
		m_specGlossEmitTexture = g_theRenderer->CreateOrGetTextureFromFile(m_specGlossEmitTextureName.c_str());

		return true;
	}
	return false;
}

