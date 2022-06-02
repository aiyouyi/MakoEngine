#pragma once

#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11Texture.h"
#include <xnamath.h>
#include "mathlib.h"
struct BaseRectVertex
{
	XMFLOAT3 Pos;//Î»ÖÃ  
	XMFLOAT2 TexCoord;//ÑÕÉ«  
};
struct RectConstantBuffer
{
	XMMATRIX mWVP; //»ìºÏ¾ØÕó
};
struct RectConstantBufferMask
{
	XMMATRIX mWVP; //»ìºÏ¾ØÕó
	XMFLOAT4 mClip;//±³¾°²Ã¼ôÇøÓò
	vec2 texSize;
};
class DX11IMAGEFILTER_EXPORTS_CLASS RectDraw
{
public:
	RectDraw();
	virtual ~RectDraw();

	bool init(float width, float height, const std::string &szTexture = "");
	bool init(float x, float y, float width, float height, const std::string &szTexture = "");

	void reRect(float x, float y, float width, float height);

	void setBlend(bool bBlend);

	void setTexture(DX11Texture *pTexture);

	void setShaderTextureView(ID3D11ShaderResourceView *pShaderTextureView);

	void renderOpaque();

	void render();

	void render(ID3D11ShaderResourceView *pShaderTextureView, float *arrClip, void *pMaskInfo, int w, int h);

	void renderAlpha(float *arrClip, void *pMaskInfo, int maskW, int maskH, int texW, int TexH);

	void render(vec2 vTrans, vec2 vScale, float fRotate, int w, int h);

	void destory();

protected:
	void updateBlendState();

private:
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	
	DX11Shader *m_pShader;
	DX11Shader *m_pShaderOpaque;
	DX11Shader *m_pShaderWithMask;
	DX11Shader *m_pShaderWithAlpha;
	DX11Texture *m_pTexture;

	bool m_bInvalidBlendState;
	bool m_bBlend;
	ID3D11BlendState *m_pBSState;
	//ID3D11BlendState *m_pBSDisable = NULL;
	DX11Texture *m_pTextureMask;

	ID3D11ShaderResourceView *m_pShaderTextureView;
	ID3D11SamplerState* m_pSamplerLinear;

	ID3D11Buffer*       m_pConstantBuffer;  //×ª»»¾ØÕó
	ID3D11Buffer*       m_pConstantBufferMask;  //×ª»»¾ØÕó
};

