#include "DX11FBO.h"
#include "DX11Context.h"
#include "Toolbox/Helper.h"

DX11FBO::DX11FBO()
{
	m_nWidth = 0;
	m_nHeight = 0;

	m_pTexture = NULL;
	m_pDestTexture = NULL;

	m_pRenderTargetView = NULL;
	m_pDepthStencilView = NULL;

	m_nlockCount = 0;
}


DX11FBO::~DX11FBO()
{
	destory();
}

bool DX11FBO::initWithTexture(int width, int height, bool useDepthBuffer, ID3D11Texture2D *pDestTexture, DXGI_FORMAT format,bool bMSAA)
{
	m_nWidth = width;
	m_nHeight = height;

	HRESULT hr = S_OK;
	DX11Texture *pTexture = NULL;
	if (pDestTexture == NULL)
	{
		pTexture = new DX11Texture();
		pTexture->initTexture(format, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, width, height,NULL,0,false,bMSAA);
		pDestTexture = pTexture->getTex();
	}
	m_pTexture = pTexture;

	if (pDestTexture != NULL)
	{
		pDestTexture->AddRef();
	}
	m_pDestTexture = pDestTexture;


	D3D11_TEXTURE2D_DESC texDesc;
	m_pDestTexture->GetDesc(&texDesc);

	//为渲染纹理创建targetview
	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	memset(&rtvd, 0, sizeof(rtvd));
	rtvd.Format = texDesc.Format;
	if (texDesc.SampleDesc.Count > 1)
	{
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	}
	rtvd.Texture2D.MipSlice = 0;
	hr = DevicePtr->CreateRenderTargetView(m_pDestTexture, &rtvd, &m_pRenderTargetView);
	if (FAILED(hr))
	{
		return false;
	}

	if (useDepthBuffer)
	{
		//创建深度模板缓冲区
		D3D11_TEXTURE2D_DESC depthStencil_desc;
		memset(&depthStencil_desc, 0, sizeof(D3D11_TEXTURE2D_DESC));
		memset(&depthStencil_desc, 0, sizeof(depthStencil_desc));
		depthStencil_desc.ArraySize = 1;
		depthStencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencil_desc.CPUAccessFlags = 0;
		depthStencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencil_desc.Width = width;
		depthStencil_desc.Height = height;
		depthStencil_desc.MipLevels = 1;
		depthStencil_desc.MiscFlags = 0;
		depthStencil_desc.Usage = D3D11_USAGE_DEFAULT;
		depthStencil_desc.SampleDesc.Count = texDesc.SampleDesc.Count;
		depthStencil_desc.SampleDesc.Quality = texDesc.SampleDesc.Quality;
		ID3D11Texture2D *pTextureDepthStencil = NULL;
		DevicePtr->CreateTexture2D(&depthStencil_desc, NULL, &pTextureDepthStencil);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		memset(&dsv_desc, 0, sizeof(dsv_desc));
		dsv_desc.Format = depthStencil_desc.Format;
		if (texDesc.SampleDesc.Count > 1)
		{
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}
		
		dsv_desc.Texture2D.MipSlice = 0;
		DevicePtr->CreateDepthStencilView(pTextureDepthStencil, &dsv_desc, &m_pDepthStencilView);
		pTextureDepthStencil->Release();
	}

	return true;
}

bool DX11FBO::UpdateWithTextue(ID3D11Texture2D * pDestTexture)
{
	return false;
}

void DX11FBO::bind()
{
	if (m_pRenderTargetView != NULL)
	{
		DeviceContextPtr->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)m_nWidth;
		vp.Height = (FLOAT)m_nHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		DeviceContextPtr->RSSetViewports(1, &vp);
	}
}

void DX11FBO::unBind()
{
	//ID3D11RenderTargetView* pNullRTV = NULL;
	//DeviceContextPtr->OMSetRenderTargets(1, &pNullRTV, NULL);
}

void DX11FBO::clear(float r, float g, float b, float a, float depth, unsigned char Stencil)
{
	float arrClear[] = { r,g,b,a };

	if (m_pRenderTargetView != NULL)
	{
		DeviceContextPtr->ClearRenderTargetView(m_pRenderTargetView, arrClear);
	}

	if (m_pDepthStencilView != NULL)
	{
		DeviceContextPtr->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, depth, Stencil);
	}
}

void DX11FBO::clearDepth(float depth, unsigned char Stencil)
{
	if (m_pDepthStencilView != NULL)
	{
		DeviceContextPtr->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, depth, Stencil);
	}
}

DX11Texture *DX11FBO::getTexture()
{
	return m_pTexture;
}

ID3D11RenderTargetView *DX11FBO::getRenderTargetView()
{
	return m_pRenderTargetView;
}

ID3D11DepthStencilView *DX11FBO::getDepthStencilView()
{
	return m_pDepthStencilView;
}

void DX11FBO::destory()
{
	SAFERALEASE(m_pRenderTargetView);
	SAFERALEASE(m_pDepthStencilView);
	SAFERALEASE(m_pDestTexture);
	SAFEDEL(m_pTexture);
}

void DX11FBO::lock()
{
	m_nlockCount++;
}

void DX11FBO::unlock()
{
	m_nlockCount--;
	if (m_nlockCount <= 0)
	{
		FBOCacheInst->returnFrameBufferToCache(this);
	}
}

void DX11FBO::resetLockCount()
{
	m_nlockCount = 0;
}