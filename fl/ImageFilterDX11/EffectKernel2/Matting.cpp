#include "Matting.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/Helper.h"
#include "Toolbox/mathlib.h"

const char *s_szMattingShader = R"(
static const float4x4 yuv_mat = { 0.182586,  0.614231,  0.062007, 0.062745,
-0.100644, -0.338572,  0.439216, 0.501961,
0.439216, -0.398942, -0.040274, 0.501961,
0.000000,  0.000000,  0.000000, 1.000000 };

cbuffer ConstantBuffer : register(b0)\
{
	float4x4 matWVP;
	float2 chroma_key;
	float2 pixel_size;
	float similarity;
	float smoothness;
	float spill;
}

Texture2D txDiffuse : register(t0);

SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 CalcColor(float4 rgba)
{
	return rgba;
}

float GetChromaDist(float3 rgb)
{
	float2 vU = mul(yuv_mat, float4(rgb, 1.0)).yz;

	float2 uvP = chroma_key;
	vU = vU-uvP;
	return length(vU);
}

float4 SampleTexture(float2 uv)
{
	return txDiffuse.Sample(samLinear, uv);
}

float GetBoxFilteredChromaDist(float3 rgb, float2 texCoord)
{
	float2 h_pixel_size = pixel_size / 2.0;
	float2 point_0 = float2(pixel_size.x, h_pixel_size.y);
	float2 point_1 = float2(h_pixel_size.x, -pixel_size.y);
	float distVal = GetChromaDist(SampleTexture(texCoord - point_0).rgb);
	distVal += GetChromaDist(SampleTexture(texCoord + point_0).rgb);
	distVal += GetChromaDist(SampleTexture(texCoord - point_1).rgb);
	distVal += GetChromaDist(SampleTexture(texCoord + point_1).rgb);
	distVal *= 2.0;
	distVal += GetChromaDist(rgb);
	return distVal / 9.0;
}

float4 ProcessChromaKey(float4 rgba, VS_OUTPUT v_in)
{
	float chromaDist = GetBoxFilteredChromaDist(rgba.rgb, v_in.uv);
	float baseMask = chromaDist - similarity;
	float fullMask = pow(saturate(baseMask / smoothness), 1.5);
	float spillVal = pow(saturate(baseMask / 0.041), 1.5);

	rgba.a *= fullMask;
	
	float avg_br = (rgba.r+rgba.b) / 2;
	if (rgba.g > avg_br)
		rgba.g = avg_br;

	return rgba;
}


VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	output.uv = input.Tex;
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 rgba = txDiffuse.Sample(samLinear, input.uv);
	return rgba;//ProcessChromaKey(rgba, input);
}
)";
/*
float3 fBGColor = float3(0,  0.526, 0.251);
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 rgba = txDiffuse.Sample(samLinear, input.uv);
return ProcessChromaKey(rgba, input);
}
)";*/
//ProcessChromaKey(rgba, v_in);
MattingDraw::MattingDraw()
{
	m_pFBO = NULL;
	m_pTargetTexture = NULL;

	similarity = 0.0438f;
	smoothness = 0.041;
	spill = 0.041;

	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;

	m_pShader = NULL;

	m_pTexture = NULL;
	m_pTextureMask = NULL;
	m_pShaderTextureView = NULL;
	m_pSamplerLinear = NULL;

	m_pConstantBuffer = NULL;
	m_pConstantBufferMask = NULL;

	m_pBSEnable = NULL;
}

MattingDraw::~MattingDraw()
{
	destory();
}

bool MattingDraw::init(float width, float height, const std::string &szTexture)
{
	return init(0,0,width,height, szTexture);
}

