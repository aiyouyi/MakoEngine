#pragma once

#include "EffectKernel/CEffectPart.h"
#include "Toolbox/GL/CCEffectModel.hpp"

class Drawable;

class CStickerEffectGL: public CEffectPart
{
public:
    CStickerEffectGL();
    virtual ~CStickerEffectGL();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	virtual void Release();

private:
    std::shared_ptr<class CCProgram> m_pShader;
    std::shared_ptr<class CCProgram> m_pShaderHead;
    std::shared_ptr<class CCProgram> m_pShader2D;
    std::vector<CCEffectModel*> m_vEffectModel;
    std::vector<CCEffect2DRect*> m_v2DEffectModel;
    CCHeaderModel m_vHeaderModel;
    long m_nAniLoopTime;

    std::map<std::string, Drawable*>m_mapImage;
};

