#include "GLTexture2D.h"
#include "CC3DEngine/Material/CC3DTexture.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/GLDynamicRHI.h"
#include "Toolbox/GL/GLResource.h"
#include <unordered_map>

GLTexture2D::GLTexture2D()
{

	m_Texture = std::make_shared<CC3DTexture>();
}

GLTexture2D::~GLTexture2D()
{
	if (m_NeedToRelease)
	{
		m_Texture->Release();
	}
}

bool GLTexture2D::InitTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer /*= nullptr*/, int rowBytes /*= 0*/, bool bGenMipmap /*= false*/, bool bMultSample /*= false*/)
{
	auto [InternalFormat, FoundType, FoundFormat] = ConvertGLTextureFormat(format);
	if (InternalFormat == 0 || FoundType == 0 || FoundFormat == 0)
	{
		return false;
	}

	if (FoundType == GL_UNSIGNED_BYTE)
	{
		return m_Texture->LoadTexture(static_cast<uint8_t*>(pBuffer), width, height, bGenMipmap);
	}
	else
	{
		return m_Texture->LoadTextureRGBA(static_cast<GLfloat*>(pBuffer), width, height, InternalFormat, FoundFormat);
	}
	
}

bool GLTexture2D::InitTextureFromFile(const std::string szFile, bool bGenMipmap /*= false*/)
{
	return m_Texture->LoadTexture(szFile.c_str(), bGenMipmap);
}

bool GLTexture2D::InitTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap /*= false*/)
{
	int32_t Width = 0;
	int32_t Height = 0;
	uint8_t* ImageData =  CreateImgFromZIP(static_cast<HZIP>(hZip), szImagePath, Width, Height);
	if (ImageData)
	{
		bool InitFlag = InitTexture(0, 0, Width, Height, ImageData, 0, bGenMipmap, false);
		delete []ImageData;
		return InitFlag;
	}
	return false;
}

bool GLTexture2D::InitTextureFromFileCPUAcess(const std::string szFile)
{
	return m_Texture->LoadTexture(szFile.c_str(), false);
}

uint32_t GLTexture2D::GetWidth() const
{
	return m_Texture->GetTextureWidth();
}

uint32_t GLTexture2D::GetHeight() const
{
	return m_Texture->GetTextureHeight();
}

void GLTexture2D::InitFrameBuffer(uint32_t texture, int nWidth, int nHeight, bool bUseDepthBuffer /*= false*/)
{
	m_Texture->Init(texture, nWidth, nHeight, bUseDepthBuffer);
}

void GLTexture2D::InitFrameBuffer(int nWidth, int nHeight, bool bUseDepthBuffer /*= false*/, GLint internalformat /*= GL_RGBA*/, 
	GLenum format /*= GL_RGBA*/, GLenum type /*= GL_UNSIGNED_BYTE*/, bool bUseMultiTarget /*= false*/)
{
	m_Texture->Init(nWidth, nHeight, bUseDepthBuffer, internalformat, format, type, bUseMultiTarget);
}

uint32_t GLTexture2D::GetTextureID() const
{
	return m_Texture->GetGLTextureID();
}

uint32_t GLTexture2D::GetFrameBufferID() const
{
	return m_Texture->m_FBO->framebufferID;
}

void GLTexture2D::AttatchTextureId(uint32_t id)
{
	m_Texture->Release();
	m_NeedToRelease = false;
	m_Texture->m_TextureID = id;
}

void GLTexture2D::AttatchTextureId(std::shared_ptr< CC3DTextureRHI> TexRHI)
{
	GLTexture2D* RHI = RHIResourceCast(TexRHI.get());
	AttatchTextureId(RHI->GetTextureID());
}

bool GLTexture2D::updateTextureInfo(void* pBuffer, int w, int h)
{
	if (m_Texture)
	{
		return m_Texture->LoadTexture(static_cast<uint8_t*>(pBuffer), w, h, false);
	}
	return false;
}
