#pragma once
#include "Toolbox/RenderState/DepthStencilState.h"
#include "common.h"

class DX11DepthStencilState : public CC3DDepthStencilState
{
public:
	DX11DepthStencilState();
	virtual ~DX11DepthStencilState();

	virtual void CreateState();

public:
	ID3D11DepthStencilState* PtrDepthState = nullptr;
};
