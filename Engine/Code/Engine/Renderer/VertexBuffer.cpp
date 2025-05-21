#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <d3d11.h>

VertexBuffer::VertexBuffer(size_t size)
	:m_size(size)
{

}

VertexBuffer::~VertexBuffer()
{
	ReleaseD3D();
}

void VertexBuffer::SetIsLinePrimative(bool isLinePrimative)
{
	if (!isLinePrimative)
	{
		m_isLinePrimitive = false;
	}
	else
	{
		m_isLinePrimitive = true;
	}
}

void VertexBuffer::ReleaseD3D()
{
	// Release buffer
	DX_SAFE_RELEASE(m_buffer);
}
