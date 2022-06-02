#include "DX11RenderTarget.h"
#include "DX11FBO.h"
#include "DX11Texture2D.h"
#include "DX11Resource.h"

extern DWORD ms_dwTextureFormatType[];

DX11RenderTarget::DX11RenderTarget()
{
	m_RenderTarget = std::make_shared<DX11FBO>();
}

DX11RenderTarget::~DX11RenderTarget()
{

}

bool DX11RenderTarget::InitWithTexture(int width, int height, bool useDepthBuffer /*= false*/, std::shared_ptr< CC3DTextureRHI> DestTexRHI /*= nullptr*/, uint8_t format /*= 0*/)
{
	m_nWidth = width;
	m_nHeight = height;
	ID3D11Texture2D* DestTex = nullptr;
	if (DestTexRHI)
	{
		DestTex = RHIResourceCast(DestTexRHI.get())->GetNativeTex();
	}
	return m_RenderTarget->initWithTexture(width, height, useDepthBuffer, DestTex, (DXGI_FORMAT)ms_dwTextureFormatType[format]);
}

void DX11RenderTarget::Bind()
{
	m_RenderTarget->bind();
}

void DX11RenderTarget::UnBind()
{
	m_RenderTarget->unBind();
}

void DX11RenderTarget::Clear(float r, float g, float b, float a, float depth /*= 1.0f*/, unsigned char Stencil /*= 0*/)
{
	m_RenderTarget->clear(r, g, b, a, depth, Stencil);
}

ID3D11Texture2D* DX11RenderTarget::GetNativeTex()
{
	if (!m_RenderTarget->getTexture())
	{
		return nullptr;
	}
	return m_RenderTarget->getTexture()->getTex();
}

ID3D11RenderTargetView* DX11RenderTarget::GetRTV()
{
	return m_RenderTarget->getRenderTargetView();
}

ID3D11ShaderResourceView* DX11RenderTarget::GetSRV()
{
	return m_RenderTarget->getTexture()->getTexShaderView();
}

ID3D11DepthStencilView* DX11RenderTarget::GetDSV()
{
	return m_RenderTarget->getDepthStencilView();
}

int DX11RenderTarget::GetWidth()
{
	return m_nWidth;
}

int DX11RenderTarget::GetHeight()
{
	return m_nHeight;
}

DX11Texture* DX11RenderTarget::GetTexture()
{
	return m_RenderTarget->getTexture();
}
