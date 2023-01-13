#ifndef CFaceLevel_H
#define CFaceLevel_H

#include "EffectKernel/CEffectPart.h"
#include <memory>

class CCFrameBuffer;
class CFaceLevelGL: public CEffectPart
{
public:
    CFaceLevelGL();
    virtual ~CFaceLevelGL();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam& RenderParam);
private:
	CCFrameBuffer *m_pFBO;
    std::shared_ptr<class CCProgram> m_pShader;
	int m_nWidth;
	int m_nHeight;
};

#endif // CFaceLevel_H
