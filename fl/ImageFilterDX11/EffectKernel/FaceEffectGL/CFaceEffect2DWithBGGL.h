#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include <memory>

class MaterialTexRHI;
class CFaceEffect2DWithBGGL : public CEffectPart
{
public:
	CFaceEffect2DWithBGGL();
	virtual ~CFaceEffect2DWithBGGL();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
private:
	mt3dface::MTFace2DInterFace *m_2DInterFace;
    std::shared_ptr<MaterialTexRHI> m_material;
	std::shared_ptr<class CCProgram> m_pShader;
};

