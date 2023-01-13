#ifndef __GlRenderTarget__
#define __GlRenderTarget__

#include "Render/RenderTargetRHI.h"
#include <unordered_map>

class GLRenderTarget : public CC3DRenderTargetRHI
{
public:
	GLRenderTarget();
	virtual ~GLRenderTarget();

	virtual bool InitWithTexture(int width, int height, bool useDepthBuffer = false, std::shared_ptr< CC3DTextureRHI> pDestTexture = nullptr, uint8_t format = 0);
	virtual void Bind();
	virtual void UnBind();
	virtual void Clear(float r, float g, float b, float a, float depth = 1.0f, unsigned char Stencil = 0);

	uint32_t GetTextureId() const;

	virtual int GetWidth();
	virtual int GetHeight();

private:
	std::shared_ptr<class CCFrameBuffer> m_FrameBuffer;

	GLuint mDepthRenderBuffer = 0;
};

#endif