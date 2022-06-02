#include "DX11ContanstBuffer.h"
#include "Toolbox/Helper.h"
#include "Toolbox/DXUtils/DXUtils.h"

DX11ConstantBuffer::DX11ConstantBuffer()
{

}

DX11ConstantBuffer::~DX11ConstantBuffer()
{
	SAFERALEASE(PtrConstantBuffer)
}

void DX11ConstantBuffer::CreateBuffer(int32_t ByteSize)
{
	PtrConstantBuffer = DXUtils::CreateConstantBuffer(ByteSize);
}
