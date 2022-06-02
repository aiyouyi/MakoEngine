#include "DX11BlendState.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/Helper.h"

DWORD ms_dwAlphaBlend[] =
{
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
	D3D11_BLEND_DEST_ALPHA,
	D3D11_BLEND_INV_DEST_ALPHA,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR

};
DWORD ms_dwAlphaBlendOP[] =
{
	D3D11_BLEND_OP_ADD,
	D3D11_BLEND_OP_SUBTRACT,
	D3D11_BLEND_OP_REV_SUBTRACT,
	D3D11_BLEND_OP_MIN,
	D3D11_BLEND_OP_MAX
};

DX11BlendState::DX11BlendState()
{

}

DX11BlendState::~DX11BlendState()
{
	SAFERALEASE(PtrBlendState)
}

void DX11BlendState::CreateState()
{
	D3D11_BLEND_DESC desc = {};

	desc.AlphaToCoverageEnable = bAlphaToCoverageEnable;
	desc.IndependentBlendEnable = bIndependentBlendEnable;
	for (unsigned int i = 0; i < 8; ++i)
	{
		D3D11_RENDER_TARGET_BLEND_DESC& out = desc.RenderTarget[i];
		out.BlendEnable = bBlendEnable[i];
		out.SrcBlend = (D3D11_BLEND)ms_dwAlphaBlend[ucSrcBlend[i]];
		out.DestBlend = (D3D11_BLEND)ms_dwAlphaBlend[ucDestBlend[i]];
		out.BlendOp = (D3D11_BLEND_OP)ms_dwAlphaBlendOP[ucBlendOp[i]];
		out.SrcBlendAlpha = (D3D11_BLEND)ms_dwAlphaBlend[ucSrcBlendAlpha[i]];
		out.DestBlendAlpha = (D3D11_BLEND)ms_dwAlphaBlend[ucDestBlendAlpha[i]];
		out.BlendOpAlpha = (D3D11_BLEND_OP)ms_dwAlphaBlendOP[ucBlendOpAlpha[i]];
		out.RenderTargetWriteMask = ucWriteMask[i];
	}

	DevicePtr->CreateBlendState(&desc, &PtrBlendState);
}

void DX11BlendState::CreateState(bool bBlend, bool bBlendAlpha, bool writeBuffer)
{
	PtrBlendState = ContextInst->fetchBlendState(bBlend, bBlendAlpha, writeBuffer);
}
