#include "CCopyTex.h"

CCopyTex::CCopyTex()
{

}


CCopyTex::~CCopyTex()
{
	
	Release();
}

void * CCopyTex::Clone()
{
	CCopyTex* result = new CCopyTex();
	*result = *this;
	return result;
}

bool CCopyTex::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	return true;
}

bool CCopyTex::Prepare()
{

	return true;
}

void CCopyTex::Render(BaseRenderParam & RenderParam)
{
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SyncAToB();
}

void CCopyTex::Release()
{

}
