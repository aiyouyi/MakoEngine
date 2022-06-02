#pragma once
#include "Toolbox/Render/RenderTargetRHI.h"
#include "common.h"

class DX11FBO;
class DX11RenderTarget : public CC3DRenderTargetRHI
{
public:
	DX11RenderTarget();
	virtual ~DX11RenderTarget();

	virtual bool InitWithTexture(int width, int height, bool useDepthBuffer = false, std::shared_ptr< CC3DTextureRHI> DestTexRHI = nullptr, uint8_t format = 0);
	virtual void Bind();
	virtual void UnBind();
	virtual void Clear(float r, float g, float b, float a, float depth = 1.0f, unsigned char Stencil = 0);

	ID3D11Texture2D* GetNativeTex();
	ID3D11RenderTargetView* GetRTV();
	ID3D11ShaderResourceView* GetSRV();
	ID3D11DepthStencilView* GetDSV();

	virtual int GetWidth();
	virtual int GetHeight();

	class DX11Texture* GetTexture();

private:
	std::shared_ptr<DX11FBO> m_RenderTarget;
	int m_nWidth;
	int m_nHeight;
};