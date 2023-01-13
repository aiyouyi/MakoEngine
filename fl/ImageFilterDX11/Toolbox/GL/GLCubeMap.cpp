#include "GLCubeMap.h"
#include "ToolBox/GL/openglutil.h"

GLCubeMap::GLCubeMap()
{

}

GLCubeMap::~GLCubeMap()
{
	Destory();
}

bool GLCubeMap::Init(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget)
{
	m_Width = nTextureWidth;
	m_Height = nTexureHeight;

	OpenGLUtil::createToCubeMap(m_CubeTexId, nTextureWidth, nTexureHeight, GL_RGB16F, GL_RGB, GenMipRetarget);
	if (m_CubeTexId == 0)
	{
		return false;
	}

	//if (GenMipRetarget)
	//{
	//	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeTexId);
	//	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	//	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	//}

	glGenFramebuffers(1, &m_captureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	
	if (UseDepth)
	{
		glGenRenderbuffers(1, &m_captureRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, nTextureWidth, nTexureHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);
		GL_CHECK();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return m_captureFBO != 0;
}

void GLCubeMap::SetRenderTarget(int32_t targetViewSlot, int32_t nMip /*= 0*/)
{
	unsigned int mipWidth = m_Width * std::pow(0.5, nMip);
	unsigned int mipHeight = m_Height * std::pow(0.5, nMip);

	glViewport(0, 0, mipWidth, mipHeight);

	if (m_captureFBO > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
		if (m_captureRBO)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			GL_CHECK();
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + targetViewSlot, m_CubeTexId, nMip);
		GL_CHECK();
	}
	

}

void GLCubeMap::ClearDepthBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLCubeMap::ClearRenderTarget(float fRed, float fGreen, float fBlue, float fAlpha)
{
	glClearColor(fRed, fGreen, fBlue, fAlpha);
}

void GLCubeMap::Destory()
{
	GL_DELETE_FRAMEBUFFER(m_captureFBO);
	GL_DELETE_RENDERBUFFER(m_captureRBO);
	GL_DELETE_TEXTURE(m_CubeTexId);
}

void GLCubeMap::UnBindRenderTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

uint32_t GLCubeMap::GetTextureId() const
{
	return m_CubeTexId;
}

GLuint GLCubeMap::GetFBO() const
{
	return m_captureFBO;
}
