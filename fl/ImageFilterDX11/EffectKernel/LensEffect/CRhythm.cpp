#include "CRhythm.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"

CRhythm::CRhythm()
{
    //m_material = 0;
    m_alpha = 0.0;
	m_EffectPart = RHYTHM_EFFECT;
}


CRhythm::~CRhythm()
{
    Release();
}

void CRhythm::Release()
{
}

void * CRhythm::Clone()
{
    CRhythm* result = new CRhythm();
    *result = *this;
    return result;
}

bool CRhythm::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CRhythm::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CRhythm::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};
	std::string  path = m_resourcePath + "/Shader/LensEffect/Rhythm.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);


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



void CRhythm::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha<0.01)
	{
		return;
	}
	Vector2 CenterPoint = Vector2(0.5, 0.5);
	float Scale = 1;

	if (RenderParam.GetFaceCount() == 0)
	{
		Scale = 1;
		CenterPoint = Vector2(0.5, 0.5);
		m_Stable = false;
	}
	else
	{
		if (m_RealScale < MaxScale&&!m_Stable)
		{
			Scale = MaxScale+0.1;
		}
		else
		{
			Scale = MidScale;
			m_Stable = true;
		}

		FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(0);
		CenterPoint.x = pFaceInfo->pFaceRect.x + pFaceInfo->pFaceRect.width*0.5;
		CenterPoint.y = pFaceInfo->pFaceRect.y + pFaceInfo->pFaceRect.height*0.5;
		CenterPoint.x /= RenderParam.GetWidth();
		CenterPoint.y /= RenderParam.GetHeight();
	}
	m_RealScale = m_RealScale * (1 - ScaleSmooth) + Scale * ScaleSmooth;
	m_RealCenterPoint = m_RealCenterPoint * (1 - PosSmooth) + CenterPoint * PosSmooth;

	//fix center
	float Radius = 0.5 / m_RealScale;
	if (m_RealCenterPoint.x - Radius<0)
	{
		m_RealCenterPoint.x = Radius;
	}
	if (m_RealCenterPoint.x+Radius>1)
	{
		m_RealCenterPoint.x = 1 - Radius;
	}
	if (m_RealCenterPoint.y - Radius < 0)
	{
		m_RealCenterPoint.y = Radius;
	}
	if (m_RealCenterPoint.y + Radius > 1)
	{
		m_RealCenterPoint.y = 1 - Radius;
	}

	float pParam[4];
	pParam[0] =1/m_RealScale;
	pParam[1] = m_RealCenterPoint.x;
	pParam[2] = m_RealCenterPoint.y;



	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SwapFBO();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();
	SetParameter("param", pParam, 0, sizeof(Vector4));
	pDoubleBuffer->SetBShaderResource(0);
	GET_SHADER_STRUCT_MEMBER(ConstantBufferVec4).ApplyToAllBuffer();
	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
}

bool CRhythm::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("param", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szAlpha = nodeDrawable.getAttribute("MaxScale");
			if (szAlpha != NULL)
			{
				MaxScale = atof(szAlpha);
			}
			szAlpha = nodeDrawable.getAttribute("MidScale");
			if (szAlpha != NULL)
			{
				MidScale = atof(szAlpha);
			}
			szAlpha = nodeDrawable.getAttribute("ScaleSmooth");
			if (szAlpha != NULL)
			{
				ScaleSmooth = atof(szAlpha);
			}
			const char * szLoop = nodeDrawable.getAttribute("PosSmooth");
			if (szLoop != NULL)
			{
				PosSmooth = atof(szLoop);
			}
		}
		return true;
	}
	return false;
}

bool CRhythm::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
// 	//copy image or animation
// 	XMLNode nodeEffect = root.addChild("typeeffect");
// 	nodeEffect.addAttribute("type", "Soulbody");
// 	nodeEffect.addAttribute("showname", m_showname.c_str());
// 	XMLNode nodeDrwable = nodeEffect.addChild("param");
// 	
// 	char alpha[256];
// 	sprintf(alpha, "%.2f", m_alpha);
// 	nodeDrwable.addAttribute("alpha", alpha);
// 
// 	sprintf(alpha, "%.2f", m_Alltime);
// 	nodeDrwable.addAttribute("Alltime", alpha);
// 
// 	sprintf(alpha, "%.2f", m_DuringTime);
// 	nodeDrwable.addAttribute("Onetime", alpha);
// 
// 	sprintf(alpha, "%d", m_Loop);
// 	nodeDrwable.addAttribute("Loopnum", alpha);

	return true;
}

void CRhythm::SetEffectCount(int count)
{
// 	m_Loop = count;
// 	m_StartTime = GetRunTime();
}