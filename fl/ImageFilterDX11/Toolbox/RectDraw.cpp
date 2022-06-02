#include "RectDraw.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Helper.h"
// #include "Toolbox/DXUtils/DXUtils.h"

const char *s_szBaseRectShader = "\
cbuffer ConstantBuffer : register(b0)\
{\
	matrix matWVP;\
}\
Texture2D txDiffuse : register(t0);\
SamplerState samLinear : register(s0);\
\
struct VS_INPUT\
{\
	float4 Pos : POSITION;\
	float2 Tex : TEXCOORD0;\
};\
struct VS_OUTPUT\
{\
	float4 Pos : SV_POSITION;\
	float2 Tex : TEXCOORD0;\
};\
\
VS_OUTPUT VS(VS_INPUT input)\
{\
	VS_OUTPUT output = (VS_OUTPUT)0;\
	output.Pos = mul(matWVP, input.Pos);\
	output.Tex = input.Tex;\
	return output;\
}\
float4 PS(VS_OUTPUT input) : SV_Target\
{\
	return txDiffuse.Sample(samLinear, input.Tex);\
}";


const char *s_szBaseRectOpaqueShader = "\
cbuffer ConstantBuffer : register(b0)\
{\
	matrix matWVP;\
}\
Texture2D txDiffuse : register(t0);\
SamplerState samLinear : register(s0);\
\
struct VS_INPUT\
{\
	float4 Pos : POSITION;\
	float2 Tex : TEXCOORD0;\
};\
struct VS_OUTPUT\
{\
	float4 Pos : SV_POSITION;\
	float2 Tex : TEXCOORD0;\
};\
\
VS_OUTPUT VS(VS_INPUT input)\
{\
	VS_OUTPUT output = (VS_OUTPUT)0;\
	output.Pos = mul(matWVP, input.Pos);\
	output.Tex = input.Tex;\
	return output;\
}\
float4 PS(VS_OUTPUT input) : SV_Target\
{\
	float4 res = txDiffuse.Sample(samLinear, input.Tex);\
	return float4(res.r, res.g, res.b, 1.0);\
}";


const char *s_szBaseRectShaderWithMask = "\
cbuffer ConstantBuffer : register(b0)\
{\
	matrix matWVP;\
	float4 bgClip;\
	float2 texSize;\
}\
Texture2D txDiffuse : register(t0);\
Texture2D txDiffuseMask : register(t1);\
Texture2D txDiffuseEffect : register(t2);\
SamplerState samLinear : register(s0);\
\
struct VS_INPUT\
{\
	float4 Pos : POSITION;\
	float2 Tex : TEXCOORD0;\
};\
struct VS_OUTPUT\
{\
	float4 Pos : SV_POSITION;\
	float2 Tex : TEXCOORD0;\
};\
\
VS_OUTPUT VS(VS_INPUT input)\
{\
	VS_OUTPUT output = (VS_OUTPUT)0;\
	output.Pos = mul(matWVP, input.Pos);\
	output.Tex = input.Tex;\
	return output;\
}\
float4 PS(VS_OUTPUT input) : SV_Target\
{\
	float resMask = 0.0;\
	float mask = txDiffuseMask.Sample(samLinear, input.Tex).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	 mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(1.0/texSize.x, 1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(0, 1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(-1.0/texSize.x, 1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(1.0/texSize.x, -1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(0, -1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(-1.0/texSize.x, -1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(-1.0/texSize.x, 0)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(1.0/texSize.x, 0)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	resMask = resMask/9.0;\
	float2 vEffectCoord = float2(input.Tex.x*(bgClip.z-bgClip.x) + bgClip.x, input.Tex.y*(bgClip.w-bgClip.y) + bgClip.y);\
	float4 vEffect = txDiffuseEffect.Sample(samLinear, vEffectCoord);\
	vEffect.a = 1.0;\
	return txDiffuse.Sample(samLinear, input.Tex)*resMask + vEffect*(1.0-resMask);\
}";

