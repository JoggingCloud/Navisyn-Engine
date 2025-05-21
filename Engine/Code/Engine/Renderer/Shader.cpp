#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <d3d11.h>

Shader::Shader(const ShaderConfig& config)
	: m_config(config)
{

}

Shader::~Shader()
{
	ReleaseD3D();
}

const std::string& Shader::GetName() const
{
	return m_config.m_name;
}

void Shader::ReleaseD3D()
{
	// Release vertex shader
	DX_SAFE_RELEASE(m_vertexShader);

	// Release pixel shader
	DX_SAFE_RELEASE(m_pixelShader);

	// Release input layout
	DX_SAFE_RELEASE(m_vertexPCUInputLayout);
	DX_SAFE_RELEASE(m_vertexPCUTBNInputLayout);
}
