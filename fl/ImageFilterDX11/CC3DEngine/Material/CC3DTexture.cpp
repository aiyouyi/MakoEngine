
#include "CC3DTexture.h"
#include"Toolbox/GL/openglutil.h"
#include"BaseDefine/Define.h"
CC3DTexture::CC3DTexture()
{
	this->Initialize();
}

CC3DTexture::~CC3DTexture(void)
{
	this->Release();
}

void CC3DTexture::Initialize()
{
	m_TextureID = 0;
	m_nTextureWidth = 0;
	m_nTextureHeight = 0;
}

void CC3DTexture::Release()
{
	GL_DELETE_TEXTURE(m_pTextureLoad);
	SAFE_DELETE(m_FBO);
}

bool CC3DTexture::LoadTexture(const char* filePath, bool useMipmap)
{
	if (filePath == NULL)
	{
		LOGE("LoadTexture faile");
	}
	uint8* pSrcData = NULL;
	int32 nWidth;
	int32 nHeight;
	int32 n;

	pSrcData = ccLoadImage(filePath, &nWidth, &nHeight, &n, 4);

	if (pSrcData == NULL || nWidth <= 0 || nHeight <= 0)
	{
		LOGE("LoadTexture %s faile !", filePath);
		SAFE_DELETE_ARRAY(pSrcData);
		return false;
	}
	bool bIsOk = LoadTexture(pSrcData, (uint32)nWidth, (uint32)nHeight, useMipmap);
	SAFE_DELETE_ARRAY(pSrcData);

	m_nTextureWidth = nWidth;
	m_nTextureHeight = nHeight;
	return bIsOk;
}

bool CC3DTexture::LoadTexture(uint8* pSrcRGBA, uint32 width, uint32 height, bool useMipmap)
{
	GL_DELETE_TEXTURE(m_TextureID);
	OpenGLUtil::createToTexture(m_TextureID, width, height, pSrcRGBA, GL_RGBA, useMipmap);

	if (m_TextureID == 0)
	{
		LOGE("Load GLTexture Faile !");
		return false;
	}
	m_pTextureLoad = m_TextureID;
	m_nTextureWidth = width;
	m_nTextureHeight = height;
	return true;
}

bool CC3DTexture::LoadTexture(float r, float g, float b, float a)
{
	GL_DELETE_TEXTURE(m_TextureID);
	uint8 tmp[] = { (uint8)(r * 255),(uint8)(g * 255),(uint8)(b * 255),(uint8)(a * 255) };
	OpenGLUtil::createToTexture(m_TextureID, 1, 1, tmp, GL_RGBA, false);

	if (m_TextureID == 0)
	{
		LOGE("Load GLTexture Faile !");
		return false;
	}
	m_nTextureWidth = 1;
	m_nTextureHeight = 1;
	m_pTextureLoad = m_TextureID;
	return true;
}

void CC3DTexture::Init(GLuint texture, int nWidth, int nHeight, bool bUseDepthBuffer /*= false*/)
{
	if (!m_FBO)
	{
		m_FBO = new CCFrameBuffer();
	}
	m_FBO->init(texture, nWidth, nHeight, bUseDepthBuffer);
	m_TextureID = m_FBO->textureID;
	m_nTextureWidth = nWidth;
	m_nTextureHeight = nHeight;
	m_pTextureLoad = m_TextureID;
}

void CC3DTexture::Init(int nWidth, int nHeight, bool bUseDepthBuffer /*= false*/, GLint internalformat /*= GL_RGBA*/, GLenum format /*= GL_RGBA*/, 
	GLenum type /*= GL_UNSIGNED_BYTE*/, bool bUseMultiTarget /*= false*/)
{
	if (!m_FBO)
	{
		m_FBO = new CCFrameBuffer();
	}
	m_FBO->init(nWidth, nHeight, bUseDepthBuffer, internalformat, format, type, bUseMultiTarget);
	m_TextureID = m_FBO->textureID;
	m_nTextureWidth = nWidth;
	m_nTextureHeight = nHeight;
	m_pTextureLoad = m_TextureID;
}

bool CC3DTexture::LoadTextureRGBA(GLfloat* pSrcRGBA, uint32 width, uint32 height, GLenum InternalFormat, GLenum Format)
{
	GL_DELETE_TEXTURE(m_TextureID);
	m_TextureID = OpenGLUtil::CreateTextureRGBAF2(width, height, pSrcRGBA, InternalFormat, Format);
	if (m_TextureID == 0)
	{
		LOGE("Load GLTexture Faile !");
		return false;
	}
	m_nTextureWidth = 1;
	m_nTextureHeight = 1;
	m_pTextureLoad = m_TextureID;
	return true;
}

void CC3DTexture::bindFBO()
{
	if (m_FBO == NULL)
	{
		m_FBO = new CCFrameBuffer();
		m_FBO->init(m_nTextureWidth, m_nTextureHeight);
	}
	m_FBO->bind();
}

void CC3DTexture::unbindFBO()
{
	m_FBO->unbind();
	m_TextureID = m_FBO->textureID;
}

uint32 CC3DTexture::GetGLTextureID()
{
	return m_TextureID;
}

uint32 CC3DTexture::GetSrcTextureID()
{
	if (m_FBO == NULL)
	{
		return m_pTextureLoad;
	}
	return m_FBO->textureID;
}

uint32 CC3DTexture::GetTextureWidth()
{
	return m_nTextureWidth;
}

uint32 CC3DTexture::GetTextureHeight()
{
	return m_nTextureHeight;
}