const char *s_szBaseRectShaderWithAlpha = "\
cbuffer ConstantBuffer : register(b0)\
{\
	matrix matWVP;\
	float4 bgClip;\
	float2 texSize;\
}\
Texture2D txDiffuse : register(t0);\
Texture2D txDiffuseMask : register(t1);\
SamplerState samLinear : register(s0);\
\
struct VS_INPUT\
{\
	float4 Pos : POSITION;\
	float2 Tex : TEXCOORD0;\
};\
struct VS_OUTPUT\
{\
	float4 Pos : SV_POSITION;\
	float2 Tex : TEXCOORD0;\
};\
\
VS_OUTPUT VS(VS_INPUT input)\
{\
	VS_OUTPUT output = (VS_OUTPUT)0;\
	output.Pos = mul(matWVP, input.Pos);\
	output.Tex = input.Tex;\
	return output;\
}\
float4 PS(VS_OUTPUT input) : SV_Target\
{\
	float resMask = 0.0;\
	float mask = txDiffuseMask.Sample(samLinear, input.Tex).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	 mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(1.0/texSize.x, 1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(0, 1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(-1.0/texSize.x, 1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(1.0/texSize.x, -1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(0, -1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(-1.0/texSize.x, -1.0/texSize.x)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(-1.0/texSize.x, 0)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	mask = txDiffuseMask.Sample(samLinear, input.Tex+float2(1.0/texSize.x, 0)).r;\
	if(mask >0.5)resMask=resMask+1.0; \
	resMask = resMask/9.0;\
	float2 vEffectCoord = float2(input.Tex.x*(bgClip.z-bgClip.x) + bgClip.x, input.Tex.y*(bgClip.w-bgClip.y) + bgClip.y);\
	float4 vRes = txDiffuse.Sample(samLinear, input.Tex);\
	vRes.a = resMask;\
	return vRes;\
}";

RectDraw::RectDraw()
{
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;

	m_pShader = NULL;
	m_pShaderOpaque = NULL;
	m_pShaderWithMask = NULL;
	m_pShaderWithAlpha = NULL;

	m_pBSState = NULL;
	m_bInvalidBlendState = true;
	m_bBlend = false;

	m_pTexture = NULL;
	m_pTextureMask = NULL;
	m_pShaderTextureView = NULL;
	m_pSamplerLinear = NULL;

	m_pConstantBuffer = NULL;
	m_pConstantBufferMask = NULL;
}

RectDraw::~RectDraw()
{
	destory();
}

bool RectDraw::init(float width, float height, const std::string &szTexture)
{
	return init(0,0,width,height, szTexture);
}

bool RectDraw::init(float x, float y, float width, float height, const std::string &szTexture)
{
	XMFLOAT3 arrCoords[] = { XMFLOAT3(x,y,0), XMFLOAT3(x,y+height,0), XMFLOAT3(x+width, y+height,0), XMFLOAT3(x+width, y,0)};
	for (int i = 0; i < 4; ++i)
	{
		arrCoords[i].x = arrCoords[i].x*2.0f - 1.0f;
		arrCoords[i].y = -(arrCoords[i].y*2.0f - 1.0f);
	}


	//创建顶点buffer
	BaseRectVertex vertices[] =
	{
		{ arrCoords[0], XMFLOAT2(0.0f, 0.0f) },
		{ arrCoords[1], XMFLOAT2(0.0f, 1.0f) },
		{ arrCoords[2], XMFLOAT2(1.0f, 1.0f) },
		{ arrCoords[3], XMFLOAT2(1.0f, 0.0f) }
	};
	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = sizeof(BaseRectVertex) * 4;
	verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	verBufferDesc.CPUAccessFlags = 0;
	verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA vertexInitData;
	memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexInitData.pSysMem = vertices;
	DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &m_rectVerticeBuffer);


	//创建索引buffer
	WORD index[] =
	{
		0, 1, 2,
		0, 2, 3
	};
	D3D11_BUFFER_DESC indexBufferDesc;
	memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.ByteWidth = sizeof(WORD) * 6;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA indexInitData;
	memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	indexInitData.pSysMem = index;
	DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &m_rectIndexBuffer);


	//手动创建纹理对象
	DX11Texture *pTexture = ContextInst->fetchTexture(szTexture);
	setTexture(pTexture);
	if (pTexture != NULL)
	{
		pTexture->unref();
	}

	//创建纹理采样
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = 0;
	HRESULT hr = DevicePtr->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
	if (FAILED(hr))return false;


	//创建shader
	m_pShader = new DX11Shader();
	m_pShader->initShaderWithString(s_szBaseRectShader);

	m_pShaderOpaque = new DX11Shader();
	m_pShaderOpaque->initShaderWithString(s_szBaseRectOpaqueShader);

	m_pShaderWithMask = new DX11Shader();
	m_pShaderWithMask->initShaderWithString(s_szBaseRectShaderWithMask);

	m_pShaderWithAlpha = new DX11Shader();
	m_pShaderWithAlpha->initShaderWithString(s_szBaseRectShaderWithAlpha);

	//创建constbuffer 参数
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(RectConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = DevicePtr->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	
	//创建constbuffer 参数
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(RectConstantBufferMask);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = DevicePtr->CreateBuffer(&bd, NULL, &m_pConstantBufferMask);
	return hr == S_OK;
}

void RectDraw::setBlend(bool bBlend)
{
	if (m_bBlend != bBlend)
	{
		m_bBlend = bBlend;
		m_bInvalidBlendState = true;
	}
}

