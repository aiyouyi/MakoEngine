#pragma once
#include "Toolbox/Render/VertexBuffer.h"
#include "common.h"

class DX11VertexBuffer : public CC3DVertexBuffer
{
public:
	DX11VertexBuffer();
	virtual ~DX11VertexBuffer();

	virtual void CreateVertexBuffer(float* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex = 0);
	virtual void CreateVertexBuffer(void* pData, int StrideByteWidth,int Count, bool StreamOut);
	virtual void UpdateVertexBUffer(float* pData, int nVertex, int perUpdateSize, int sizePerVertex, int begin = 0, int offset = 0);
	
	virtual int32_t GetStride() const;
	virtual int32_t GetCount() const;
	ID3D11Buffer* GetNativeBuffer() const;
public:
	ID3D11Buffer* PtrVertexBuffer = nullptr;
	int32_t m_Stride = 0;
	int32_t m_Vertex = 0;
};
