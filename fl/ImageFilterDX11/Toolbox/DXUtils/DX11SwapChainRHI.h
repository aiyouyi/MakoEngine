#pragma once
#include "Toolbox/Render/SwapChainRHI.h"
#include "Toolbox/irefptr.h"
#include "Toolbox/vec2.h"
#include <d3d11.h>

class DX11DepthBuffer;

class DX11SwapChainRHI : public SwapChainRHI
{
public:
	DX11SwapChainRHI();
	virtual ~DX11SwapChainRHI();

	bool CreateSwapChain(void* hWnd, bool enableDepth, uint32_t width, uint32_t height) override;
	void SetRenderTarget() override;
	void Present()override;
	bool ReSize(uint32_t width, uint32_t height) override;
	void Clear(float r, float g, float b, float a) override;

private:
	IRefPtr<IDXGISwapChain>			SwapChain;
	IRefPtr<ID3D11RenderTargetView> RenderTargetView;
	
	HWND							WndHandle = nullptr;
	core::vec2u						TargetViewSize;
	bool							EnableDepth = false;
	std::shared_ptr<DX11DepthBuffer> DepthBuffer;
};