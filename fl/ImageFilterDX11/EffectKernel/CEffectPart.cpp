#include "CEffectPart.h"
#include "ImageInfo.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "ResourceManager.h"
#include "FileManager.h"
#include "..\3rdparty\SplitScreenDetectSDK\include\SplitScreenDetectDef.h"

CEffectPart::CEffectPart()
{
}


CEffectPart::~CEffectPart()
{
	Release();
}

void * CEffectPart::Clone()
{
	CEffectPart* result = new CEffectPart();
	*result = *this;
	return result;
}

bool CEffectPart::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
{
	const char* showname = childNode.getAttribute("showname");
	if (NULL != showname)
	{
		m_showname = showname;
	}
	XMLNode nodeTime = childNode.getChildNode("time", 0);
	if (!nodeTime.isEmpty())
	{
		const char* szDelay = nodeTime.getAttribute("delay");
		const char* szPlay = nodeTime.getAttribute("play");
		const char* szAll = nodeTime.getAttribute("all");
		if (szDelay != NULL)
		{
			m_play.delaytime = atoi(szDelay);
		}
		if (szPlay != NULL)
		{
			m_play.playtime = atoi(szPlay);
		}

		if (szAll != NULL)
		{
			m_play.alltime = atoi(szAll);
		}


	}
	return true;
}

bool CEffectPart::ReadConfig(XMLNode & childNode,const std::string & path)
{
	const char* showname = childNode.getAttribute("showname");
	if (NULL != showname)
	{
		m_showname = showname;
	}
	XMLNode nodeTime = childNode.getChildNode("time", 0);
	if (!nodeTime.isEmpty())
	{
		const char* szDelay = nodeTime.getAttribute("delay");
		const char* szPlay = nodeTime.getAttribute("play");
		const char* szAll = nodeTime.getAttribute("all");
		if (szDelay != NULL)
		{
			m_play.delaytime = atoi(szDelay);
		}
		if (szPlay != NULL)
		{
			m_play.playtime = atoi(szPlay);
		}

		if (szAll != NULL)
		{
			m_play.alltime = atoi(szAll);
		}


	}

	return true;
}

bool CEffectPart::WriteConfig(XMLNode& root, HZIP dst, HZIP src) {
	return true;
}

bool CEffectPart::WriteConfig(std::string &tempPath, XMLNode& root, HZIP dst, HZIP src) {
	return true;
}

bool CEffectPart::Prepare()
{

	return true;
}

void CEffectPart::Render(BaseRenderParam &RenderParam)
{
}

void CEffectPart::SetAlpha(float Alpha)
{
	m_alpha = Alpha;
}

void CEffectPart::Release()
{
	ResourceManager::Instance().freeMaterial(m_anim_id);
}

long CEffectPart::GetRunTime()
{
	long runTime = m_runTime;
	if (m_play.alltime >0)
	{
		runTime = runTime % m_play.alltime;
	}
	runTime = runTime - m_play.delaytime;
	if (runTime >m_play.playtime && m_play.playtime != -1)
	{
		return -1;
	}
	return runTime;
}

BlendType CEffectPart::GetBlendType(const char *szBlendType)
{
	if (szBlendType != NULL)//正常
	{
		auto it = m_BlendMap.find(szBlendType);
		if (it != m_BlendMap.end())
		{
			return (*it).second;
		}
		/*return m_BlendMap[szBlendType];*/
	}
	else
	{
		return NORMAL_TYPE;
	}
}

float CEffectPart::GetBlendParm(BlendType &BlendType)
{
	if (BlendType == NORMAL_TYPE)//正常
	{
		m_BlendName = "Normal";
		return 0.0;
	}
	else if (BlendType == MULTIPLY_TYPE)//正片叠底
	{
		m_BlendName = "Multiply";
		return 1.0;
	}
	else if (BlendType == SOFTLIGHT_TYPE)//柔光
	{
		m_BlendName = "Softlight";
		return 2.0;
	}
	else if (BlendType == SCREEN_TYPE)//滤色
	{
		m_BlendName = "Screen";
		return 3.0;
	}
	else if (BlendType == DARKEN_TYPE)//变暗
	{
		m_BlendName = "Darken";
		return 4.0;
	}
	else if (BlendType == COLOR_BURN_TYPE)//颜色加深
	{
		m_BlendName = "Colorburn";
		return 5.0;
	}
	else if (BlendType == LINEAR_BURN_TYPE)//线性加深
	{
		m_BlendName = "Linearburn";
		return 6.0;
	}
	else if (BlendType == DARKER_COLOR_TYPE)//深色
	{
		m_BlendName = "Darkercolor";
		return 7.0;
	}
	else if (BlendType == LIGHTEN_TYPE)//变亮
	{
		m_BlendName = "Lighten";
		return 8.0;
	}
	else if (BlendType == COLOR_DODGE_TYPE)//颜色减淡
	{
		m_BlendName = "Colordodge";
		return 9.0;
	}
	else if (BlendType == LINEAR_DODGE_TYPE)//线性减淡
	{
		m_BlendName = "Lineardodge";
		return 10.0;
	}
	else if (BlendType == LIGHTER_COLOR_TYPE)//浅色
	{
		m_BlendName = "Lightercolor";
		return 11.0;
	}
	else if (BlendType == OVERLAY_TYPE)//叠加
	{
		m_BlendName = "Overlay";
		return 12.0;
	}
	else
	{
		m_BlendName = "Normal";
		return 0.0;
	}
}

const char* CEffectPart::getShowName() 
{
	return m_showname.c_str();
}

void CEffectPart::setShowName(const char* showname)
{
	m_showname = showname;
}

void CEffectPart::createEffect(char * path, const char * type, CEffectPart * part)
{
	HZIP hZip = OpenZip(path, nullptr);
	if (hZip != nullptr) {
		ZIPENTRY ze;
		int index;
		if (ZR_OK == FindZipItem(hZip, "test.xml", true, &index, &ze)) {
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res != ZR_OK) {
				delete[]pDataBuffer;
				CloseZip(hZip);
				return;
			}

			XMLResults xResults;
			XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "models", &xResults);
			delete[]pDataBuffer;

			XMLNode nodeEffect = nodeModels.getChildNode("typeeffect");
			const char *szType = nodeEffect.getAttribute("type");
			if (szType != NULL && !strcmp(szType, type))
			{
				part->ReadConfig(nodeEffect, hZip, path);
				part->Prepare();
			}
		}
	}
	CloseZip(hZip);
}

void CEffectPart::SetResourcePath(string & path)
{
	m_resourcePath = path;
}

void CEffectPart::setAnim(AnimInfo &info)
{
	ResourceManager::Instance().freeAnim(m_anim_id);
	m_anim_id = ResourceManager::Instance().loadAnim(info);
}
void CEffectPart::getAnim(AnimInfo &info)
{
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	info = anim->info;
}
void CEffectPart::setAnimFps(float fps)
{
	ResourceManager::Instance().SetAnimFps(m_anim_id, fps);
}
