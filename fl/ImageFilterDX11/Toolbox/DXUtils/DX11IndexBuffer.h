#pragma once
#include "Toolbox/Render/IndexBuffer.h"
#include "common.h"

class DX11IndexBuffer : public CC3DIndexBuffer
{
public:
	DX11IndexBuffer();
	virtual ~DX11IndexBuffer();

	virtual void CreateIndexBuffer(unsigned short* pData, int nTriangle);
	virtual void CreateIndexBuffer(unsigned int* pData, int nTriangle);
	virtual void UpdateIndexBuffer(unsigned short* pData, int nTriangle, int sizePerTriangle, int offset);
	virtual uint32_t GetNumberTriangle() const;
	virtual int32_t GetIndexFormat() const;
public:
	ID3D11Buffer* PtrIndexBuffer = nullptr;
private:
	uint32_t m_NumberTrangle = 0;
	int32_t m_IndexFormat = DXGI_FORMAT_R16_UINT;
};
