#include "D3D11Preview.h"
#include <QFrame>
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/SwapChainRHI.h"
#include "Toolbox/Render/TextureRHI.h"
#include "ModelRenderLogic.h"
#include "Toolbox/RectDraw.h"

D3D11Preview::D3D11Preview(QObject* parent /*= nullptr*/)
	:BasicPreivew(parent)
{

}

D3D11Preview::~D3D11Preview()
{

}

void D3D11Preview::ReSize(uint32_t width, uint32_t height)
{
	m_SwapChain->ReSize(width, height);
}

bool D3D11Preview::Init()
{
	m_SwapChain = GetDynamicRHI()->CreateSwapChain();
	if (GetDynamicRHI()->CreateDevice(true))
	{
		bool Result = m_SwapChain->CreateSwapChain(m_WndId, true, m_FrameRect.width(), m_FrameRect.height());
		if (!Result)
		{
			return false;
		}
	}

	m_Rasterizer = GetDynamicRHI()->CreateRasterizerState(CC3DRasterizerState::CT_NONE);
	m_BlendState = GetDynamicRHI()->CreateBlendState(true, false, true, false);
	m_DepthStencialState = GetDynamicRHI()->CreateDefaultStencilState(false, true);
	m_renderTargetTex = GetDynamicRHI()->CreateTexture();
	m_renderTargetTex->InitTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_RENDER_TARGET, m_FrameRect.width(), m_FrameRect.height());

	m_rectDraw = std::make_shared<RectDraw>();
	m_rectDraw->init(1, 1, "");
	m_CurrentSize = m_FrameRect.size();

	return true;
}

void D3D11Preview::UnInit()
{
	BasicPreivew::UnInit();

	m_SwapChain = {};
	m_Rasterizer = {};
	m_BlendState = {};
	m_DepthStencialState = {};

}

void D3D11Preview::DoRender()
{
	std::lock_guard<std::mutex> lock(m_SizeLock);
	if (m_Resize)
	{
		ReSize(m_FrameRect.width(), m_FrameRect.height());
		m_Resize = false;
		m_renderTargetTex = GetDynamicRHI()->CreateTexture();
		m_renderTargetTex->InitTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_RENDER_TARGET, m_FrameRect.width(), m_FrameRect.height());
	}

	if (m_RenderLogic)
	{
		m_RenderLogic->DoRender(m_FrameRect.size(), m_renderTargetTex);
	}
	
	if (m_rectDraw)
	{
		m_rectDraw->setShaderTextureView(m_renderTargetTex);
	}
	

	GetDynamicRHI()->SetViewPort(0, 0, m_FrameRect.width(), m_FrameRect.height());
	m_SwapChain->SetRenderTarget();
	m_SwapChain->Clear(0, 0, 0, 1);

	float blendFactor[] = { 0.f,0.f,0.f,0.f };

	GetDynamicRHI()->SetBlendState(m_BlendState, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetRasterizerState(m_Rasterizer);
	GetDynamicRHI()->SetDepthStencilState(m_DepthStencialState);
	if (m_rectDraw)
	{

		m_rectDraw->render(Vector2(0, 0), Vector2(1, 1), 0, m_FrameRect.width(), m_FrameRect.height());
	}
	

	m_SwapChain->Present();
}

void D3D11Preview::DoPrepare()
{
	BasicPreivew::DoPrepare();
}
