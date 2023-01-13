#include "DX11DoubleBufferWrapper.h"
#include "Toolbox/DXUtils/DX11Resource.h"

extern DWORD ms_dwTextureFormatType[];

void DX11DoubleBufferWrapper::InitDoubleBuffer(std::shared_ptr<DoubleBufferRHI> doubleBuffer, int nWidth, int nHeight, bool UseDepth /*= false*/, bool bMSAA /*= false*/, int32_t format /*= CC3DTextureRHI::SFT_A8R8G8B8*/)
{
	DX11DoubleBuffer* DoubleBufferRHI = RHIResourceCast(doubleBuffer.get());
	DoubleBufferRHI->InitDoubleBuffer(nWidth, nHeight, UseDepth, bMSAA, static_cast<DXGI_FORMAT>(ms_dwTextureFormatType[format]));
}

