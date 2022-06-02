#include "DX11Texture2D.h"
#include "Toolbox/DXUtils/DX11Texture.h"

DWORD ms_dwTextureFormatType[] =
{
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_B8G8R8A8_UNORM,
	DXGI_FORMAT_B8G8R8X8_UNORM,
	DXGI_FORMAT_D16_UNORM,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R16_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_D24_UNORM_S8_UINT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_B5G6R5_UNORM,
	DXGI_FORMAT_A8_UNORM,
	DXGI_FORMAT_R16G16_UNORM,
	DXGI_FORMAT_R16G16B16A16_UNORM,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R8G8B8A8_SNORM,
	DXGI_FORMAT_R16G16_SNORM,
	DXGI_FORMAT_R16_UNORM,
	DXGI_FORMAT_R16G16B16A16_SNORM,
	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC2_UNORM,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC5_UNORM
};
DWORD ms_dwSRGBTextureFormatType[] =
{
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
	DXGI_FORMAT_D16_UNORM,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R16_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_D24_UNORM_S8_UINT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_B5G6R5_UNORM,
	DXGI_FORMAT_A8_UNORM,
	DXGI_FORMAT_R16G16_UNORM,
	DXGI_FORMAT_R16G16B16A16_UNORM,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R8G8B8A8_SNORM,
	DXGI_FORMAT_R16G16_SNORM,
	DXGI_FORMAT_R16_UNORM,
	DXGI_FORMAT_R16G16B16A16_SNORM,
	DXGI_FORMAT_BC1_UNORM_SRGB,
	DXGI_FORMAT_BC2_UNORM_SRGB,
	DXGI_FORMAT_BC3_UNORM_SRGB,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC5_UNORM
};

DX11Texture2D::DX11Texture2D()
{
	m_DX11Texture = std::make_shared<DX11Texture>();
}

DX11Texture2D::~DX11Texture2D()
{
}

bool DX11Texture2D::InitTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer /*= NULL*/, int rowBytes /*= 0*/, bool bGenMipmap /*= false*/, bool bMultSample /*= false*/)
{
	m_Attatched = nullptr;
	UINT DXBindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (BindFlags == OT_RENDER_TARGET)
	{
		DXBindFlags |= D3D11_BIND_RENDER_TARGET;
	}

	return m_DX11Texture->initTexture((DXGI_FORMAT)ms_dwTextureFormatType[format], DXBindFlags, width, height, pBuffer, rowBytes, bGenMipmap, bMultSample);
}

bool DX11Texture2D::InitTextureFromFile(const std::string szFile, bool bGenMipmap /*= false*/)
{
	m_Attatched = nullptr;
	return m_DX11Texture->initTextureFromFile(szFile, bGenMipmap);
}

bool DX11Texture2D::InitTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap /*= false*/)
{
	m_Attatched = nullptr;
	return m_DX11Texture->initTextureFromZip((HZIP)hZip, szImagePath, bGenMipmap);
}

bool DX11Texture2D::InitTextureFromFileCPUAcess(const std::string szFile)
{
	m_Attatched = nullptr;
	return m_DX11Texture->initTextureFromFileCPUAcess(szFile);
}

uint32_t DX11Texture2D::GetWidth() const
{
	if (m_Attatched)
	{
		return m_Attatched->width();
	}
	return m_DX11Texture->width();
}

uint32_t DX11Texture2D::GetHeight() const
{
	if (m_Attatched)
	{
		return m_Attatched->height();
	}
	return m_DX11Texture->height();
}

ID3D11Texture2D* DX11Texture2D::GetNativeTex()
{
	if (m_Attatched)
	{
		return m_Attatched->getTex();
	}
	return m_DX11Texture->getTex();
}

ID3D11ShaderResourceView* DX11Texture2D::GetSRV()
{
	if (m_Attatched)
	{
		return m_Attatched->getTexShaderView();
	}
	return m_DX11Texture->getTexShaderView();
}

void DX11Texture2D::Attatch(DX11Texture* Tex)
{
	m_DX11Texture.reset();
	m_Attatched = Tex;
}

