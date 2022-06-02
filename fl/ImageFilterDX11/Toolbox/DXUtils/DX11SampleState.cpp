#include "DX11SampleState.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/Helper.h"

DWORD ms_dwTextureFilterType[CC3DSamplerState::FM_MAX][CC3DSamplerState::FM_MAX][CC3DSamplerState::FM_MAX] =
{
	D3D11_FILTER_MIN_MAG_MIP_POINT,	// none none none
	D3D11_FILTER_MIN_MAG_MIP_POINT,	// none none point
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // none none line
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // none none anisotropic

	D3D11_FILTER_MIN_MAG_MIP_POINT,// none point none
	D3D11_FILTER_MIN_MAG_MIP_POINT,// none point point
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // none point line
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // none point anisotropic

	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,// none line none
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,// none line point
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, // none line line
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, // none line anisotropic

	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, //none, anisotropic, none
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,	//none, anisotropic, point
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, // none anisotropic line
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, // none anisotropic anisotropic



	D3D11_FILTER_MIN_MAG_MIP_POINT, //point, none, none
	D3D11_FILTER_MIN_MAG_MIP_POINT,	//point, none, point
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // point none line
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // point none anisotropic

	D3D11_FILTER_MIN_MAG_MIP_POINT, //point, point, none
	D3D11_FILTER_MIN_MAG_MIP_POINT,	//point, point, point
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // point point line
	D3D11_FILTER_MIN_MAG_MIP_POINT, // point point anisotropic

	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, //point, line, none
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,	//point, line, point
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, // point line line
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, // point line anisotropic

	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, //point, anisotropic, none
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,	//point, anisotropic, point
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, // point anisotropic line
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, // point anisotropic anisotropic



	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT, //line, none, none
	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,	//line, none, point
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, // line none line
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, // line none anisotropic

	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT, //line, point, none
	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,	//line, point, point
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, // line point line
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, // line point anisotropic

	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, //line, line, none
	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,	//line, line, point
	D3D11_FILTER_MIN_MAG_MIP_LINEAR, // line line line
	D3D11_FILTER_MIN_MAG_MIP_LINEAR, // line line anisotropic

	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, //line, anisotropic, none
	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,	//line, anisotropic, point
	D3D11_FILTER_MIN_MAG_MIP_LINEAR, // line anisotropic line
	D3D11_FILTER_MIN_MAG_MIP_LINEAR, // line anisotropic anisotropic


	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT, //anisotropic, none, none
	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,	//anisotropic, none, point
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, // anisotropic none line
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, // anisotropic none anisotropic

	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT, //anisotropic, point, none
	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,	//anisotropic, point, point
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, // anisotropic point line
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, // anisotropic point anisotropic

	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, //anisotropic, line, none
	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,	//anisotropic, line, point
	D3D11_FILTER_MIN_MAG_MIP_LINEAR, // anisotropic line line
	D3D11_FILTER_MIN_MAG_MIP_LINEAR, // anisotropic line anisotropic

	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, //anisotropic, anisotropic, none
	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,	//anisotropic, anisotropic, point
	D3D11_FILTER_MIN_MAG_MIP_LINEAR, // anisotropic anisotropic line
	D3D11_FILTER_ANISOTROPIC, // anisotropic anisotropic anisotropic

};
DWORD ms_dwTextureAddrType[] =
{
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_MIRROR,
	D3D11_TEXTURE_ADDRESS_BORDER,
};


DX11SampleState::DX11SampleState()
{

}

DX11SampleState::~DX11SampleState()
{
	SAFERALEASE(PtrSamplerState);
}

void DX11SampleState::CreateState()
{
	D3D11_SAMPLER_DESC sampDesc;

	sampDesc.Filter = (D3D11_FILTER)ms_dwTextureFilterType[m_uiMin][m_uiMag][m_uiMip];
	sampDesc.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)ms_dwTextureAddrType[m_uiCoordU];
	sampDesc.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)ms_dwTextureAddrType[m_uiCoordV];
	sampDesc.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)ms_dwTextureAddrType[m_uiCoordW];
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = m_uiMipLevel * 1.0f;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[0] = m_BorderColor.x;
	sampDesc.BorderColor[1] = m_BorderColor.y;
	sampDesc.BorderColor[2] = m_BorderColor.z;
	sampDesc.BorderColor[3] = m_BorderColor.w;
	sampDesc.MaxAnisotropy = m_uiAniLevel;
	sampDesc.MipLODBias = m_MipLODBias;

	HRESULT hr = DevicePtr->CreateSamplerState(&sampDesc, &PtrSamplerState);
}
