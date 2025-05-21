#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <d3d11.h>

IndexBuffer::IndexBuffer(size_t size)
	:m_size(size)
{

}

IndexBuffer::~IndexBuffer()
{
	ReleaseD3D();
}

size_t IndexBuffer::GetSize()
{
	return m_size;
}

void IndexBuffer::ReleaseD3D()
{
	// Release buffer
	DX_SAFE_RELEASE(m_buffer);
}
