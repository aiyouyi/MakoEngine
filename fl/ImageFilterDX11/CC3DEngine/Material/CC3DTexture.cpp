
#include "CC3DTexture.h"
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
	m_pTexture = NULL;
	m_pTextureLoad = NULL;
	m_nTextureWidth = 0;
	m_nTextureHeight = 0;
}

void CC3DTexture::Release()
{
	SAFE_DELETE(m_pTextureLoad);

}

bool CC3DTexture::LoadTexture(const char * filePath, bool useMipmap)
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
	bool bIsOk = LoadTexture(pSrcData, (uint32)nWidth, (uint32)nHeight,useMipmap);
	SAFE_DELETE_ARRAY(pSrcData);
	m_nTextureWidth = nWidth;
	m_nTextureHeight = nHeight;
	return bIsOk;
}

bool CC3DTexture::LoadTexture(uint8 * pSrcRGBA, uint32 width, uint32 height, bool useMipmap)
{
	SAFE_DELETE(m_pTexture);
	m_pTexture = new DX11Texture();
	m_pTexture->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, width, height, pSrcRGBA, width * 4, useMipmap);
	m_pTextureLoad = m_pTexture;
	m_nTextureWidth = width;
	m_nTextureHeight = height;
	return true;
}

bool CC3DTexture::LoadTexture(float r, float g, float b, float a)
{

	SAFE_DELETE(m_pTexture);
	m_pTexture = new DX11Texture();
	
	uint8 tmp[] = { (uint8)(r*255),(uint8)(g*255),(uint8)(b*255),(uint8)(a*255) };
	m_pTexture->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, 1, 1,tmp, 4, false);
	m_nTextureWidth = 1;
	m_nTextureHeight = 1;
	m_pTextureLoad = m_pTexture;
	return true;
}


uint32 CC3DTexture::GetTextureWidth()
{
	return m_nTextureWidth;
}

uint32 CC3DTexture::GetTextureHeight()
{
	return m_nTextureHeight;
}







