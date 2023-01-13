#include "GLRenderTarget.h"
#include "CCFrameBuffer.h"
#include "Toolbox/GL/GLDynamicRHI.h"

GLRenderTarget::GLRenderTarget()
{
	m_FrameBuffer = std::make_shared<CCFrameBuffer>();
}

GLRenderTarget::~GLRenderTarget()
{
	GL_DELETE_RENDERBUFFER(mDepthRenderBuffer);
}

bool GLRenderTarget::InitWithTexture(int width, int height, bool useDepthBuffer /*= false*/, std::shared_ptr< CC3DTextureRHI> pDestTexture /*= nullptr*/, uint8_t format /*= 0*/)
{
	auto [InternalFormat, FoundType,FoundFormat] = ConvertGLTextureFormat(format);

    //GL_CHECK();
	m_FrameBuffer->init(width, height, useDepthBuffer, InternalFormat, FoundFormat, FoundType);
	return true;
}

void GLRenderTarget::Bind()
{
	if (m_FrameBuffer)
	{
		m_FrameBuffer->bind();
	}
	
	GetDynamicRHI()->SetViewPort(0, 0, m_FrameBuffer->width, m_FrameBuffer->height);
}

void GLRenderTarget::UnBind()
{
	if (m_FrameBuffer)
	{
		m_FrameBuffer->unbind();
	}
}

void GLRenderTarget::Clear(float r, float g, float b, float a, float depth /*= 1.0f*/, unsigned char Stencil /*= 0*/)
{
	glClearColor(r, g, b, a);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

uint32_t GLRenderTarget::GetTextureId() const
{
	return m_FrameBuffer->textureID;
}

int GLRenderTarget::GetWidth()
{
	return m_FrameBuffer->width;
}

int GLRenderTarget::GetHeight()
{
	return m_FrameBuffer->height;
}

