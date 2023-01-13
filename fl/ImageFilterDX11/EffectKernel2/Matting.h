#pragma once
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11FBO.h"
#include <xnamath.h>
class DX11Texture;

struct MattingRectVertex
{
	XMFLOAT3 Pos;//位置  
	XMFLOAT2 TexCoord;//颜色  
};

struct SMattingBuffer
{
	XMMATRIX mWVP;
	float chroma_key[2];
	float pixel_size[2];
	float similarity;
	float smoothness;
	float spill;
};

class DX11IMAGEFILTER_EXPORTS_CLASS MattingDraw
{
public:
	MattingDraw();
	virtual ~MattingDraw();

	bool init(float width, float height, const std::string &szTexture = "");
	bool init(float x, float y, float width, float height, const std::string &szTexture = "");

	void reRect(float x, float y, float width, float height);
	void setTexture(DX11Texture *pTexture);

	void setShaderTextureView(ID3D11ShaderResourceView *pShaderTextureView);

	void render(int w, int h);

	bool renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height);

	void destory();

public:
	float similarity;
	float smoothness;
	float spill;
private:
	DX11FBO *m_pFBO;
	ID3D11Texture2D *m_pTargetTexture;


	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	
	DX11Shader *m_pShader;
	DX11Texture *m_pTexture;

	DX11Texture *m_pTextureMask;

	ID3D11ShaderResourceView *m_pShaderTextureView;
	ID3D11SamplerState* m_pSamplerLinear;

	ID3D11Buffer*       m_pConstantBuffer;  //转换矩阵
	ID3D11Buffer*       m_pConstantBufferMask;  //转换矩阵

	ID3D11BlendState *m_pBSEnable;
};

