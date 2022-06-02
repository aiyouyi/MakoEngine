#include "CFaceEffectFlipSplit.h"
#include "CFaceEffect2DWithBG.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/Render/DynamicRHI.h"
//#include "opencv2/opencv.hpp"



CFaceEffectFlipSplit::CFaceEffectFlipSplit()
{
	m_pConstantBuffer = NULL;
	m_alpha = 1.0;
	m_EffectPart = FlipSplit;
}


CFaceEffectFlipSplit::~CFaceEffectFlipSplit()
{
	Release();
}

void CFaceEffectFlipSplit::Release()
{
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
}

void* CFaceEffectFlipSplit::Clone()
{
	CFaceEffectFlipSplit* result = new CFaceEffectFlipSplit();
	*result = *this;
	return result;
}

bool CFaceEffectFlipSplit::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath)
{
	return true;
}

bool CFaceEffectFlipSplit::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	string path = m_resourcePath + "/Shader/face2dFlipSplit.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	Vector3 arrCoords[4] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };
	unsigned short index[] =
	{
		0, 1, 2,
		1, 2, 3
	};
	mVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, 4, 3);
	mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);
	return true;
}

void CFaceEffectFlipSplit::Render(BaseRenderParam& RenderParam)
{
	//test
	/*mControlParam.x	 = PI /2;
	mControlParam.y = 1240.0f / 930.0f;
	mControlParam.z = 3;
	mControlParam.w = 15;
	if (m_alpha < 0.001f)
	{
		return;
	}*/

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOB();
	m_pShader->useShader();
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &mControlParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
	pDoubleBuffer->SwapFBO();
}

