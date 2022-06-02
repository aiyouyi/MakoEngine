#pragma once
#include "DX11Texture.h"
#include "IReferenceCounted.h"
class DX11IMAGEFILTER_EXPORTS_CLASS DX11FBO
{
public:
	DX11FBO();
	virtual ~DX11FBO();

	bool initWithTexture(int width, int height, bool useDepthBuffer=false, ID3D11Texture2D *pDestTexture = nullptr,DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool bMSAA = false);
	bool UpdateWithTextue(ID3D11Texture2D *pDestTexture);
	void bind();
	void unBind();

	void lock();
	void unlock();

	void clear(float r, float g, float b, float a, float depth=1.0f, unsigned char Stencil=0);
	void clearDepth( float depth = 1.0f, unsigned char Stencil = 0);
	DX11Texture *getTexture();
	ID3D11RenderTargetView *getRenderTargetView();

	ID3D11DepthStencilView *getDepthStencilView();

	int width() { return m_nWidth; }
	int height() { return m_nHeight; }

	void destory();

	friend class DX11FBOCache;
protected:
	void resetLockCount();

private:
	ID3D11RenderTargetView *m_pRenderTargetView;
	ID3D11DepthStencilView *m_pDepthStencilView;
	DX11Texture *m_pTexture;
	ID3D11Texture2D *m_pDestTexture;

	int m_nWidth;
	int m_nHeight;

	int m_nlockCount;
};

