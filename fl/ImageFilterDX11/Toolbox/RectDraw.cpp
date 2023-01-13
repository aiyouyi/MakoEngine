#include "RectDraw.h"
#include "Helper.h"
// #include "Toolbox/DXUtils/DXUtils.h"
#include "RectConstBuffer.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/DXUtils/DX11Context.h"

struct RectVertex
{
	Vector3 Pos;//位置  
	Vector2 TexCoord;//颜色  
};

const char *s_szBaseRectShader = "\
cbuffer ConstantBuffer : register(b0)\
{\
	float4x4 matWVP;\
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
	float4x4 matWVP;\
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
	float4x4 matWVP;\
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
	vEffect.a = 1.0-resMask;\
	return vEffect;\
}";

const char *s_szBaseRectShaderWithAlpha = "\
cbuffer ConstantBuffer : register(b0)\
{\
	float4x4 matWVP;\
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

static bool saveAsBMP(const char* pData, uint32_t nLen, int w, int h, int bitCount, const wchar_t* pszFileName)
{
	// Writes a BMP file
	// save to file
	HANDLE hFile = ::CreateFileW(pszFileName,            // file to create 
		GENERIC_WRITE,                // open for writing 
		0,                            // do not share 
		NULL,                         // default security 
		OPEN_ALWAYS,                  // overwrite existing 
		FILE_ATTRIBUTE_NORMAL,        // normal file 
		NULL);                        // no attr. template 
	if (!hFile || hFile == INVALID_HANDLE_VALUE)
	{
		return false;	// 
	}

	DWORD dwSizeBytes = nLen;

	// fill in the headers
	BITMAPFILEHEADER bmfh;
	bmfh.bfType = 0x4D42; // 'BM'
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwSizeBytes;//整个文件的大小
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//图像数据偏移量，即图像数据在文件中的保存位置

	DWORD dwBytesWritten;
	::WriteFile(hFile, &bmfh, sizeof(bmfh), &dwBytesWritten, NULL);
	if (dwBytesWritten != sizeof(bmfh))
	{
	}

	BITMAPINFOHEADER bmih;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = w;
	bmih.biHeight = -h;
	bmih.biPlanes = 1; // 图像的目标显示设备的位数，通常为1
	bmih.biBitCount = bitCount; // 每个像素的位数，可以为1、4、8、24、32
	if (bitCount == 8)
	{
		bmih.biCompression = BI_RLE8;
	}
	else
	{
		bmih.biCompression = BI_RGB;// 是否压缩
	}
	
	bmih.biSizeImage = 0;//图像大小的字节数
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	::WriteFile(hFile, &bmih, sizeof(bmih), &dwBytesWritten, NULL);
	if (dwBytesWritten != sizeof(bmih))
	{
	}

	::WriteFile(hFile, pData, dwSizeBytes, &dwBytesWritten, NULL);
	if (dwBytesWritten != dwSizeBytes)
	{
	}

	::CloseHandle(hFile);
	return true;
}

RectDraw::RectDraw()
{
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
	Vector3 arrCoords[] = { Vector3(x,y,0), Vector3(x,y+height,0), Vector3(x+width, y+height,0), Vector3(x+width, y,0)};
	for (int i = 0; i < 4; ++i)
	{
		arrCoords[i].x = arrCoords[i].x*2.0f - 1.0f;
		arrCoords[i].y = -(arrCoords[i].y*2.0f - 1.0f);
	}

	//创建顶点buffer
	RectVertex vertices[] =
	{
		{ arrCoords[0], Vector2(0.0f, 0.0f) },
		{ arrCoords[1], Vector2(0.0f, 1.0f) },
		{ arrCoords[2], Vector2(1.0f, 1.0f) },
		{ arrCoords[3], Vector2(1.0f, 0.0f) }
	};

	m_VertexBuffer = GetDynamicRHI()->CreateVertexBuffer(vertices, sizeof(BaseRectVertex),4,false);

	unsigned short index[] =
	{
		0, 1, 2,
		0, 2, 3
	};
	m_IndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);

	std::shared_ptr<CC3DTextureRHI> TexRHI =  GetDynamicRHI()->FetchTexture(szTexture,false);
	setTexture(TexRHI);

	//创建shader
	m_pShader = GetDynamicRHI()->CreateShaderRHI();
	m_pShader->InitShaderWithString(s_szBaseRectShader);

	m_pShaderOpaque = GetDynamicRHI()->CreateShaderRHI();
	m_pShaderOpaque->InitShaderWithString(s_szBaseRectOpaqueShader);

	m_pShaderWithMask = GetDynamicRHI()->CreateShaderRHI();
	m_pShaderWithMask->InitShaderWithString(s_szBaseRectShaderWithMask);

	m_pShaderWithAlpha = GetDynamicRHI()->CreateShaderRHI();
	m_pShaderWithAlpha->InitShaderWithString(s_szBaseRectShaderWithAlpha);

	return true;
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
		m_pBSState = GetDynamicRHI()->CreateBlendState(m_bBlend, m_bBlend, true);
	}
}

