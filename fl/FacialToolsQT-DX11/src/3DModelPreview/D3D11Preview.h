#pragma once
#include "BasicPreview.h"

class CC3DTextureRHI;
class RectDraw;
class SwapChainRHI;

class D3D11Preview : public BasicPreivew
{
	Q_OBJECT
public:
	D3D11Preview(QObject* parent = nullptr);
	~D3D11Preview();
	void ReSize(uint32_t width, uint32_t height);

private:
	virtual bool Init() override;
	virtual void UnInit() override;
	virtual void DoRender() override;
	virtual void DoPrepare() override;

private:
	std::shared_ptr<CC3DTextureRHI> m_renderTargetTex;
	std::shared_ptr<RectDraw> m_rectDraw;
	std::shared_ptr<SwapChainRHI> m_SwapChain;
	std::shared_ptr<class CC3DRasterizerState> m_Rasterizer;
	std::shared_ptr<class CC3DBlendState> m_BlendState;
	std::shared_ptr<class CC3DDepthStencilState> m_DepthStencialState;
	QSize m_CurrentSize;
};