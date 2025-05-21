#pragma once

class IndexBuffer
{
	friend class Renderer;

public:
	IndexBuffer(size_t size);
	IndexBuffer(const IndexBuffer& copy) = delete;
	virtual ~IndexBuffer();

	size_t GetSize();

	void ReleaseD3D();

private:
	struct ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;
};