void RectDraw::reRect(float x, float y, float width, float height)
{
	m_VertexBuffer.reset();

	//创建顶点buffer
	RectVertex vertices[] =
	{
		{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f) },
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f) },
		{ Vector3(1, 1.0f, 0.0f), Vector2(1.0f, 0.0f) }
	};

	m_VertexBuffer = GetDynamicRHI()->CreateVertexBuffer(vertices, sizeof(BaseRectVertex), 4,false);

}

void RectDraw::setTexture(std::shared_ptr<CC3DTextureRHI> pTexture)
{
	m_pTexture = pTexture;
}

void RectDraw::setShaderTextureView(std::shared_ptr<CC3DTextureRHI> pShaderTextureView)
{
	m_pTexture = pShaderTextureView;
}

void RectDraw::renderOpaque()
{
	//设置shader
	m_pShaderOpaque->UseShader();

	//设置矩阵变换
	glm::mat4 Identity = glm::mat4(1.0);
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).SetParameter("matWVP", Identity);
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).ApplyToVSBuffer();

	GetDynamicRHI()->SetPSShaderResource(0, m_pTexture);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);
	GetDynamicRHI()->DrawPrimitive(m_VertexBuffer, m_IndexBuffer);
}

void RectDraw::render()
{

	//设置shader
	m_pShader->UseShader();
	
	glm::mat4 Identity = glm::mat4(1.0);
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).SetParameter("matWVP", Identity);
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).ApplyToVSBuffer();

	GetDynamicRHI()->SetPSShaderResource(0, m_pTexture);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

	GetDynamicRHI()->DrawPrimitive(m_VertexBuffer, m_IndexBuffer);
}

void RectDraw::render(Vector2 vTrans, Vector2 vScale, float fRotate, int w, int h)
{

	//设置shader
	m_pShader->UseShader();


	glm::mat4 matIdentity = glm::mat4(1.0);
	glm::mat4 matScale = glm::scale(matIdentity, glm::vec3(vScale[0], vScale[1], 1.0f));
	glm::mat4 matRotateZ = glm::rotate(matIdentity, fRotate * 3.1416f / 180.0f, glm::vec3{ 0.0f, 0.0f, 1.0f });

	glm::mat4 matPreScale = glm::scale(matIdentity, glm::vec3(w * 1.0f / h, 1.0f, 1.0f));
	glm::mat4 matEndScale = glm::scale(matIdentity, glm::vec3(h * 1.0f / w, 1.0f, 1.0f));

	matRotateZ = matPreScale * matRotateZ;
	matRotateZ = matRotateZ * matEndScale;

	matScale = matRotateZ * matScale;

	glm::mat4 matTranslate = glm::translate(matIdentity, glm::vec3(vTrans[0], vTrans[1], 0));
	matTranslate = matTranslate * matScale;
	
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).SetParameter("matWVP", matTranslate);
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).ApplyToVSBuffer();

	GetDynamicRHI()->SetPSShaderResource(0, m_pTexture);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

	GetDynamicRHI()->DrawPrimitive(m_VertexBuffer, m_IndexBuffer);
}

