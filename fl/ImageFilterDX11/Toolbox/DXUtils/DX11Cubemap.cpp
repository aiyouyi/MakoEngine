#include "DX11CubeMap.h"
#include "DX11Context.h"
#include "common.h"
#include "Toolbox/Helper.h"
#include <d3dx11.h>  
#include "DDS/DDSTextureLoader.h"
#include "CC3DEngine/Common/CC3DUtils.h"
DX11CubeMap::DX11CubeMap() :
	textureWidth(0),
	textureHeight(0),
	dsv(nullptr),
	depthTexture(nullptr),
	srv(nullptr),
	cubeMapTexture(nullptr)
{
	for (int i = 0; i < CCMaxMipMap; i++)
	{
		for (int index = 0; index < 6; ++index)
		{
			rtvs[index][i] = nullptr;
		}

	}

}


DX11CubeMap::~DX11CubeMap()
{
	destory();
}


DX11CubeMap::DX11CubeMap(int nTextureWidth, int nTexureHeight,int nMipLevels, bool UseDepth, bool GenMipRetarget) :
	textureWidth(nTextureWidth),
	textureHeight(nTexureHeight),
	dsv(nullptr),
	depthTexture(nullptr),
	srv(nullptr),
	cubeMapTexture(nullptr)
{
	for (int i = 0; i < CCMaxMipMap; i++)
	{
		for (int index = 0; index < 6; ++index)
		{
			rtvs[index][i] = nullptr;
		}

	}

	Init(nTextureWidth, nTexureHeight,nMipLevels,UseDepth,GenMipRetarget);
}


bool DX11CubeMap::Init(int nTextureWidth, int nTexureHeight,int nMipLevels, bool UseDepth, bool GenMipRetarget)
{
	textureWidth = nTextureWidth;
	textureHeight = nTexureHeight;
	
	//第一,填充2D纹理形容结构体,并创建2D渲染目标纹理
	D3D11_TEXTURE2D_DESC cubeMapTextureDesc;
	ZeroMemory(&cubeMapTextureDesc, sizeof(cubeMapTextureDesc));
	cubeMapTextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	cubeMapTextureDesc.Width = nTextureWidth;
	cubeMapTextureDesc.Height = nTexureHeight;
	cubeMapTextureDesc.MipLevels = 0;
	cubeMapTextureDesc.ArraySize = 6;
	cubeMapTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapTextureDesc.CPUAccessFlags = 0;
	cubeMapTextureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	cubeMapTextureDesc.SampleDesc.Count = 1;
	cubeMapTextureDesc.SampleDesc.Quality = 0;
	HRESULT hr = S_OK;
	hr = DevicePtr->CreateTexture2D(&cubeMapTextureDesc, NULL, &cubeMapTexture);
	if (FAILED(hr))
	{
		return false;
	}

	int nMipMap = 1;
	if (GenMipRetarget)
	{
		nMipMap = (std::min)(CCMaxMipMap, (std::max)(1, nMipLevels));;
	}
		
	for (int i=0;i<nMipMap;i++)
	{
		//第二，填充渲染目标视图形容体,并进行创建目标渲染视图
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = cubeMapTextureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		renderTargetViewDesc.Texture2D.MipSlice = i;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;

		for (int index = 0; index < 6; ++index)
		{
			renderTargetViewDesc.Texture2DArray.FirstArraySlice = index;
			hr = DevicePtr->CreateRenderTargetView(cubeMapTexture, &renderTargetViewDesc, &rtvs[index][i]);
			if (FAILED(hr))
			{
				return false;
			}
		}
	}


	//第三,创建着色器资源视图
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = cubeMapTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = nMipLevels;
	hr = DevicePtr->CreateShaderResourceView(cubeMapTexture, &shaderResourceViewDesc, &srv);
	if (FAILED(hr))
	{
		return false;
	}

	if (UseDepth)
	{
		//第四,创建深度缓存(模板缓存)
		D3D11_TEXTURE2D_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.Width = nTextureWidth;
		depthStencilDesc.Height = nTexureHeight;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		hr = DevicePtr->CreateTexture2D(&depthStencilDesc, 0, &depthTexture);
		if (FAILED(hr))
		{
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		hr = DevicePtr->CreateDepthStencilView(depthTexture, &depthStencilViewDesc, &dsv);
		if (FAILED(hr))
		{
			return false;
		}
	}



	viewport.Width = static_cast<float>(nTextureWidth);
	viewport.Height = static_cast<float>(nTexureHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	return true;

}


void DX11CubeMap::SetRenderTarget(int targetViewSlot, int nMip)
{
	//绑定渲染目标视图和深度模板视图到输出渲染管线，此时渲染输出到两张纹理中
	DeviceContextPtr->OMSetRenderTargets(1, &rtvs[targetViewSlot][nMip], dsv);

	//设置相应的视口
	DeviceContextPtr->RSSetViewports(1, &viewport);
}

void DX11CubeMap::ClearDepthBuffer()
{
	//清除深度缓存和模板缓存
	DeviceContextPtr->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DX11CubeMap::ClearRenderTarget(float fRed, float fGreen, float fBlue, float fAlpha)
{
	//设置清除缓存为的颜色
	float color[4];
	color[0] = fRed;
	color[1] = fGreen;
	color[2] = fBlue;
	color[3] = fAlpha;

	for (int index = 0; index < 6; ++index)
	{
		DeviceContextPtr->ClearRenderTargetView(rtvs[index][0], color);
	}
}

void DX11CubeMap::destory()
{
	SAFE_RELEASE_BUFFER(srv);
	SAFE_RELEASE_BUFFER(cubeMapTexture);

	for (int i = 0; i < CCMaxMipMap; i++)
	{
		for (int index = 0; index < 6; ++index)
		{
			SAFE_RELEASE_BUFFER(rtvs[index][i]);
		}

	}

	SAFE_RELEASE_BUFFER(depthTexture);
	SAFE_RELEASE_BUFFER(dsv);
}