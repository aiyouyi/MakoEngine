#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include <memory>

class MaterialTexRHI;
class CFaceShadowHighLightGL : public CEffectPart
{
public:
	CFaceShadowHighLightGL();
	virtual ~CFaceShadowHighLightGL();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam& RenderParam);
private:
	std::shared_ptr<class CCProgram> m_pShader;
	mt3dface::MTFace2DInterFace *m_2DInterFace;
	std::shared_ptr<MaterialTexRHI> m_mask;
	std::shared_ptr<MaterialTexRHI> m_Normal;
	std::shared_ptr<MaterialTexRHI> m_Highlight;
	std::shared_ptr<MaterialTexRHI> m_Shadowdow;
    bool isFirst = true;
};

