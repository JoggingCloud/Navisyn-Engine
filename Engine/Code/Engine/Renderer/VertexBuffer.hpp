#pragma once

class VertexBuffer
{
	friend class Renderer;

public:
	VertexBuffer(size_t sizezz);
	VertexBuffer(const VertexBuffer& copy) = delete;
	virtual ~VertexBuffer();

	void SetIsLinePrimative(bool isLinePrimative);
	void ReleaseD3D();

	struct ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;
	bool m_isLinePrimitive = false;
};