#include "DX11CubeMapRHI.h"
#include "DX11CubeMap.h"

DX11CubeMapRHI::DX11CubeMapRHI()
{
	m_CubeMap = std::make_shared<DX11CubeMap>();
}

DX11CubeMapRHI::~DX11CubeMapRHI()
{

}

bool DX11CubeMapRHI::Init(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget)
{
	return m_CubeMap->Init(nTextureWidth, nTexureHeight, nMipLevels, UseDepth, GenMipRetarget);
}

void DX11CubeMapRHI::SetRenderTarget(int32_t targetViewSlot, int32_t nMip /*= 0*/)
{
	m_CubeMap->SetRenderTarget(targetViewSlot, nMip);
}

void DX11CubeMapRHI::ClearDepthBuffer()
{
	m_CubeMap->ClearDepthBuffer();
}

void DX11CubeMapRHI::ClearRenderTarget(float fRed, float fGreen, float fBlue, float fAlpha)
{
	m_CubeMap->ClearRenderTarget(fRed, fGreen, fBlue, fAlpha);
}

void DX11CubeMapRHI::Destory()
{
	m_CubeMap->destory();
}

ID3D11ShaderResourceView* DX11CubeMapRHI::GetSRV()
{
	return m_CubeMap->srv;
}

ID3D11DepthStencilView* DX11CubeMapRHI::GetDSV()
{
	return m_CubeMap->dsv;
}

