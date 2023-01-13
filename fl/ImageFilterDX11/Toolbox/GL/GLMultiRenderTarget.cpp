#include "GLMultiRenderTarget.h"
#include "CCFrameBuffer.h"
#include "Toolbox/GL/GLDynamicRHI.h"

GLMultiRenderTarget::GLMultiRenderTarget()
{
	m_MrtBuffer = std::make_shared<CCFrameBuffer>();
}

GLMultiRenderTarget::~GLMultiRenderTarget()
{
	GL_DELETE_RENDERBUFFER(mDepthRenderBuffer);
}

bool GLMultiRenderTarget::InitWithTexture(int width, int height, bool useDepthBuffer /*= false*/, std::shared_ptr< CC3DTextureRHI> pDestTexture /*= nullptr*/, uint8_t format /*= 0*/)
{
	auto [InternalFormat, FoundType, FoundFormat] = ConvertGLTextureFormat(format);
	GL_CHECK();
	m_MrtBuffer->init(width, height, useDepthBuffer, InternalFormat, FoundFormat, FoundType,true);
	GL_CHECK();

	GL_CHECK();
	return true;
}

void GLMultiRenderTarget::Bind()
{
	if (m_MrtBuffer)
	{
		m_MrtBuffer->bind();
	}

	GetDynamicRHI()->SetViewPort(0, 0, m_MrtBuffer->width, m_MrtBuffer->height);
}

void GLMultiRenderTarget::UnBind()
{
	if (m_MrtBuffer)
	{
		m_MrtBuffer->unbind();
	}
}

void GLMultiRenderTarget::Clear(float r, float g, float b, float a, float depth /*= 1.0f*/, unsigned char Stencil /*= 0*/)
{
	glClearColor(r, g, b, a);
	if (mDepthRenderBuffer)
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
}

uint32_t GLMultiRenderTarget::GetTextureId() const
{
	return m_MrtBuffer->textureID;
}

uint32_t GLMultiRenderTarget::GetSecondTextureId() const
{
	return m_MrtBuffer->secondID;
}

