#pragma once
#include "Toolbox/Render/CubeMapRHI.h"
#include "common.h"

class DX11CubeMapRHI : public CC3DCubeMapRHI
{
public:
	DX11CubeMapRHI();
	virtual ~DX11CubeMapRHI();

	virtual bool Init(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget);

	virtual void SetRenderTarget(int32_t targetViewSlot, int32_t nMip = 0);
	virtual void ClearDepthBuffer();
	virtual void ClearRenderTarget(float fRed, float fGreen, float fBlue, float fAlpha);
	virtual void Destory();

	ID3D11ShaderResourceView* GetSRV();
	ID3D11DepthStencilView* GetDSV();
private:
	std::shared_ptr<class DX11CubeMap> m_CubeMap;
};