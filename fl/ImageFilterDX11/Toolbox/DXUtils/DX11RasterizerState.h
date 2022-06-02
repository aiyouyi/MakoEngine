#pragma once
#include "Toolbox/RenderState/RasterizerState.h"
#include "common.h"

class DX11RasterizerState : public CC3DRasterizerState
{
public:
	DX11RasterizerState();
	virtual ~DX11RasterizerState();

	virtual void CreateState();

public:
	ID3D11RasterizerState* PtrRasterizerState = nullptr;
};