bool MattingDraw::init(float x, float y, float width, float height, const std::string &szTexture)
{
	XMFLOAT3 arrCoords[] = { XMFLOAT3(x,y,0), XMFLOAT3(x,y+height,0), XMFLOAT3(x+width, y+height,0), XMFLOAT3(x+width, y,0)};
	for (int i = 0; i < 4; ++i)
	{
		arrCoords[i].x = arrCoords[i].x*2.0f - 1.0f;
		arrCoords[i].y = -(arrCoords[i].y*2.0f - 1.0f);
	}


	//创建顶点buffer
	MattingRectVertex vertices[] =
	{
		{ arrCoords[0], XMFLOAT2(0.0f, 0.0f) },
		{ arrCoords[1], XMFLOAT2(0.0f, 1.0f) },
		{ arrCoords[2], XMFLOAT2(1.0f, 1.0f) },
		{ arrCoords[3], XMFLOAT2(1.0f, 0.0f) }
	};
	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = sizeof(MattingRectVertex) * 4;
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
	m_pShader->initShaderWithString(s_szMattingShader);

	//创建constbuffer 参数
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SMattingBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = DevicePtr->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	return hr == S_OK;
}

void MattingDraw::reRect(float x, float y, float width, float height)
{
	SAFERALEASE(m_rectVerticeBuffer);

	//创建顶点buffer
	MattingRectVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) }
	};
	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = sizeof(MattingRectVertex) * 4;
	verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	verBufferDesc.CPUAccessFlags = 0;
	verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA vertexInitData;
	memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexInitData.pSysMem = vertices;
	DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &m_rectVerticeBuffer);
}

void MattingDraw::setTexture(DX11Texture *pTexture)
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

void MattingDraw::setShaderTextureView(ID3D11ShaderResourceView *pShaderTextureView)
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

void MattingDraw::render(int w, int h)
{
	//混合状态
	m_pBSEnable = ContextInst->fetchBlendState(false, false, true);

	unsigned int nStride = sizeof(MattingRectVertex);
	unsigned int nOffset = 0;

	//设置shader
	m_pShader->useShader();
	
	//设置矩阵变换
	SMattingBuffer mWVP;
	mWVP.mWVP  =XMMatrixIdentity();
	{
		static const float yuv_mat[16] = 
		  { 0.182586f, -0.100644f, 0.439216f,  0.0f,
			0.614231f, -0.338572f, -0.398942f, 0.0f,
			0.062007f, 0.439216f,  -0.040274f, 0.0f,
			0.062745f, 0.501961f,  0.501961f,  1.0f };
		mat4 matYUVTrans = mat4(yuv_mat);
		mat4 matYUVTransT = matYUVTrans.transpose();
		vec4 rgb = vec4(0, 1, 0, 1);

		vec4 yuvBase = matYUVTrans*rgb;

		mWVP.chroma_key[0] = yuvBase.y;
		mWVP.chroma_key[1] = yuvBase.z;

		mWVP.pixel_size[0] = 1.0f / w;
		mWVP.pixel_size[1] = 1.0f / h;

		mWVP.similarity = similarity;
		mWVP.smoothness = smoothness;
		mWVP.spill = spill;
	}
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//设置纹理以及纹理采样
	ID3D11ShaderResourceView *pMyShaderResourceView = m_pShaderTextureView;
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	//设置顶点数据
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(6, 0, 0);
}

bool MattingDraw::renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height)
{
	this->setShaderTextureView(pInputTexture);

	if (m_pFBO == NULL || m_pFBO->width() != width || m_pFBO->height() != height || m_pTargetTexture != pTargetTexture)
	{
		if (m_pFBO != NULL)
		{
			delete m_pFBO;
		}

		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(width, height, false, pTargetTexture);

		m_pTargetTexture = pTargetTexture;
	}

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContextPtr->RSSetViewports(1, &vp);
	m_pFBO->bind();
	m_pFBO->clear(0, 0, 0, 0);
	render(width, height);
	m_pBSEnable = ContextInst->fetchBlendState(true, false, true);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);
	return true;
}

void MattingDraw::destory()
{
	SAFERALEASE(m_rectVerticeBuffer);
	SAFERALEASE(m_rectIndexBuffer);
	SAFERALEASE(m_pShaderTextureView);
	SAFERALEASE(m_pConstantBuffer);
	SAFERALEASE(m_pConstantBufferMask);
	SAFERALEASE(m_pSamplerLinear);

	SAFEDEL(m_pShader);

	SAFEDEL(m_pFBO);
	m_pTargetTexture = NULL;

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