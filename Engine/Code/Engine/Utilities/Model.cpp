#include "Engine/Utilities/Model.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include <filesystem>
#include <string>

extern Renderer* g_theRenderer;

Model::~Model()
{
	SafeDelete(m_material);
	SafeDelete(m_cpuMesh);
	SafeDelete(m_gpuMesh);
}

bool Model::Load(const std::string& filename)
{
	std::string extension = filename.substr(filename.find_last_of(".") + 1);
	if (extension == "obj")
	{
		LoadObj(filename);
		return true;
	}
	return false;
}

bool Model::LoadXml(const std::string& filename)
{
	tinyxml2::XMLDocument document;
	if (document.LoadFile(filename.c_str()) == tinyxml2::XML_SUCCESS)
	{
		// Get the root element 
		tinyxml2::XMLElement* rootElement = document.RootElement();
		GUARANTEE_OR_DIE(rootElement, "XML couldn't be loaded");

		// Get the parent directory of the xml file
		std::filesystem::path xmlDirectory = std::filesystem::path(filename).parent_path();

		// Get and process the OBJ relative path
		std::string objRelativeFilePath = ParseXmlAttribute(*rootElement, "path", std::string());
		std::filesystem::path objFileName = std::filesystem::path(objRelativeFilePath).filename(); // Extract the filename from the parsed xml
		std::filesystem::path objAbsPath = xmlDirectory / objFileName; // Combine the xml directory and the obj filename

		// Get and process the Material relative path, if present
		std::string materialRelativePath = ParseXmlAttribute(*rootElement, "material", std::string());
		if (!materialRelativePath.empty())
		{
			std::filesystem::path runRootDirectory = xmlDirectory.lexically_normal().parent_path().parent_path();
			std::filesystem::path materialAbsPath = std::filesystem::absolute(runRootDirectory / materialRelativePath);

			m_material = new Material();
			if (!m_material->Load(materialAbsPath.string()))
			{
				GUARANTEE_OR_DIE(false, "Invalid material in XML");
			}
		}
		else
		{
			m_material = nullptr;
		}

		Vec3 xBasis = ParseXmlAttribute(*rootElement->FirstChildElement("Transform"), "x", Vec3(1.f, 0.f, 0.f));
		Vec3 yBasis = ParseXmlAttribute(*rootElement->FirstChildElement("Transform"), "y", Vec3(1.f, 0.f, 0.f));
		Vec3 zBasis = ParseXmlAttribute(*rootElement->FirstChildElement("Transform"), "z", Vec3(1.f, 0.f, 0.f));
		Vec3 translation = ParseXmlAttribute(*rootElement->FirstChildElement("Transform"), "scale", Vec3::ZERO);
		std::string scaleString = ParseXmlAttribute(*rootElement->FirstChildElement("Transform"), "scale", std::string());

		Mat44 transformMatrix;
		transformMatrix.SetIJKT3D(xBasis, yBasis, zBasis, translation);

		Strings scaleValues = SplitStringOnDelimiter(scaleString, ',');
		if (scaleValues.size() == 1)
		{
			// Use uniform scaling if there's only one value
			float uniformScale = std::stof(scaleValues[0]);
			transformMatrix.AppendScaleUniform3D(uniformScale);
		}
		else if (scaleValues.size() == 3)
		{
			// Use non-uniform scaling if there are three values
			float scaleX = std::stof(scaleValues[0]);
			float scaleY = std::stof(scaleValues[1]);
			float scaleZ = std::stof(scaleValues[2]);
			Vec3 nonUniformScale(scaleX, scaleY, scaleZ);
			transformMatrix.AppendScaleNonUniform3D(nonUniformScale);
		}
		else
		{
			GUARANTEE_OR_DIE(false, "Invalid scale format in XML");
		}

		LoadObj(objAbsPath.string(), transformMatrix);
		return true;
	}
	return false;
}

bool Model::LoadObj(const std::string& filename, const Mat44& transform)
{
	if (m_cpuMesh == nullptr)
	{
		m_cpuMesh = new CPUMesh();
	}

	m_cpuMesh->Load(filename, transform);

	CalculateTangentSpaceBasisVectors(m_cpuMesh->m_vertexes, m_cpuMesh->m_indexes);

	if (m_gpuMesh == nullptr)
	{
		m_gpuMesh = new GPUMesh();
	}

	m_gpuMesh->Create(m_cpuMesh);

	m_objFilename = filename;

	return true;
}


void Model::Update()
{
}

void Model::Render() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->BindShader(nullptr);
	if (m_gpuMesh)
	{
		g_theRenderer->DrawVertexBufferIndex(m_gpuMesh->m_vertexBuffer, m_gpuMesh->m_indexBuffer, VertexType::Vertex_PCUTBN, static_cast<int>(m_cpuMesh->m_indexes.size()));
	}
}

void Model::RenderLoadedModel() const
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizerState(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	if (m_material)
	{
		g_theRenderer->BindShader(m_material->m_shader);

		if (m_material->m_diffuseTexture)
		{
			g_theRenderer->BindTexture(0, m_material->m_diffuseTexture);
		}

		if (m_material->m_normalTexture)
		{
			g_theRenderer->BindTexture(1, m_material->m_normalTexture);
		}

		if (m_material->m_specGlossEmitTexture)
		{
			g_theRenderer->BindTexture(2, m_material->m_specGlossEmitTexture);
		}
	}
	else
	{
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(0, nullptr);
	}

	if (m_gpuMesh)
	{
		g_theRenderer->DrawVertexBufferIndex(m_gpuMesh->m_vertexBuffer, m_gpuMesh->m_indexBuffer, VertexType::Vertex_PCUTBN, static_cast<int>(m_cpuMesh->m_indexes.size()));
	}
}

Mat44 Model::GetModelMatrix() const
{
	Mat44 translation = Mat44::CreateTranslation3D(m_position);
	Mat44 orientation = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();

	translation.Append(orientation);
	return translation;
}
