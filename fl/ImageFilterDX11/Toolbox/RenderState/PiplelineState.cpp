#include "PiplelineState.h"
#include "Toolbox/Helper.h"

std::shared_ptr<CC3DSamplerState> CC3DPiplelineState::ClampLinerSampler ;
std::shared_ptr<CC3DSamplerState> CC3DPiplelineState::ShadowSampler ;
std::shared_ptr<CC3DSamplerState> CC3DPiplelineState::WarpLinerSampler ;
std::shared_ptr<CC3DSamplerState> CC3DPiplelineState::MirrorLinerSampler;
std::shared_ptr<CC3DBlendState> CC3DPiplelineState::BlendDisable;
std::shared_ptr<CC3DBlendState> CC3DPiplelineState::BlendAlphaOn;
std::shared_ptr<CC3DBlendState> CC3DPiplelineState::BlendAlphaOff ;
std::shared_ptr<CC3DBlendState> CC3DPiplelineState::BSDisableWriteDisable;
std::shared_ptr<CC3DBlendState> CC3DPiplelineState::BlendDisableAlphaWriteDisable;
std::shared_ptr<CC3DDepthStencilState> CC3DPiplelineState::DepthStateDisable;
std::shared_ptr<CC3DDepthStencilState> CC3DPiplelineState::DepthStateEnable;
std::shared_ptr<CC3DDepthStencilState> CC3DPiplelineState::DepthStateWriteEnable;
std::shared_ptr<CC3DDepthStencilState> CC3DPiplelineState::DepthStateEnableWriteDisable;
std::shared_ptr<CC3DDepthStencilState> CC3DPiplelineState::DepthStateWriteDisable;
std::shared_ptr<CC3DRasterizerState> CC3DPiplelineState::RasterizerStateCullNone;
std::shared_ptr<CC3DRasterizerState> CC3DPiplelineState::RasterizerStateCullBack;
std::shared_ptr<CC3DRasterizerState> CC3DPiplelineState::RasterizerStateCullFront;

CC3DPiplelineState::CC3DPiplelineState()
{

}

CC3DPiplelineState::~CC3DPiplelineState()
{

}

void CC3DPiplelineState::Initialize()
{
	ClampLinerSampler = GetDynamicRHI()->CreateSampler();
	WarpLinerSampler = GetDynamicRHI()->CreateSampler(CC3DSamplerState::CM_WRAP);
	MirrorLinerSampler = GetDynamicRHI()->CreateSampler(CC3DSamplerState::CM_MIRROR);
	ShadowSampler = GetDynamicRHI()->CreateSampler(CC3DSamplerState::CM_BORDER, CC3DSamplerState::FM_POINT, CC3DSamplerState::FM_POINT, CC3DSamplerState::FM_POINT);
	BlendDisable = GetDynamicRHI()->CreateBlendState(false, false, true);
	BlendAlphaOff = GetDynamicRHI()->CreateBlendState(true,false,true);
	BlendDisableAlphaWriteDisable = GetDynamicRHI()->CreateBlendState(false, false, true, true); //true 表示只写入RGB
	BlendAlphaOn = GetDynamicRHI()->CreateBlendState(true, true, true);
	
	DepthStateDisable = GetDynamicRHI()->CreateDefaultStencilState(false,false);
	DepthStateEnable = GetDynamicRHI()->CreateDefaultStencilState(true, true);
	DepthStateEnableWriteDisable = GetDynamicRHI()->CreateDefaultStencilState(true, false);
	DepthStateWriteEnable = DepthStateEnable;
	DepthStateWriteDisable = DepthStateDisable;
	RasterizerStateCullNone = GetDynamicRHI()->CreateRasterizerState(CC3DRasterizerState::CT_NONE);
	RasterizerStateCullBack = GetDynamicRHI()->CreateRasterizerState(CC3DRasterizerState::CT_CW);
	RasterizerStateCullFront = GetDynamicRHI()->CreateRasterizerState(CC3DRasterizerState::CT_CCW);

	BSDisableWriteDisable = GetDynamicRHI()->CreateBlendState(false, false, false);
}

void CC3DPiplelineState::DestroyAll()
{
	ClampLinerSampler.reset();
	WarpLinerSampler.reset();
	MirrorLinerSampler.reset();
	ShadowSampler.reset();
	BlendDisable.reset();
	BSDisableWriteDisable.reset();
	BlendAlphaOn.reset();
	BlendAlphaOff.reset();
	BlendDisableAlphaWriteDisable.reset();
	DepthStateDisable.reset();
	DepthStateEnable.reset();
	DepthStateWriteEnable.reset();
	DepthStateWriteDisable.reset();
	RasterizerStateCullNone.reset();
	RasterizerStateCullBack.reset();
	RasterizerStateCullFront.reset();
	DepthStateEnableWriteDisable.reset();
}
