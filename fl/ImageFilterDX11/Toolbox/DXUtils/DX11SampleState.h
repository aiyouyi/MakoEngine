#pragma once
#include "Toolbox/RenderState/SamplerState.h"
#include "common.h"

class DX11SampleState : public CC3DSamplerState
{
public:
	DX11SampleState();
	virtual ~DX11SampleState();

	virtual void CreateState();

public:
	ID3D11SamplerState* PtrSamplerState = nullptr;
};