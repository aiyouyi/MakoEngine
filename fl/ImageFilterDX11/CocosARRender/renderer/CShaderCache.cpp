#include "CShaderCache.h"

const char *s_szDiffuseShader_Lit = R"(
static const int DIRECT_LIGHT_COUNT = 4;
cbuffer ConstantBuffer : register(b0)
{
	matrix matWorld;
	matrix matWorldR;
	matrix matVP;
	float4 u_arrLights[DIRECT_LIGHT_COUNT * 3];
}
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal: NORMAL; 
	float3 ViewDir: COLOR0;
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWorld, input.Pos);
	output.ViewDir = normalize(output.Pos.xyz);
	output.Pos = mul(matVP, output.Pos);
	float4 vNormalW = mul(matWorldR, float4(input.Normal.x, input.Normal.y, input.Normal.z, 1.0));
	output.Normal = vNormalW.xyz;   
	output.Tex = float2(input.Tex.x, 1.0-input.Tex.y);
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float fDiffuse = dot(input.Normal, u_arrLights[3].xyz);
	float3 vDiffuse = u_arrLights[1].xyz*max(fDiffuse, 0.0) + u_arrLights[0].xyz;
	vDiffuse = min(vDiffuse, float3(1.0, 1.0, 1.0));
	
	float3 vReflect = reflect(-u_arrLights[3].xyz, input.Normal);
	float fHightLight = dot(vReflect, -input.ViewDir);
	float3 vHightLight = u_arrLights[2].xyz*max(pow(fHightLight,5.0), 0.0);
	float4 vTexture = txDiffuse.Sample(samLinear, input.Tex);

	return vTexture*float4(vDiffuse.x, vDiffuse.y, vDiffuse.z, 1.0) + float4(vHightLight.x, vHightLight.y, vHightLight.z, 0.0);
})";

const char *s_szDiffuseShader = R"(
static const int DIRECT_LIGHT_COUNT = 4;
cbuffer ConstantBuffer : register(b0)
{
	matrix matWorld;
	matrix matWorldR;
	matrix matVP;
	float4 u_arrLights[DIRECT_LIGHT_COUNT * 3];
}
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal: NORMAL; 
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWorld, input.Pos);
	output.Pos = mul(matVP, output.Pos);
	output.Normal = input.Normal;
	output.Tex = float2(input.Tex.x, 1.0-input.Tex.y);
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	return txDiffuse.Sample(samLinear, input.Tex);
})";

const char *s_szDiffuseSkinShader_Lit = R"(
static const int SKINNING_JOINT_COUNT = 128;
static const int DIRECT_LIGHT_COUNT = 4;\
cbuffer ConstantBuffer : register(b0)
{
	matrix matWorld;
	matrix matWorldR;\
	matrix matVP;
	float4 u_matrixPalette[SKINNING_JOINT_COUNT * 3];
	float4 u_arrLights[DIRECT_LIGHT_COUNT * 4];\
}
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float4 BlendWeight: BLENDWEIGHT;
	float4 BlendIndex:  BLENDINDICES;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal: NORMAL; 
	float3 ViewDir: COLOR0;
	float2 Tex : TEXCOORD0;
};

