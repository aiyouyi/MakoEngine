#pragma once
#include <d3d11.h>
#include <string>
#include "Toolbox/zip/unzip.h"
#include "IReferenceCounted.h"

#define CCMaxMipMap 10
class DX11IMAGEFILTER_EXPORTS_CLASS DX11CubeMap: public IReferenceCounted
{
public:
	DX11CubeMap();
	virtual ~DX11CubeMap();
	DX11CubeMap(int nTextureWidth, int nTexureHeight,int nMipLevels = -1,bool UseDepth = false,bool GenMipRetarget = false);


	bool Init(int nTextureWidth, int nTexureHeight, int nMipLevels,bool UseDepth, bool GenMipRetarget);

	void SetRenderTarget(int targetViewSlot,int nMip = 0);
	void ClearDepthBuffer();
	void ClearRenderTarget(float fRed, float fGreen, float fBlue, float fAlpha);

	void destory();

public:
	int textureWidth, textureHeight;

	ID3D11RenderTargetView* rtvs[6][CCMaxMipMap];
	ID3D11ShaderResourceView* srv;
	ID3D11DepthStencilView* dsv;
private:
	ID3D11Texture2D* cubeMapTexture;
	//DepthBuffer
	ID3D11Texture2D* depthTexture;

	D3D11_VIEWPORT viewport;

};

