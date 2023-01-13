#include "DX11DepthBuffer.h"
#include "Toolbox/DXUtils/DX11Context.h"

DX11DepthBuffer::DX11DepthBuffer()
{

}

DX11DepthBuffer::~DX11DepthBuffer()
{

}

bool DX11DepthBuffer::Create(uint32_t sizeW, uint32_t sizeH, int sampleCount, int quality)
{
	Size = { sizeW,sizeH };
	//DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//D3D11_TEXTURE2D_DESC dsDesc;
	//dsDesc.Width = sizeW;
	//dsDesc.Height = sizeH;
	//dsDesc.MipLevels = 1;
	//dsDesc.ArraySize = 1;
	//dsDesc.Format = format;
	//dsDesc.SampleDesc.Count = sampleCount;
	//dsDesc.SampleDesc.Quality = quality;
	//dsDesc.Usage = D3D11_USAGE_DEFAULT;
	//dsDesc.CPUAccessFlags = 0;
	//dsDesc.MiscFlags = 0;
	//dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//DevicePtr->CreateTexture2D(&dsDesc, nullptr, Tex.ReleaseAndGetAddressOf());

	//D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	//memset(&dsv_desc, 0, sizeof(dsv_desc));
	//dsv_desc.Format = dsDesc.Format;
	//dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//dsv_desc.Texture2D.MipSlice = 0;

	//DevicePtr->CreateDepthStencilView(Tex, &dsv_desc, TexDsv.ReleaseAndGetAddressOf());

	D3D11_TEXTURE2D_DESC depthStencil_desc;
	memset(&depthStencil_desc, 0, sizeof(D3D11_TEXTURE2D_DESC));
	depthStencil_desc.ArraySize = 1;
	depthStencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencil_desc.CPUAccessFlags = 0;
	depthStencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencil_desc.Width = Size.cx;
	depthStencil_desc.Height = Size.cy;
	depthStencil_desc.MipLevels = 1;
	depthStencil_desc.MiscFlags = 0;
	depthStencil_desc.Usage = D3D11_USAGE_DEFAULT;
	depthStencil_desc.SampleDesc.Count = sampleCount;
	depthStencil_desc.SampleDesc.Quality = quality;

	DevicePtr->CreateTexture2D(&depthStencil_desc, nullptr, Tex.ReleaseAndGetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	memset(&dsv_desc, 0, sizeof(dsv_desc));
	dsv_desc.Format = depthStencil_desc.Format;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Texture2D.MipSlice = 0;
	DevicePtr->CreateDepthStencilView(Tex, &dsv_desc, TexDsv.ReleaseAndGetAddressOf());

	return Tex.IsValidRefObj() && TexDsv.IsValidRefObj();
}


bool DX11DepthBuffer::Create32Float(uint32_t sizeW, uint32_t sizeH, bool Msaa)
{
	//创建深度模板缓冲区
	D3D11_TEXTURE2D_DESC depthStencil_desc;
	memset(&depthStencil_desc, 0, sizeof(D3D11_TEXTURE2D_DESC));
	memset(&depthStencil_desc, 0, sizeof(depthStencil_desc));
	depthStencil_desc.ArraySize = 1;
	depthStencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencil_desc.CPUAccessFlags = 0;
	depthStencil_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthStencil_desc.Width = sizeW;
	depthStencil_desc.Height = sizeH;
	depthStencil_desc.MipLevels = 1;
	depthStencil_desc.MiscFlags = 0;
	depthStencil_desc.Usage = D3D11_USAGE_DEFAULT;
	depthStencil_desc.SampleDesc.Quality = 0;
	if (Msaa)
	{
		depthStencil_desc.SampleDesc.Count = 4;
		
	}
	else
	{
		depthStencil_desc.SampleDesc.Count = 1;
	}
	HRESULT hr = DevicePtr->CreateTexture2D(&depthStencil_desc, nullptr , &Tex);
	if (FAILED(hr))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	memset(&dsv_desc, 0, sizeof(dsv_desc));
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	if (Msaa)
	{
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	}

	dsv_desc.Texture2D.MipSlice = 0;
	hr = DevicePtr->CreateDepthStencilView(Tex, &dsv_desc, TexDsv.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC SRDesc;
	SRDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRDesc.ViewDimension = Msaa ? (D3D11_SRV_DIMENSION_TEXTURE2DMS) : (D3D11_SRV_DIMENSION_TEXTURE2D);
	SRDesc.Texture2D.MostDetailedMip = 0;
	SRDesc.Texture2D.MipLevels = 1;

	hr = DevicePtr->CreateShaderResourceView(Tex, &SRDesc, DepthStencilSRV.ReleaseAndGetAddressOf());
	return hr == S_OK;
}

IRefPtr<ID3D11DepthStencilView> DX11DepthBuffer::GetDSV()
{
	return TexDsv;
}

IRefPtr<ID3D11ShaderResourceView> DX11DepthBuffer::GetDepthSRV()
{
	return DepthStencilSRV;
}
