#ifndef _GL_MULTI_RENDERTARGET_H_
#define _GL_MULTI_RENDERTARGET_H_

#include "Render/RenderTargetRHI.h"
#include <unordered_map>

class GLMultiRenderTarget : public CC3DRenderTargetRHI
{
public:
	GLMultiRenderTarget();
	virtual ~GLMultiRenderTarget();

	virtual bool InitWithTexture(int width, int height, bool useDepthBuffer = false, std::shared_ptr< CC3DTextureRHI> pDestTexture = nullptr, uint8_t format = 0);
	virtual void Bind();
	virtual void UnBind();
	virtual void Clear(float r, float g, float b, float a, float depth = 1.0f, unsigned char Stencil = 0);

	uint32_t GetTextureId() const;
	uint32_t GetSecondTextureId() const;

private:
	std::shared_ptr<class CCFrameBuffer> m_MrtBuffer;

	GLuint mDepthRenderBuffer = 0;
};

#endif