void getPositionAndNormal(VS_INPUT input, out float4 position, out float3 normal)
{
	float blendWeight = input.BlendWeight[0];

	int matrixIndex = int(input.BlendIndex[0]) * 3;
	float4 matrixPalette1 = u_matrixPalette[matrixIndex] * blendWeight;
	float4 matrixPalette2 = u_matrixPalette[matrixIndex + 1] * blendWeight;
	float4 matrixPalette3 = u_matrixPalette[matrixIndex + 2] * blendWeight;


	blendWeight = input.BlendWeight[1];
	if (blendWeight > 0.0)
	{
		matrixIndex = int(input.BlendIndex[1]) * 3;
		matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
		matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
		matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;

		blendWeight = input.BlendWeight[2];
		if (blendWeight > 0.0)
		{
			matrixIndex = int(input.BlendIndex[2]) * 3;
			matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
			matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
			matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;

			blendWeight = input.BlendWeight[3];
			if (blendWeight > 0.0)
			{
				matrixIndex = int(input.BlendIndex[3]) * 3;
				matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
				matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
				matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;
			}
		}
	}

	float4 p = float4(input.Pos.xyz, 1.0);
	position.x = dot(p, matrixPalette1);
	position.y = dot(p, matrixPalette2);
	position.z = dot(p, matrixPalette3);
	position.w = p.w;

	float4 n = float4(input.Normal, 0.0);
	normal.x = dot(n, matrixPalette1);
	normal.y = dot(n, matrixPalette2);
	normal.z = dot(n, matrixPalette3);

	float4 ns = float4(0.0, 0.0, 0.0, 0.0);
	float3 normalS0 = float3(dot(ns, matrixPalette1), dot(ns, matrixPalette2), dot(ns, matrixPalette3));

	normal = normalize(normal - normalS0);
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 vPos;
	float3 vNormal;
	getPositionAndNormal(input, vPos, vNormal);

	output.Pos = mul(matWorld, vPos);
	output.ViewDir = normalize(output.Pos.xyz);
	output.Pos = mul(matVP, output.Pos);
	float4 vNormalW = mul(matWorldR, float4(vNormal.x, vNormal.y, vNormal.z, 1.0));
	output.Normal = vNormalW.xyz;   
	output.Tex = float2(input.Tex.x, 1.0-input.Tex.y);
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float fDiffuse = dot(input.Normal, u_arrLights[3].xyz);
	float3 vDiffuse = u_arrLights[1].xyz*max(fDiffuse, 0.0) + u_arrLights[0].xyz;
	vDiffuse = min(vDiffuse, float3(1.0, 1.0, 1.0));
	
	float3 vReflect = reflect(-u_arrLights[3].xyz, input.Normal);
	float fHightLight = dot(vReflect, -input.ViewDir);
	float3 vHightLight = u_arrLights[2].xyz*max(pow(fHightLight,5.0), 0.0);
	float4 vTexture = txDiffuse.Sample(samLinear, input.Tex);

	return vTexture*float4(vDiffuse.x, vDiffuse.y, vDiffuse.z, 1.0) + float4(vHightLight.x, vHightLight.y, vHightLight.z, 0.0);
})";


const char *s_szDiffuseSkinShader_NPR = R"(
static const int SKINNING_JOINT_COUNT = 128;
static const int DIRECT_LIGHT_COUNT = 4;\
cbuffer ConstantBuffer : register(b0)
{
	matrix matWorld;
	matrix matWorldR;\
	matrix matVP;
	float4 u_matrixPalette[SKINNING_JOINT_COUNT * 3];
	float4 u_arrLights[DIRECT_LIGHT_COUNT * 4];\
    float4 u_NPRInfo;
    float4 u_NPRLineColor;
}
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float4 BlendWeight: BLENDWEIGHT;
	float4 BlendIndex:  BLENDINDICES;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal: NORMAL; 
	float3 ViewDir: COLOR0;
	float2 Tex : TEXCOORD0;
};

