#include "DX11SwapChainRHI.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/DXUtils/DX11DepthBuffer.h"
#include "common.h"
#include "Toolbox/Render/DynamicRHI.h"

DX11SwapChainRHI::DX11SwapChainRHI()
{

}

DX11SwapChainRHI::~DX11SwapChainRHI()
{
	
}


bool DX11SwapChainRHI::CreateSwapChain(void* hWnd, bool enableDepth, uint32_t width, uint32_t height)
{
	EnableDepth = enableDepth;
	WndHandle = static_cast<HWND>(hWnd);

	TargetViewSize = { width,height };

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = TargetViewSize.w;
	sd.BufferDesc.Height = TargetViewSize.h;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = WndHandle;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;


	HRESULT hr = E_FAIL;
	IRefPtr<IDXGIDevice> dxgiDevice;
	if (FAILED(hr = DevicePtr->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice)))
	{
		return false;
	}

	IRefPtr<IDXGIAdapter> dxgiAdapter;
	if (FAILED(hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter)))
	{
		return false;
	}

	IRefPtr<IDXGIFactory> dxgiFactory;
	if (FAILED(hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
	{
		return false;
	}

	if (FAILED(hr = dxgiFactory->CreateSwapChain(DevicePtr, &sd, &SwapChain)))
	{
		return false;
	}

	IRefPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
	{
		return false;
	}

	if (FAILED(hr = DevicePtr->CreateRenderTargetView(backBuffer, NULL, &RenderTargetView)))
	{
		return false;
	}

	if (enableDepth)
	{
		DepthBuffer = std::make_shared<DX11DepthBuffer>();
		if (!DepthBuffer->Create(width,height,1,0))
		{
			return false;
		}
	}

	GetDynamicRHI()->SetViewPort(0, 0, width, height);
	SetRenderTarget();

	return true;
}

void DX11SwapChainRHI::SetRenderTarget(void)
{
	if (!DeviceContextPtr)
	{
		return;
	}

	if (!RenderTargetView)
	{
		return;
	}

	if (EnableDepth)
	{
		if (!DepthBuffer)
		{
			return;
		}
		DeviceContextPtr->OMSetRenderTargets(1, &RenderTargetView, DepthBuffer->GetDSV());
	}
	else
	{
		DeviceContextPtr->OMSetRenderTargets(1, &RenderTargetView, nullptr);
	}

}

void DX11SwapChainRHI::Present()
{
	if (SwapChain)
	{
		SwapChain->Present(0, 0);
	}
	
}

bool DX11SwapChainRHI::ReSize(uint32_t width, uint32_t height)
{
	if (!SwapChain.IsValidRefObj())
	{
		return false;
	}
	TargetViewSize = { width,height };
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	DeviceContextPtr->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);

	RenderTargetView.ReleaseRefObj();
	DepthBuffer.reset();
	HRESULT hr = SwapChain->ResizeBuffers(1, TargetViewSize.cx, TargetViewSize.cy, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr))
	{
		return false;
	}
	if (EnableDepth)
	{
		DepthBuffer = std::make_shared<DX11DepthBuffer>();
		if (!DepthBuffer->Create(width, height, 1, 0))
		{
			return false;
		}
	}

	IRefPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
	{
		return false;
	}

	if (FAILED(hr = DevicePtr->CreateRenderTargetView(backBuffer, NULL, &RenderTargetView)))
	{
		return false;
	}

	GetDynamicRHI()->SetViewPort(0, 0, width, height);
	SetRenderTarget();

	return true;
}

void DX11SwapChainRHI::Clear(float r, float g, float b, float a)
{
	float ClearColor[4] = { r, g, b, a }; // rgba  
	DeviceContextPtr->ClearRenderTargetView(RenderTargetView, ClearColor);
	if (DepthBuffer)
	{
		DeviceContextPtr->ClearDepthStencilView(DepthBuffer->GetDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	
}
