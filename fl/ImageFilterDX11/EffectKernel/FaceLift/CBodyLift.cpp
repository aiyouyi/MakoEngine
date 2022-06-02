#include "CBodyLift.h"
#include "common.h"
#include <xnamath.h>
#include<BaseDefine/Vectors.h>

CBodyLift::CBodyLift()
{
	m_EffectPart = BODY_LIFT_EFFECT;
	m_alpha = 0.0;
}


CBodyLift::~CBodyLift()
{
	
	Release();
}

void * CBodyLift::Clone()
{
	CBodyLift* result = new CBodyLift();
	*result = *this;
	return result;
}

bool CBodyLift::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	return true;
}

bool CBodyLift::Prepare()
{

	return true;
}

void CBodyLift::Render(BaseRenderParam & RenderParam)
{
	if (m_alpha < 0.001f&&m_alpha>-0.001f)
	{
		return;
	}

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	pDoubleBuffer->BindFBOB();

	pDoubleBuffer->m_rectDraw->setShaderTextureView(pDoubleBuffer->GetFBOTextureA()->getTexShaderView());;
	if (m_alpha>0)
	{
		pDoubleBuffer->m_rectDraw->render(vec2(0,0), vec2(1.0, 1.0+0.2*m_alpha), 0, pDoubleBuffer->GetWidth(), pDoubleBuffer->GetHeight());
	}
	else
	{
		pDoubleBuffer->GetFBOB()->clear(0, 1.0, 0, 0.0);
		pDoubleBuffer->m_rectDraw->render(vec2(0, 0), vec2(1.0 + 0.2*m_alpha,1.0), 0, pDoubleBuffer->GetWidth(), pDoubleBuffer->GetHeight());
	}
	pDoubleBuffer->SwapFBO();

}

void CBodyLift::Release()
{

}
