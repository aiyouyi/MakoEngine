#include "CBodyLiftYY.h"
#include "common.h"
#include <BaseDefine/Vectors.h>
#include "Toolbox/DXUtils/DX11Resource.h"

CBodyLiftYY::CBodyLiftYY()
{
	m_EffectPart = BODY_LIFT_EFFECT;
	m_alpha = 0.0;
}


CBodyLiftYY::~CBodyLiftYY()
{
	
	Release();
}

void * CBodyLiftYY::Clone()
{
	CBodyLiftYY* result = new CBodyLiftYY();
	*result = *this;
	return result;
}

bool CBodyLiftYY::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	return true;
}

bool CBodyLiftYY::Prepare()
{

	return true;
}

void CBodyLiftYY::Render(BaseRenderParam & RenderParam)
{
	if (m_alpha < 0.001f&&m_alpha>-0.001f)
	{
		return;
	}

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	pDoubleBuffer->BindFBOB();

	auto dx11DoubleBuffer = RHIResourceCast(pDoubleBuffer.get());
	dx11DoubleBuffer->m_rectDraw->setShaderTextureView(dx11DoubleBuffer->GetFBOTextureA());

	if (m_alpha<0)
	{
		dx11DoubleBuffer->m_rectDraw->render(Vector2(0,0), Vector2(1.0, 1.0-0.2*m_alpha), 0, pDoubleBuffer->GetWidth(), pDoubleBuffer->GetHeight());
	}
	else
	{
		dx11DoubleBuffer->GetFBOB()->clear(0, 1.0, 0, 0.0);
		dx11DoubleBuffer->m_rectDraw->render(Vector2(0, 0), Vector2(1.0 + 0.2*m_alpha,1.0), 0, pDoubleBuffer->GetWidth(), pDoubleBuffer->GetHeight());
	}
	pDoubleBuffer->SwapFBO();

}

void CBodyLiftYY::Release()
{

}
