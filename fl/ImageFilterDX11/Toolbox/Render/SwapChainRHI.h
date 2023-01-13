#pragma once
#include "Toolbox/inc.h"

class SwapChainRHI
{
public:
	SwapChainRHI();
	virtual ~SwapChainRHI();

	virtual bool CreateSwapChain(void* hWnd, bool enableDepth, uint32_t width, uint32_t height) = 0;
	virtual void SetRenderTarget() = 0;
	virtual void Clear(float r, float g, float b, float a) = 0;
	virtual void Present() = 0;
	virtual bool ReSize(uint32_t width, uint32_t height) = 0;
};