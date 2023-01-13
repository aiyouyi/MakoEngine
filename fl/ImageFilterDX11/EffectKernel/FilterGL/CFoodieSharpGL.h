#ifndef CFoodieSharp_H
#define CFoodieSharp_H

#include "EffectKernel/CEffectPart.h"
#include <memory>

class CCProgram;
class CCFrameBuffer;

class CFoodieSharpGL: public CEffectPart
{
public:
    CFoodieSharpGL();
    virtual ~CFoodieSharpGL();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);
private:
    std::shared_ptr<CCProgram> m_pShaderSharp;
    std::shared_ptr<CCProgram> m_pShader;
	CCFrameBuffer *m_pFBO;
	int m_nWidth;
	int m_nHeight;
};

#endif // CFoodieSharp_H
