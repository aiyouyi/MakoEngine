#ifndef CC3D_RenderTargetRHI
#define CC3D_RenderTargetRHI

#include "CC3DEngine/Common/CC3DUtils.h"

class CC3DTextureRHI;

class CC3DRenderTargetRHI
{
public:
	CC3DRenderTargetRHI();
	virtual ~CC3DRenderTargetRHI();

	virtual bool InitWithTexture(int width, int height, bool useDepthBuffer = false, std::shared_ptr< CC3DTextureRHI> pDestTexture = nullptr, uint8_t format = 0) = 0;
	virtual void Bind() = 0;
	virtual void UnBind() = 0;
	virtual void Clear(float r, float g, float b, float a, float depth = 1.0f, unsigned char Stencil = 0) = 0;
	virtual int GetWidth()=0;
	virtual int GetHeight()=0;
};

#endif