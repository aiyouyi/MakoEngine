#pragma once
#include "Toolbox/Render/DynamicRHI.h"

class CC3DPiplelineState
{
public:
	CC3DPiplelineState();
	~CC3DPiplelineState();

	static void Initialize();
	static void DestroyAll();

public:
	static std::shared_ptr<CC3DSamplerState> ClampLinerSampler;
	static std::shared_ptr<CC3DSamplerState> WarpLinerSampler;
	static std::shared_ptr<CC3DSamplerState> MirrorLinerSampler;
	static std::shared_ptr<CC3DSamplerState> ShadowSampler;
	
	static std::shared_ptr<CC3DBlendState> BlendDisable;
	static std::shared_ptr<CC3DBlendState> BlendAlphaOff;
	static std::shared_ptr<CC3DBlendState> BSDisableWriteDisable;

	static std::shared_ptr<CC3DDepthStencilState> DepthStateDisable;
	static std::shared_ptr<CC3DDepthStencilState> DepthStateEnable;

	static std::shared_ptr<CC3DRasterizerState> RasterizerStateCullNone;
	static std::shared_ptr<CC3DRasterizerState> RasterizerStateCullBack;
	static std::shared_ptr<CC3DRasterizerState> RasterizerStateCullFront;
};