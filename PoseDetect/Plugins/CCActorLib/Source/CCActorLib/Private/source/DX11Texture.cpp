#include "win/DX11Texture.h"
#include "win/EffectDX11Wrapper.h"
#include <memory.h>

DX11Texture::DX11Texture(std::shared_ptr<FEffectDX11Wrapper> DX11Wrapper)
	:mDX11Wrapper(DX11Wrapper)
{
	m_texShaderView = NULL;
	m_pTexture = NULL;

	m_nWidth = 0;
	m_nHeight = 0;
}


DX11Texture::~DX11Texture()
{
	Destory();
}

bool DX11Texture::InitTexture(DXGI_FORMAT format, uint32_t BindFlags, int width, int height, uint32_t MiscFlag, uint32_t CpuFlag, bool MsAA)
{
	m_MSAA = MsAA;
	m_nWidth = width;
	m_nHeight = height;
	if (width*height == 0) {
		return false; 
	}

	TRefCountPtr<ID3D11Texture2D> CopyTexture;

	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(texDesc));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	if (m_MSAA)
	{
		texDesc.SampleDesc.Count = 4;
	}
	else
	{
		texDesc.SampleDesc.Count = 1;
	}
	
	texDesc.CPUAccessFlags = CpuFlag;
	if (CpuFlag == D3D11_CPU_ACCESS_WRITE)
	{
		texDesc.Usage = D3D11_USAGE_DEFAULT;
	}
	else if (CpuFlag == D3D11_CPU_ACCESS_READ)
	{
		texDesc.Usage = D3D11_USAGE_STAGING;
	}
	else
	{
		texDesc.Usage = D3D11_USAGE_DEFAULT;
	}
	
	texDesc.BindFlags = BindFlags;
	texDesc.MiscFlags = MiscFlag;

	hr = mDX11Wrapper->GetDevice()->CreateTexture2D(&texDesc, nullptr, m_pTexture.GetInitReference());
	if (FAILED(hr))
	{
		return false;
	}

	if (BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc;
		view_desc.Format = format;
		if (m_MSAA)
		{
			view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		}
		view_desc.Texture2D.MipLevels = texDesc.MipLevels;
		view_desc.Texture2D.MostDetailedMip = 0;
		hr = mDX11Wrapper->GetDevice()->CreateShaderResourceView(m_pTexture, &view_desc, m_texShaderView.GetInitReference());
		if (FAILED(hr))
		{
			return false;;
		}
	}

	if (BindFlags & D3D11_BIND_RENDER_TARGET)
	{
		HRESULT hr = mDX11Wrapper->GetDevice()->CreateRenderTargetView(m_pTexture, nullptr, m_RTV.GetInitReference());
		if (FAILED(hr))
		{
			//logger::err() << __FUNCTIONW__ L" failded to CreateShaderResourceView DXGI_FORMAT_R8G8B8A8_UNORM";
			return false;
		}
	}

	if ( (MiscFlag & D3D11_RESOURCE_MISC_SHARED) && m_pTexture)
	{
		TRefCountPtr<IDXGIResource> pOtherResource = nullptr;
		HRESULT handle_hr;
		handle_hr = m_pTexture->QueryInterface(__uuidof(IDXGIResource), (void**)pOtherResource.GetInitReference());
		if (SUCCEEDED(handle_hr))
		{
			handle_hr = pOtherResource->GetSharedHandle(&m_SharedHandle);
		}
	}

	return true;
}

bool DX11Texture::UpdateTextureInfo(void* pBuffer, int left, int top, int right, int bottom, int SrcRowPitch, int SrcDepthPitch)
{
	if (m_pTexture != NULL && pBuffer != NULL)
	{
		D3D11_BOX box;
		box.left = left;
		box.top = top;
		box.right = right;
		box.bottom = bottom;
		box.front = 0;
		box.back = 1;

		mDX11Wrapper->GetDeviceContext()->UpdateSubresource(m_pTexture, 0, &box, pBuffer, SrcRowPitch, SrcDepthPitch);
		return true;
	}
	return false;
}

bool DX11Texture::ReadTextureToCpu(void * pBuffer)
{
	if (m_pTexture != NULL && pBuffer != NULL)
	{
		D3D11_MAPPED_SUBRESOURCE mapData;
		mDX11Wrapper->GetDeviceContext()->Map(m_pTexture, 0, D3D11_MAP_READ, 0, &mapData);


		char* pdata = static_cast<char*>(mapData.pData);
		char* pbuf = static_cast<char*>(pBuffer);
		for (int i = 0; i < m_nHeight; i++) {
			memcpy(pbuf, pdata, m_nWidth * 4);
			pbuf += m_nWidth * 4;
			pdata += mapData.RowPitch;
		}
		mDX11Wrapper->GetDeviceContext()->Unmap(m_pTexture, 0);
		return true;
	}

	return false;
}

void DX11Texture::CopyResource(TRefCountPtr<ID3D11Texture2D> src)
{
	if (m_pTexture)
	{
		mDX11Wrapper->GetDeviceContext()->CopyResource(m_pTexture, src);
	}
}

void DX11Texture::ResloveResource(TRefCountPtr<ID3D11Texture2D> src)
{
	D3D11_TEXTURE2D_DESC texDesc;
	src->GetDesc(&texDesc);
	unsigned int sub = D3D11CalcSubresource(0, 0, 1);
	mDX11Wrapper->GetDeviceContext()->ResolveSubresource(m_pTexture, sub, src, sub, texDesc.Format);
}

bool DX11Texture::OpenResource(HANDLE SharedHandle)
{
	m_SharedHandle = SharedHandle;
	HRESULT hr = mDX11Wrapper->GetDevice()->OpenSharedResource(SharedHandle, __uuidof(ID3D11Resource), (void**)m_pTexture.GetInitReference());
	if (FAILED(hr)) {
		return false;
	}
	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(texDesc));
	m_pTexture->GetDesc(&texDesc);
	m_nWidth = texDesc.Width;
	m_nHeight = texDesc.Height;
	return true;
}

TRefCountPtr<ID3D11Texture2D> DX11Texture::GetTex()
{
	return m_pTexture;
}

TRefCountPtr<ID3D11ShaderResourceView> DX11Texture::GetTexShaderView()
{
	return m_texShaderView;
}

TRefCountPtr<ID3D11RenderTargetView> DX11Texture::GetRTV()
{
	return m_RTV;
}

HANDLE DX11Texture::GetSharedHandle()
{
	return m_SharedHandle;
}

int DX11Texture::GetWidth()
{
	return m_nWidth;
}
int DX11Texture::GetHeight()
{
	return m_nHeight;
}

void DX11Texture::Destory()
{
	m_nWidth = 0;
	m_nHeight = 0;

	m_texShaderView.SafeRelease();
	m_pTexture.SafeRelease();
}