void getPositionAndNormal(VS_INPUT input, out float4 position, out float3 normal)
{
	float blendWeight = input.BlendWeight[0];

	int matrixIndex = int(input.BlendIndex[0]) * 3;
	float4 matrixPalette1 = u_matrixPalette[matrixIndex] * blendWeight;
	float4 matrixPalette2 = u_matrixPalette[matrixIndex + 1] * blendWeight;
	float4 matrixPalette3 = u_matrixPalette[matrixIndex + 2] * blendWeight;


	blendWeight = input.BlendWeight[1];
	if (blendWeight > 0.0)
	{
		matrixIndex = int(input.BlendIndex[1]) * 3;
		matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
		matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
		matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;

		blendWeight = input.BlendWeight[2];
		if (blendWeight > 0.0)
		{
			matrixIndex = int(input.BlendIndex[2]) * 3;
			matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
			matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
			matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;

			blendWeight = input.BlendWeight[3];
			if (blendWeight > 0.0)
			{
				matrixIndex = int(input.BlendIndex[3]) * 3;
				matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
				matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
				matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;
			}
		}
	}

    float3 pSrc = input.Pos.xyz;
    if(u_NPRInfo.x>0.001)
    {
        pSrc += input.Normal*u_NPRInfo.x;
    }
    
	float4 p = float4(pSrc, 1.0);
	position.x = dot(p, matrixPalette1);
	position.y = dot(p, matrixPalette2);
	position.z = dot(p, matrixPalette3);
	position.w = p.w;

	float4 n = float4(input.Normal, 0.0);
	normal.x = dot(n, matrixPalette1);
	normal.y = dot(n, matrixPalette2);
	normal.z = dot(n, matrixPalette3);

	float4 ns = float4(0.0, 0.0, 0.0, 0.0);
	float3 normalS0 = float3(dot(ns, matrixPalette1), dot(ns, matrixPalette2), dot(ns, matrixPalette3));

	normal = normalize(normal - normalS0);
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 vPos;
	float3 vNormal;
	getPositionAndNormal(input, vPos, vNormal);

	output.Pos = mul(matWorld, vPos);
	output.ViewDir = normalize(output.Pos.xyz);
	output.Pos = mul(matVP, output.Pos);
    output.Pos/=output.Pos.w;
	output.Pos.z = (output.Pos.z - u_NPRInfo.y)* u_NPRInfo.z;
    if(u_NPRInfo.x>0.001)
    {
        output.Pos.z += u_NPRInfo.w;
    }
	float4 vNormalW = mul(matWorldR, float4(vNormal.x, vNormal.y, vNormal.z, 1.0));
	output.Normal = vNormalW.xyz;   
	output.Tex = float2(input.Tex.x, 1.0-input.Tex.y);
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
    if(u_NPRInfo.x<0.001)
    {
        float4 _SpecularColor = float4(1.0,1.0,1.0,1.0);
        float _Shininess = 5;
        float _SpecThresold = u_arrLights[2].y ;

        float4 _KCool = float4(1.0,1.0,1.0,1.0);
        float4 _KWarm = float4(0.8,0.8,0.8,1.0);
        float _Darkness = u_arrLights[2].z ;
        float4 _KSSS = float4(1.0,1.0,1.0,1.0);


        float4 vTexture = txDiffuse.Sample(samLinear, input.Tex);
        float fDiffuse = dot(input.Normal, u_arrLights[3].xyz);
        fDiffuse = fDiffuse*0.5+0.5;
        float darkness = 1.0 - step(_Darkness, fDiffuse);
        float4 cool = (_KCool * _KSSS) * (1.0 - darkness);
        float4 warm = _KWarm * darkness;
		float4 diffuseNPR = (cool + warm)* vTexture;
        float4 vDiffuse = u_arrLights[0]*vTexture +u_arrLights[1] *diffuseNPR;


        float3 vReflect = reflect(-u_arrLights[3].xyz, input.Normal);
        float fHightLight = dot(vReflect, -input.ViewDir);

        fHightLight = max(fHightLight,0.0);
        float spec = pow(fHightLight,_Shininess);
		 
        if (spec < _SpecThresold)
        {
            spec = 0.0;
        }
        else if (spec > _SpecThresold+u_arrLights[2].w )
        {
            spec = 1.0;
        }
        else
        {
            spec =  (spec-_SpecThresold)/u_arrLights[2].w;
        }

       
        //spec = step(_SpecThresold, spec);
        float4 vHightLight = _SpecularColor*spec;
        float4 SrcColor =  vDiffuse + u_arrLights[2].x * vHightLight;
        SrcColor = min(float4(1.0,1.0,1.0,1.0),SrcColor);
        return SrcColor;//*alpha +  float4(0.5,0.5,0.5,1.0)*(1.0-alpha);
    }
    else
    {
        return u_NPRLineColor;
    }
}
)";

const char *s_szDiffuseSkinShader = R"(
static const int SKINNING_JOINT_COUNT = 128;
cbuffer ConstantBuffer : register(b0)
{
	matrix matWorld;
	matrix matWorldR;
	matrix matVP;
	float4 u_matrixPalette[SKINNING_JOINT_COUNT * 3];
}
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float4 BlendWeight: BLENDWEIGHT;
	float4 BlendIndex:  BLENDINDICES;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal: NORMAL; 
	float2 Tex : TEXCOORD0;
};

