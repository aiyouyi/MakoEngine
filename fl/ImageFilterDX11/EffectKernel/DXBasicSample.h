#pragma once
#include "BaseRenderParam.h"
#include "Toolbox/DXUtils/DX11Shader.h"

class DXBaicSample
{
public:
	DXBaicSample();
	virtual ~DXBaicSample();

protected:
	ID3D11BlendState* m_pBlendStateNormal = nullptr;
	ID3D11SamplerState* m_pSamplerLinear = nullptr;
	DX11Shader* m_pShader = nullptr;
};