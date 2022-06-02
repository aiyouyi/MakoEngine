#include "DX11IndexBuffer.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/Helper.h"

DX11IndexBuffer::DX11IndexBuffer()
{

}

DX11IndexBuffer::~DX11IndexBuffer()
{
	SAFERALEASE(PtrIndexBuffer);
}

void DX11IndexBuffer::CreateIndexBuffer(unsigned short* pData, int nTriangle)
{
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
	m_NumberTrangle = nTriangle;
	PtrIndexBuffer = DXUtils::CreateIndexBuffer(pData, nTriangle);
}

void DX11IndexBuffer::CreateIndexBuffer(unsigned int* pData, int nTriangle)
{
	m_IndexFormat = DXGI_FORMAT_R32_UINT;
	m_NumberTrangle = nTriangle;
	PtrIndexBuffer = DXUtils::CreateIndexBuffer(pData, nTriangle);
}

uint32_t DX11IndexBuffer::GetNumberTriangle() const 
{
	return m_NumberTrangle;
}

int32_t DX11IndexBuffer::GetIndexFormat() const
{
	return m_IndexFormat;
}

std::shared_ptr<CC3DIndexBuffer> CC3DCreateIndexBuffer(unsigned short* pData, int nTriangle)
{
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer = std::make_shared<DX11IndexBuffer>();
	IndexBuffer->CreateIndexBuffer(pData, nTriangle);
	return IndexBuffer;
}

std::shared_ptr<CC3DIndexBuffer> CC3DCreateIndexBuffer(unsigned int* pData, int nTriangle)
{
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer = std::make_shared<DX11IndexBuffer>();
	IndexBuffer->CreateIndexBuffer(pData, nTriangle);
	return IndexBuffer;
}