void getPositionAndNormal(VS_INPUT input, out float4 position, out float3 normal)
{
	float blendWeight = input.BlendWeight[0];

	int matrixIndex = int(input.BlendIndex[0]) * 3;
	float4 matrixPalette1 = u_matrixPalette[matrixIndex] * blendWeight;
	float4 matrixPalette2 = u_matrixPalette[matrixIndex + 1] * blendWeight;
	float4 matrixPalette3 = u_matrixPalette[matrixIndex + 2] * blendWeight;


	blendWeight = input.BlendWeight[1];
	if (blendWeight > 0.0)
	{
		matrixIndex = int(input.BlendIndex[1]) * 3;
		matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
		matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
		matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;

		blendWeight = input.BlendWeight[2];
		if (blendWeight > 0.0)
		{
			matrixIndex = int(input.BlendIndex[2]) * 3;
			matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
			matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
			matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;

			blendWeight = input.BlendWeight[3];
			if (blendWeight > 0.0)
			{
				matrixIndex = int(input.BlendIndex[3]) * 3;
				matrixPalette1 += u_matrixPalette[matrixIndex] * blendWeight;
				matrixPalette2 += u_matrixPalette[matrixIndex + 1] * blendWeight;
				matrixPalette3 += u_matrixPalette[matrixIndex + 2] * blendWeight;
			}
		}
	}

	float4 p = float4(input.Pos.xyz, 1.0);
	position.x = dot(p, matrixPalette1);
	position.y = dot(p, matrixPalette2);
	position.z = dot(p, matrixPalette3);
	position.w = p.w;

	float4 n = float4(input.Normal, 0.0);
	normal.x = dot(n, matrixPalette1);
	normal.y = dot(n, matrixPalette2);
	normal.z = dot(n, matrixPalette3);

	float4 ns = float4(0.0, 0.0, 0.0, 0.0);
	float3 normalS0 = float3(dot(ns, matrixPalette1), dot(ns, matrixPalette2), dot(ns, matrixPalette3));

	normal = normalize(normal - normalS0);
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 vPos;
	float3 vNormal;
	getPositionAndNormal(input, vPos, vNormal);

	output.Pos = mul(matWorld, vPos);
	output.Pos = mul(matVP, output.Pos);
	output.Normal = vNormal;
	output.Tex = float2(input.Tex.x, 1.0-input.Tex.y);
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	return txDiffuse.Sample(samLinear, input.Tex);
})";



NS_CC_BEGIN
class CocosDefaultShader : public DX11Shader
{
public:
	CocosDefaultShader(MeshVertexAttrib *attrib, int nSize) 
	{
		this->attrib = new MeshVertexAttrib[nSize];
		memcpy(this->attrib, attrib, sizeof(MeshVertexAttrib)*nSize);
		this->nSize = nSize;
	}
	virtual~CocosDefaultShader() 
	{
		CC_SAFE_DELETE(attrib);
	}

	virtual bool initInputLayout(ID3DBlob *pVSBlob)
	{
		string arrSemantic[] = { "POSITION", "COLOR", "TEXCOORD", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3", "NORMAL", "BLENDWEIGHT", "BLENDINDICES", "TANGENT", "BINORMAL" };
		DXGI_FORMAT arrFormat[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT};
		HRESULT hr = S_OK;
		D3D11_INPUT_ELEMENT_DESC layout[10];
		unsigned int step = 0;
		for (int i = 0; i < nSize; ++i)
		{
			layout[i] = { arrSemantic[attrib[i].vertexAttrib].c_str(), 0, arrFormat[attrib[i].size-1], 0, step, D3D11_INPUT_PER_VERTEX_DATA , 0 };
			step += sizeof(float)*attrib[i].size;
		}
		UINT numElements = nSize;
		hr = DevicePtr->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);

		return (hr == S_OK);
	}

protected:
	MeshVertexAttrib *attrib;
	int nSize;
};

CCShaderCache *CCShaderCache::s_pInst = NULL;
CCShaderCache* CCShaderCache::shareInst()
{
	if (s_pInst == NULL)
	{
		s_pInst = new CCShaderCache();
	}

	return s_pInst;
}

void CCShaderCache::destoryInst()
{
	if (s_pInst != NULL)
	{
		delete s_pInst;
		s_pInst = NULL;
	}
}

DX11Shader *CCShaderCache::fetchShader(MeshVertexAttrib *attrib, int nSize, bool lighting, bool bNPR )
{
	if (m_pDiffuseShaderSkin == NULL)
	{
		loadDefaultShaders();
	}

	bool bHasLighting = false;
	bool bHasSkin = false;

	if (lighting)
	{
		for (int i = 0; i < nSize; ++i)
		{
			if (attrib[i].vertexAttrib == VertexData::VERTEX_ATTRIB_NORMAL)
			{
				bHasLighting = true;
				break;
			}
		}
	}

	unsigned char flag = 0;
	for (int i = 0; i < nSize; ++i)
	{
		if (attrib[i].vertexAttrib == VertexData::VERTEX_ATTRIB_BLEND_WEIGHT)
		{
			flag += 1;
		}
		else if (attrib[i].vertexAttrib == VertexData::VERTEX_ATTRIB_BLEND_INDEX)
		{
			flag += 2;
		}
	}
	if (flag == 3)
	{
		bHasSkin = true;
	}

	if (bHasSkin)
	{
		if (bHasLighting)
		{
			if (bNPR)
			{
				return m_pDiffuseShaderSkin_NPR;
			}

			return m_pDiffuseLitShaderSkin;
		}
		else
		{
			return m_pDiffuseShaderSkin;
		}
	}
	else
	{
		if (bHasLighting)
		{
			return m_pDiffuseLitShader;
		}
		else
		{
			return m_pDiffuseShader;
		}
	}
}

