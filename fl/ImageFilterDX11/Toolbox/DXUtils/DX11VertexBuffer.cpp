#include "DX11VertexBuffer.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/Helper.h"

DX11VertexBuffer::DX11VertexBuffer()
{

}

DX11VertexBuffer::~DX11VertexBuffer()
{
	SAFERALEASE(PtrVertexBuffer);
}

void DX11VertexBuffer::CreateVertexBuffer(float* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex)
{
	m_Stride = nStride * sizeof(float);
	m_Vertex = nVertex;
	PtrVertexBuffer = DXUtils::CreateVertexBuffer(pData, nVertex, nStride);
}

void DX11VertexBuffer::CreateVertexBuffer(void* pData, int StrideByteWidth, int Count,bool StreamOut)
{
	m_Stride = StrideByteWidth ;
	m_Vertex = Count;
	if (StreamOut)
	{
		PtrVertexBuffer = DXUtils::CreateStreamOutVertexBuffer(pData, StrideByteWidth*Count);
	}
	else
	{
		PtrVertexBuffer = DXUtils::CreateVertexBuffer(pData, StrideByteWidth * Count);
	}
	
}

void DX11VertexBuffer::UpdateVertexBUffer(float* pData, int nVertex, int perUpdateSize, int sizePerVertex, int begin /*= 0*/, int offset /*= 0*/)
{
	m_Vertex = nVertex;
	DXUtils::UpdateVertexBuffer(PtrVertexBuffer, pData, nVertex, perUpdateSize, sizePerVertex, begin, offset);
}

int32_t DX11VertexBuffer::GetStride() const
{
	return m_Stride;
}

int32_t DX11VertexBuffer::GetCount() const
{
	return m_Vertex;
}

ID3D11Buffer* DX11VertexBuffer::GetNativeBuffer() const
{
	return PtrVertexBuffer;
}

