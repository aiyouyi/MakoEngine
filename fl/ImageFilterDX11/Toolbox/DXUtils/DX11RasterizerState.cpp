#include "DX11RasterizerState.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/Helper.h"

DWORD ms_dwCullType[] =
{
	D3D11_CULL_NONE,
	D3D11_CULL_FRONT,
	D3D11_CULL_BACK
};

DX11RasterizerState::DX11RasterizerState()
{

}

DX11RasterizerState::~DX11RasterizerState()
{
	SAFERALEASE(PtrRasterizerState);
}

void DX11RasterizerState::CreateState()
{
	D3D11_RASTERIZER_DESC rasterDesc;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = (D3D11_CULL_MODE)ms_dwCullType[m_uiCullType];
	rasterDesc.DepthBias = (int)m_fDepthBias;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = (m_bWireEnable) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = m_bScissorTestEnable;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	HRESULT hr = DevicePtr->CreateRasterizerState(&rasterDesc, &PtrRasterizerState);
}