void RectDraw::render(std::shared_ptr<CC3DTextureRHI> pShaderTextureView, float *arrClip, void *pMaskInfo, int w, int h)
{
	if (pMaskInfo == nullptr || w <= 0 || h<0)
		render();


	//设置shader
	m_pShaderWithMask->UseShader();

	//设置矩阵变换

	GET_SHADER_STRUCT_MEMBER(ConstantBufferMask).SetParameter("matWVP", glm::mat4(1));
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMask).SetParameter("clip", Vector4(arrClip[0], arrClip[1], arrClip[2], arrClip[3]));
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMask).SetParameter("texSize", Vector2(1280 / 1.3, 720 / 1.3));
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMask).ApplyToAllBuffer();

	//设置纹理以及纹理采样
	GetDynamicRHI()->SetPSShaderResource(0, pShaderTextureView);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

	//std::vector<char> ConvertBuf;
	//ConvertBuf.resize(w * h * 4);

	//char* pImageSourceTemp = (char*)pMaskInfo;
	//char* pImageDestTemp = ConvertBuf.data();
	//for (int r = 0; r < h; ++r)
	//{
	//	for (int c = 0; c < w; ++c)
	//	{
	//		pImageDestTemp[0] = pImageSourceTemp[0];
	//		pImageDestTemp[1] = pImageSourceTemp[0];
	//		pImageDestTemp[2] = pImageSourceTemp[0];
	//		pImageDestTemp[3] = 255;

	//		pImageSourceTemp += 1;
	//		pImageDestTemp += 4;
	//	}
	//}

	//创建纹理
	if (m_pTextureMask == nullptr)
	{
		m_pTextureMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R8, 0, w, h, pMaskInfo, w, false);
	}
	else if (m_pTextureMask->GetWidth() == w && m_pTextureMask->GetHeight() == h)
	{
		m_pTextureMask->updateTextureInfo(pMaskInfo, w, h);
	}
	else
	{
		m_pTextureMask.reset();
		m_pTextureMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R8, 0, w, h, pMaskInfo, w , false);
	}


	GetDynamicRHI()->SetPSShaderResource(1, m_pTextureMask);
	GetDynamicRHI()->SetPSShaderResource(2, pShaderTextureView);

	GetDynamicRHI()->DrawPrimitive(m_VertexBuffer, m_IndexBuffer);
}

void RectDraw::renderAlpha(float *arrClip, void *pMaskInfo, int maskW, int maskH, int texW, int TexH)
{
	if (pMaskInfo == NULL || maskW <= 0 || maskH<0)render();

	if (!m_pBSState)
	{
		m_pBSState = GetDynamicRHI()->CreateBlendState(true, false, true);
	}

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(m_pBSState, blendFactor, 0xffffffff);

	//设置shader
	m_pShaderWithAlpha->UseShader();

	//设置矩阵变换
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMask).SetParameter("matWVP", glm::mat4(1));
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMask).SetParameter("clip", Vector4(arrClip[0], arrClip[1], arrClip[2], arrClip[3]));
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMask).SetParameter("texSize", Vector2(texW / 1.3, TexH / 1.3));
	GET_SHADER_STRUCT_MEMBER(ConstantBufferMask).ApplyToAllBuffer();


	//设置纹理以及纹理采样
	GetDynamicRHI()->SetPSShaderResource(0, m_pTexture);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

	//创建纹理
	if (m_pTextureMask == nullptr)
	{
		m_pTextureMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R8, 0, maskW, maskH, pMaskInfo, maskW, false);
	}
	else if (m_pTextureMask->GetWidth() == maskW && m_pTextureMask->GetHeight() == maskH)
	{
		m_pTextureMask->updateTextureInfo(pMaskInfo, maskW, maskH);
	}
	else
	{
		m_pTextureMask.reset();
		m_pTextureMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R8, 0, maskW, maskH, pMaskInfo, maskW, false);
	}

	GetDynamicRHI()->SetPSShaderResource(1, m_pTextureMask);
	GetDynamicRHI()->DrawPrimitive(m_VertexBuffer, m_IndexBuffer);
}

void RectDraw::destory()
{
	m_VertexBuffer.reset();
	m_IndexBuffer.reset();
	m_pTexture.reset();
	m_pTextureMask.reset();

	m_pBSState.reset();

	m_pShader.reset();
	m_pShaderOpaque.reset();
	m_pShaderWithAlpha.reset();
	m_pShaderWithMask.reset();
}