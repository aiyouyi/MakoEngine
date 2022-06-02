#pragma once
#include "Toolbox/RenderState/BlendState.h"
#include "common.h"

class DX11BlendState : public CC3DBlendState
{
public:
	DX11BlendState();
	virtual ~DX11BlendState();

	virtual void CreateState();
	void CreateState(bool bBlend, bool bBlendAlpha, bool writeBuffer);

	ID3D11BlendState* PtrBlendState = nullptr;
};