#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/GL/CCEffectModel.hpp"
#include "BaseDefine/commonFunc.h"
#include <memory>

class CFaceEffect3DModelGL : public CEffectPart
{
public:
	CFaceEffect3DModelGL();
	~CFaceEffect3DModelGL();

	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

private:
	bool loadFromXML(HZIP hZip, const char *szFilePath, const char *szFileXml);

    std::vector<CCEffectModel*> m_vEffectModel;

	long m_nAniLoopTime;

	//渲染模型信息
	std::shared_ptr<class CCProgram> m_pShaderForHeaderCull;
	std::shared_ptr<class CCProgram> m_pShader;

	std::map<std::string, Drawable*>m_mapImage;


};
