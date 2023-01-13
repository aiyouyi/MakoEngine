#pragma once
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11FBO.h"
#include "Toolbox/RectDraw.h"
#include "Toolbox/mathlib.h"

class CC3DTextureRHI;

class MaskEffect
{
public:
	MaskEffect();
	virtual ~MaskEffect();

	bool renderEffectToTexture(ID3D11ShaderResourceView *pBGTexture, ID3D11ShaderResourceView *pVideoTexture, ID3D11Texture2D *pDestTexture, int w, int h, void *pMaskInfo);

	bool renderRGBATexture(ID3D11ShaderResourceView *pVideoTexture, ID3D11Texture2D *pTargetTexture, int width, int height, void *pMaskInfo);

	void destory();

private:
	DX11FBO *m_pFBO;
	RectDraw *m_rectDraw;

	ID3D11Texture2D *m_pTargetTexture;
	std::shared_ptr<CC3DTextureRHI> m_InputSRV;
	std::shared_ptr<CC3DTextureRHI> m_VideoSRV;
};

