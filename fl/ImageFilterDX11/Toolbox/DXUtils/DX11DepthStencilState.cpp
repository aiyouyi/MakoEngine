#include "DX11DepthStencilState.h"
#include "Toolbox/Helper.h"
#include "Toolbox/DXUtils/DX11Context.h"

DWORD ms_dwStencilOperation[] =
{
	D3D11_STENCIL_OP_KEEP,
	D3D11_STENCIL_OP_ZERO,
	D3D11_STENCIL_OP_REPLACE,
	D3D11_STENCIL_OP_INCR,
	D3D11_STENCIL_OP_DECR,
	D3D11_STENCIL_OP_INVERT,
	D3D11_STENCIL_OP_INCR_SAT,
	D3D11_STENCIL_OP_DECR_SAT
};
DWORD ms_dwCompare[] =
{
	D3D11_COMPARISON_NEVER,
	D3D11_COMPARISON_LESS,
	D3D11_COMPARISON_EQUAL,
	D3D11_COMPARISON_LESS_EQUAL,
	D3D11_COMPARISON_GREATER,
	D3D11_COMPARISON_NOT_EQUAL,
	D3D11_COMPARISON_GREATER_EQUAL,
	D3D11_COMPARISON_ALWAYS
};


DX11DepthStencilState::DX11DepthStencilState()
{

}

DX11DepthStencilState::~DX11DepthStencilState()
{
	SAFERALEASE(PtrDepthState)
}

void DX11DepthStencilState::CreateState()
{

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	depthStencilDesc.DepthEnable = m_bDepthEnable;
	depthStencilDesc.DepthWriteMask = m_bDepthWritable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = (D3D11_COMPARISON_FUNC)ms_dwCompare[m_uiDepthCompareMethod];

	depthStencilDesc.StencilEnable = m_bStencilEnable;
	depthStencilDesc.StencilReadMask = m_uiMask;
	depthStencilDesc.StencilWriteMask = m_uiWriteMask;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)ms_dwStencilOperation[m_uiSFailZPassOP];
	depthStencilDesc.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)ms_dwStencilOperation[m_uiSPassZFailOP];
	depthStencilDesc.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)ms_dwStencilOperation[m_uiSPassZPassOP];
	depthStencilDesc.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)ms_dwCompare[m_uiStencilCompareMethod];

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = (D3D11_STENCIL_OP)ms_dwStencilOperation[m_uiCCW_SFailZPassOP];
	depthStencilDesc.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)ms_dwStencilOperation[m_uiCCW_SPassZFailOP];
	depthStencilDesc.BackFace.StencilPassOp = (D3D11_STENCIL_OP)ms_dwStencilOperation[m_uiCCW_SPassZPassOP];
	depthStencilDesc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)ms_dwCompare[m_uiCCW_StencilCompareMethod];

	HRESULT hResult = NULL;
	hResult = DevicePtr->CreateDepthStencilState(&depthStencilDesc, &PtrDepthState);
}
