#include "CFilterWithMask.h"
#include "common.h"
#include <xnamath.h>
#include<BaseDefine/Vectors.h>
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

struct RectConstantFilterMask
{
	XMMATRIX mWVP; //混合矩阵
};

CFilterWithMask::CFilterWithMask()
{
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBufferMask = NULL;
}


CFilterWithMask::~CFilterWithMask()
{
	
	Release();
}

void * CFilterWithMask::Clone()
{
	CFilterWithMask* result = new CFilterWithMask();
	*result = *this;
	return result;
}

bool CFilterWithMask::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	return true;
}

bool CFilterWithMask::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
	};
	string path = m_resourcePath + "/Shader/FilterWithMask.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	float vertices[] =
	{
		-1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,0.0f, 1.0f,
		1.0f, -1.0f, 0.0f,1.0f, 1.0f,
		1, 1.0f, 0.0f ,1.0f, 0.0f
	};

	m_rectVerticeBuffer = DXUtils::CreateVertexBuffer(vertices, 4, 5);

	unsigned short index[] =
	{
		0, 1, 2,
		0, 2, 3
	};
	m_rectIndexBuffer = DXUtils::CreateIndexBuffer(index, 2);

	m_pConstantBufferMask = DXUtils::CreateConstantBuffer(sizeof(RectConstantFilterMask));
	return true;
}

void CFilterWithMask::Render(BaseRenderParam & RenderParam)
{
	auto BodyTexture = RenderParam.GetBodyMaskTexture();
	if (BodyTexture == NULL)
	{
		return;
	}

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();


	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOA();

	m_pShader->useShader();
	auto pMaskView = RHIResourceCast(BodyTexture.get())->GetSRV();
	auto pSrcShaderView = RHIResourceCast(RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB().get())->GetSRV();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMaskView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

		float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

	RectConstantFilterMask mWVP;
	mWVP.mWVP = XMMatrixIdentity();
	DeviceContextPtr->UpdateSubresource(m_pConstantBufferMask, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBufferMask);


	//设置顶点数据
	unsigned int nStride = 5 * sizeof(float);
	unsigned int nOffset = 0;
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

}

void CFilterWithMask::Release()
{
	SAFE_RELEASE_BUFFER(m_pConstantBufferMask);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
}