CCShaderCache::CCShaderCache()
{
	m_pDiffuseShader = NULL;
	m_pDiffuseShaderSkin = NULL;
	m_pDiffuseLitShader = NULL;
	m_pDiffuseLitShaderSkin = NULL;
	m_pDiffuseShaderSkin_NPR = NULL;
}

CCShaderCache::~CCShaderCache()
{
	CC_SAFE_DELETE(m_pDiffuseShader);
	CC_SAFE_DELETE(m_pDiffuseShaderSkin);
	CC_SAFE_DELETE(m_pDiffuseLitShader);
	CC_SAFE_DELETE(m_pDiffuseLitShaderSkin);
	CC_SAFE_DELETE(m_pDiffuseShaderSkin_NPR);
}

void CCShaderCache::loadDefaultShaders()
{
	MeshVertexAttrib arrDiffuseAttrib[] = 
	{
		{ 3, 0, VertexData::VERTEX_ATTRIB_POSITION, 3 * sizeof(float) },
		{ 3, 0, VertexData::VERTEX_ATTRIB_NORMAL, 3 * sizeof(float) },
		{ 2, 0, VertexData::VERTEX_ATTRIB_TEX_COORD, 2 * sizeof(float) }
	};
	m_pDiffuseShader = new CocosDefaultShader(arrDiffuseAttrib, 3);
	m_pDiffuseShader->initShaderWithString(s_szDiffuseShader);


	MeshVertexAttrib arrDiffuseAttrib_Lit[] =
	{
		{ 3, 0, VertexData::VERTEX_ATTRIB_POSITION, 3 * sizeof(float)},
		{ 3, 0, VertexData::VERTEX_ATTRIB_NORMAL, 3 * sizeof(float)},
		{ 2, 0, VertexData::VERTEX_ATTRIB_TEX_COORD, 2 * sizeof(float)}
	};
	m_pDiffuseLitShader = new CocosDefaultShader(arrDiffuseAttrib_Lit, 3);
	m_pDiffuseLitShader->initShaderWithString(s_szDiffuseShader_Lit);


	MeshVertexAttrib arrDiffuseSkinAttrib[] =
	{
		{ 3, 0, VertexData::VERTEX_ATTRIB_POSITION, 3 * sizeof(float) },
		{ 3, 0, VertexData::VERTEX_ATTRIB_NORMAL, 3 * sizeof(float) },
		{ 2, 0, VertexData::VERTEX_ATTRIB_TEX_COORD, 2 * sizeof(float) },
		{ 4, 0, VertexData::VERTEX_ATTRIB_BLEND_WEIGHT, 4 * sizeof(float) },
		{ 4, 0, VertexData::VERTEX_ATTRIB_BLEND_INDEX, 4 * sizeof(float) }
	};
	m_pDiffuseShaderSkin = new CocosDefaultShader(arrDiffuseSkinAttrib, 5);
	m_pDiffuseShaderSkin->initShaderWithString(s_szDiffuseSkinShader);


	MeshVertexAttrib arrDiffuseSkinAttrib_Lit[] =
	{
		{ 3, 0, VertexData::VERTEX_ATTRIB_POSITION, 3 * sizeof(float) },
		{ 3, 0, VertexData::VERTEX_ATTRIB_NORMAL, 3 * sizeof(float) },
		{ 2, 0, VertexData::VERTEX_ATTRIB_TEX_COORD, 2 * sizeof(float) },
		{ 4, 0, VertexData::VERTEX_ATTRIB_BLEND_WEIGHT, 4 * sizeof(float) },
		{ 4, 0, VertexData::VERTEX_ATTRIB_BLEND_INDEX, 4 * sizeof(float) }
	};
	m_pDiffuseLitShaderSkin = new CocosDefaultShader(arrDiffuseSkinAttrib_Lit, 5);
	m_pDiffuseLitShaderSkin->initShaderWithString(s_szDiffuseSkinShader_Lit);

	m_pDiffuseShaderSkin_NPR = new CocosDefaultShader(arrDiffuseSkinAttrib_Lit, 5);
	m_pDiffuseShaderSkin_NPR->initShaderWithString(s_szDiffuseSkinShader_NPR);
}

NS_CC_END