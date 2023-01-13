#ifndef CLUTFIlter_H
#define CLUTFIlter_H

#include "EffectKernel/CEffectPart.h"
#include <memory>

class CCProgram;
class MaterialTexRHI;

class CLUTFIlterGL: public CEffectPart
{
public:
    CLUTFIlterGL();
    virtual ~CLUTFIlterGL();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam& RenderParam);

    std::shared_ptr<MaterialTexRHI> m_material;  //美白
    std::shared_ptr<MaterialTexRHI> m_materialAdjust;//调色
    std::shared_ptr<MaterialTexRHI> m_materialClear;//清晰
    std::shared_ptr<MaterialTexRHI> m_materialType;//风格滤镜

	float m_alphaAdjust = 0.0;
	float m_alphaClear = 0.0;
	float m_alphaType = 0.0;
private:
    std::shared_ptr<CCProgram> m_pShader;
};

#endif // CLUTFIlter_H