void RectDraw::updateBlendState()
{
	if (m_bInvalidBlendState)
	{
		m_bInvalidBlendState = false;
		m_pBSState = ContextInst->fetchBlendState(m_bBlend, m_bBlend, true);
	}
}

void RectDraw::reRect(float x, float y, float width, float height)
{
	SAFERALEASE(m_rectVerticeBuffer);

	//创建顶点buffer
	BaseRectVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) }
	};
	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = sizeof(BaseRectVertex) * 4;
	verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	verBufferDesc.CPUAccessFlags = 0;
	verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA vertexInitData;
	memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexInitData.pSysMem = vertices;
	DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &m_rectVerticeBuffer);
}

void RectDraw::setTexture(DX11Texture *pTexture)
{
	if (m_pTexture == pTexture) return;

	if (m_pTexture != NULL)
	{
		m_pTexture->unref();
	}

	if (m_pShaderTextureView != NULL)
	{
		m_pShaderTextureView->Release();
	}

	m_pTexture = pTexture;
	if (m_pTexture != NULL)
	{
		m_pTexture->ref();

		m_pShaderTextureView = m_pTexture->getTexShaderView();

		if (m_pShaderTextureView != NULL)
		{
			m_pShaderTextureView->AddRef();
		}
	}
}

void RectDraw::setShaderTextureView(ID3D11ShaderResourceView *pShaderTextureView)
{
	if (m_pShaderTextureView == pShaderTextureView) return;

	if (m_pTexture != NULL)
	{
		m_pTexture->unref();
		m_pTexture = NULL;
	}

	if (m_pShaderTextureView != NULL)
	{
		m_pShaderTextureView->Release();
	}

	m_pShaderTextureView = pShaderTextureView;
	if (m_pShaderTextureView != NULL)
	{
		m_pShaderTextureView->AddRef();
	}
}

void RectDraw::renderOpaque()
{
	unsigned int nStride = sizeof(BaseRectVertex);
	unsigned int nOffset = 0;

	//设置shader
	m_pShaderOpaque->useShader();

	//设置矩阵变换
	RectConstantBuffer mWVP;
	mWVP.mWVP = XMMatrixIdentity();
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//设置纹理以及纹理采样
	ID3D11ShaderResourceView *pMyShaderResourceView = m_pShaderTextureView;
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//设置顶点数据
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(6, 0, 0);
}

void RectDraw::render()
{
	//if (m_pBSDisable == NULL)
	//{
	//	m_pBSDisable = DXUtils::CloseBlendState();
	//}
	//float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//DeviceContextPtr->OMSetBlendState(m_pBSDisable, blendFactor, 0xffffffff);
	unsigned int nStride = sizeof(BaseRectVertex);
	unsigned int nOffset = 0;

	//设置shader
	m_pShader->useShader();
	
	//设置矩阵变换
	RectConstantBuffer mWVP;
	mWVP.mWVP  =XMMatrixIdentity();
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//设置纹理以及纹理采样
	ID3D11ShaderResourceView *pMyShaderResourceView = m_pShaderTextureView;
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//设置顶点数据
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(6, 0, 0);
}

void RectDraw::render(vec2 vTrans, vec2 vScale, float fRotate, int w, int h)
{
	XMMATRIX matScale =  XMMatrixScaling(vScale[0], vScale[1], 1.0f);
	
	XMMATRIX matPreScale = XMMatrixScaling(w*1.0f/h, 1.0f, 1.0f);
	XMMATRIX matRotateZ = XMMatrixRotationZ(fRotate / 180.0f*3.1416f);
	XMMATRIX matEndScale = XMMatrixScaling(h*1.0f/w, 1.0f, 1.0f);
	matRotateZ = XMMatrixMultiply(matPreScale, matRotateZ);
	matRotateZ = XMMatrixMultiply(matRotateZ, matEndScale);

	matScale = XMMatrixMultiply(matRotateZ, matScale);

	XMMATRIX matTranslate = XMMatrixTranslation(vTrans[0], vTrans[1], 0);

	matTranslate = XMMatrixMultiply(matTranslate, matScale);

	unsigned int nStride = sizeof(BaseRectVertex);
	unsigned int nOffset = 0;

	//设置shader
	m_pShader->useShader();

	//设置矩阵变换
	RectConstantBuffer mWVP;
	mWVP.mWVP = matTranslate;// XMMatrixIdentity();
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//设置纹理以及纹理采样
	ID3D11ShaderResourceView *pMyShaderResourceView = m_pShaderTextureView;
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//设置顶点数据
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(6, 0, 0);
}

