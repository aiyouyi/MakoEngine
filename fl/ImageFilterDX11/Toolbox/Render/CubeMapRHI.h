#ifndef CC3D_CubeMapRHI
#define CC3D_CubeMapRHI

#include "CC3DEngine/Common/CC3DUtils.h"

class CC3DCubeMapRHI
{
public:
	CC3DCubeMapRHI();
	virtual ~CC3DCubeMapRHI();

	virtual bool Init(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget) = 0;

	virtual void SetRenderTarget(int32_t targetViewSlot, int32_t nMip = 0) = 0;
	virtual void ClearDepthBuffer() = 0;
	virtual void ClearRenderTarget(float fRed, float fGreen, float fBlue, float fAlpha) = 0;
	virtual void Destory() = 0;
	virtual void UnBindRenderTarget() {}
};

#endif