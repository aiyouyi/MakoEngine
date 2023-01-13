#include "CBodyLift2.h"
#include "common.h"
#include <xnamath.h>
#include <BaseDefine/Vectors.h>
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/Render/RenderTargetRHI.h"
#include "Toolbox/Render/VertexBuffer.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/DXUtils/DX11Resource.h"

CBodyLift2::CBodyLift2()
{
	m_EffectPart = BODY_LIFT_EFFECT;
	m_alpha = 0.0;
}


CBodyLift2::~CBodyLift2()
{
	
	Release();
}

void * CBodyLift2::Clone()
{
	CBodyLift2* result = new CBodyLift2();
	*result = *this;
	return result;
}

bool CBodyLift2::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("mask");
			if (szDrawableName != NULL)
			{
				m_FaceMask = GetDynamicRHI()->CreateTextureFromZip(hZip, szDrawableName, false);
			}
		}
		return true;
	}
	return false;
}

bool CBodyLift2::Prepare()
{

	//´´½¨shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
	};

	std::string path = m_resourcePath + "/Shader/Bodylift.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2,true);

	CCVetexAttribute pAttribute2[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	path = m_resourcePath + "/Shader/face2dTexture.fx";
	m_pShaderFace = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute2, 2,true);


	unsigned short index[] =
	{
	   9,5,6,5,9,8,1,2,5,3,0,7,7,0,4,5,2,6,7,4,8,8,9,7
	};
	mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 8);
	m_InterFM = std::make_shared<InterFacePointDeform>();
	return true;
}

void CBodyLift2::Render(BaseRenderParam & RenderParam)
{
	if (m_alpha < 0.001f&&m_alpha>-0.001f)
	{
		return;
	}
	FilterToFaceFBO(RenderParam, RenderParam.GetWidth(), RenderParam.GetHeight());

	const int nPoint = 10;
	Vector2 pSrcPoint[nPoint];
	Vector2 pDstPoint[nPoint];

	pSrcPoint[0] = pDstPoint[0] = Vector2(0, 0);
	pSrcPoint[1] = pDstPoint[1] = Vector2(1, 0);
	pSrcPoint[2] = pDstPoint[2] = Vector2(1, 1);
	pSrcPoint[3] = pDstPoint[3] = Vector2(0, 1);
	pSrcPoint[8] = pDstPoint[8] = Vector2(m_Center.x, 0);
	pSrcPoint[9] = pDstPoint[9] = Vector2(m_Center.x, 1);

	pSrcPoint[4] = Vector2(0.01, 0);
	pSrcPoint[5] = Vector2(0.99, 0);
	pSrcPoint[6] = Vector2(0.99, 1);
	pSrcPoint[7] = Vector2(0.01, 1);
	float LiftAlpha = m_alpha * 0.06;
	pDstPoint[4] = Vector2(0.01 + LiftAlpha, 0);
	pDstPoint[5] = Vector2(0.99 - LiftAlpha, 0);
	pDstPoint[6] = Vector2(0.99 - LiftAlpha, 1);
	pDstPoint[7] = Vector2(0.01 + LiftAlpha, 1);
	for (int i=0;i< nPoint;i++)
	{
		pDstPoint[i] = pDstPoint[i] * 2.0 - Vector2(1.0, 1.0);
	}


	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	pDoubleBuffer->BindFBOB();

	m_pShader->useShader();
	RHIResourceCast(pDoubleBuffer.get())->SetAShaderResource(0);
	GetDynamicRHI()->SetPSShaderResource(1, m_pFBOFace);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);
	if (!mVertexBuffer[0])
	{
		mVertexBuffer[0] = GetDynamicRHI()->CreateVertexBuffer((float*)pDstPoint, nPoint, 2);
	}
	else
	{
		mVertexBuffer[0]->UpdateVertexBUffer((float*)pDstPoint, nPoint, 2 * sizeof(float), 2 * sizeof(float));
	}

	if (!mVertexBuffer[1])
	{
		mVertexBuffer[1] = GetDynamicRHI()->CreateVertexBuffer((float*)pSrcPoint, nPoint, 2);
	}
	else
	{
		mVertexBuffer[1]->UpdateVertexBUffer((float*)pSrcPoint, nPoint, 2 * sizeof(float), 2 * sizeof(float));
	}


	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, 2, mIndexBuffer);

	pDoubleBuffer->SwapFBO();

}

void CBodyLift2::Release()
{

}

void CBodyLift2::FilterToFaceFBO(BaseRenderParam & RenderParam, int nWidth, int nHeight)
{
	if (m_nWidth != nWidth || m_nHeight != nHeight)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_pFBOFace = GetDynamicRHI()->CreateRenderTarget(nWidth*0.25, nHeight*0.25, false, nullptr, CC3DTextureRHI::SFT_A8R8G8B8);
	}

	int nFaceCount = RenderParam.GetFaceCount();
	if (nFaceCount < 1)
	{
		return;
	}
	m_pFBOFace->Bind();
	m_pFBOFace->Clear(0, 0.0, 0.0, 0.0);
	m_pShaderFace->useShader();
	GetDynamicRHI()->SetPSShaderResource(0, m_FaceMask);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);
	for (int faceIndex = 0; faceIndex < 1; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_130);
		m_Center = pFacePoint[32] * 0.5 + pFacePoint[0] * 0.5;
		m_Center.x /= m_nWidth;
		m_Center.y /= m_nHeight;

		Vector2 pSrcpoint442[442];
		m_InterFM->RunFace118To442(pFacePoint, 0, pSrcpoint442);
		for (int i = 0; i < 442; i++) {
			pSrcpoint442[i].x = pSrcpoint442[i].x / m_nWidth;
			pSrcpoint442[i].y = pSrcpoint442[i].y / m_nHeight;
			pSrcpoint442[i] = pSrcpoint442[i] * 2.0 - 1.0;
		}
		if (!mVertexBuffer2[0])
		{
			mVertexBuffer2[0] = GetDynamicRHI()->CreateVertexBuffer((float*)pSrcpoint442, 442, 2);
		}
		else
		{
			mVertexBuffer2[0]->UpdateVertexBUffer((float*)pSrcpoint442, 442, 2 * sizeof(float), 2 * sizeof(float));
		}

		if (!mVertexBuffer2[1])
		{
			mVertexBuffer2[1] = GetDynamicRHI()->CreateVertexBuffer((float*)m_InterFM->m_StandDstPoint, 442, 2);
		}
		if (!mIndexBuffer2)
		{
			mIndexBuffer2 = GetDynamicRHI()->CreateIndexBuffer(m_InterFM->m_StandIndex, 832);
		}

		GetDynamicRHI()->DrawPrimitive(mVertexBuffer2, 2, mIndexBuffer2);
	}
}