void RectDraw::render(ID3D11ShaderResourceView *pShaderTextureView, float *arrClip, void *pMaskInfo, int w, int h)
{
	if (pMaskInfo == NULL || w <= 0 || h<0)render();


	unsigned int nStride = sizeof(BaseRectVertex);
	unsigned int nOffset = 0;

	//设置shader
	m_pShaderWithMask->useShader();

	//设置矩阵变换
	RectConstantBufferMask mWVP;
	mWVP.mWVP = XMMatrixIdentity();
	mWVP.mClip = XMFLOAT4(arrClip[0], arrClip[1], arrClip[2], arrClip[3]);
	mWVP.texSize = vec2(1280/1.3, 720/1.3);

	DeviceContextPtr->UpdateSubresource(m_pConstantBufferMask, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBufferMask);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBufferMask);

	//设置纹理以及纹理采样
	ID3D11ShaderResourceView *pMyShaderResourceView = m_pShaderTextureView;
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//创建纹理
	if (m_pTextureMask == NULL)
	{
		m_pTextureMask = new DX11Texture();
		m_pTextureMask->initTexture(DXGI_FORMAT_R8_UNORM, D3D11_BIND_SHADER_RESOURCE, w, h, pMaskInfo, w, false);
	}
	else if (m_pTextureMask->width() == w && m_pTextureMask->height() == h)
	{
		m_pTextureMask->updateTextureInfo(pMaskInfo, w, h);
	}
	else
	{
		m_pTextureMask->destory();
		m_pTextureMask->initTexture(DXGI_FORMAT_R8_UNORM, D3D11_BIND_SHADER_RESOURCE, w, h, pMaskInfo, w, false);
	}

	pMyShaderResourceView = m_pTextureMask->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMyShaderResourceView);

	DeviceContextPtr->PSSetShaderResources(2, 1, &pShaderTextureView);

	//设置顶点数据
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(6, 0, 0);
}

void RectDraw::renderAlpha(float *arrClip, void *pMaskInfo, int maskW, int maskH, int texW, int TexH)
{
	if (pMaskInfo == NULL || maskW <= 0 || maskH<0)render();

	if (m_pBSState == NULL)
	{
		m_pBSState = ContextInst->fetchBlendState(true, false, true);
	}

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBSState, blendFactor, 0xffffffff);
	unsigned int nStride = sizeof(BaseRectVertex);
	unsigned int nOffset = 0;

	//设置shader
	m_pShaderWithAlpha->useShader();

	//设置矩阵变换
	RectConstantBufferMask mWVP;
	mWVP.mWVP = XMMatrixIdentity();
	mWVP.mClip = XMFLOAT4(arrClip[0], arrClip[1], arrClip[2], arrClip[3]);
	mWVP.texSize = vec2(texW/1.3, TexH/1.3);
	
	DeviceContextPtr->UpdateSubresource(m_pConstantBufferMask, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBufferMask);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBufferMask);

	//设置纹理以及纹理采样
	ID3D11ShaderResourceView *pMyShaderResourceView = m_pShaderTextureView;
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//创建纹理
	if (m_pTextureMask == NULL)
	{
		m_pTextureMask = new DX11Texture();
		m_pTextureMask->initTexture(DXGI_FORMAT_R8_UNORM, D3D11_BIND_SHADER_RESOURCE, maskW, maskH, pMaskInfo, maskW, false);
	}
	else if (m_pTextureMask->width() == maskW && m_pTextureMask->height() == maskH)
	{
		m_pTextureMask->updateTextureInfo(pMaskInfo, maskW, maskH);
	}
	else
	{
		m_pTextureMask->destory();
		m_pTextureMask->initTexture(DXGI_FORMAT_R8_UNORM, D3D11_BIND_SHADER_RESOURCE, maskW, maskH, pMaskInfo, maskW, false);
	}

	pMyShaderResourceView = m_pTextureMask->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMyShaderResourceView);

	//设置顶点数据
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(6, 0, 0);
}

void RectDraw::destory()
{
	SAFERALEASE(m_rectVerticeBuffer);
	SAFERALEASE(m_rectIndexBuffer);
	SAFERALEASE(m_pShaderTextureView);
	SAFERALEASE(m_pConstantBuffer);
	SAFERALEASE(m_pConstantBufferMask);
	SAFERALEASE(m_pSamplerLinear);

	SAFERALEASE(m_pBSState);
	//SAFERALEASE(m_pBSDisable);

	SAFEDEL(m_pShader);
	SAFEDEL(m_pShaderOpaque);
	SAFEDEL(m_pShaderWithAlpha);
	SAFEDEL(m_pShaderWithMask);

	if (m_pTexture != NULL)
	{
		m_pTexture->unref();
		m_pTexture = NULL;
	}

	if (m_pTextureMask != NULL)
	{
		m_pTextureMask->unref();
		m_pTextureMask = NULL;
	}
}