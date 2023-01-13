#ifndef CDENOSIEFILTER_H
#define CDENOSIEFILTER_H

#include "EffectKernel/CEffectPart.h"
#include <memory>
#define NUM_RECORD_FBO 6

class CCFrameBuffer;
class GLDoubleBuffer;
class CCProgram;

class CDeNosieFilterGL : public CEffectPart
{

public:
    CDeNosieFilterGL();

    virtual ~CDeNosieFilterGL();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);

private:

    void FilterKGAndCov(BaseRenderParam &RenderParam);

private:
    std::shared_ptr<CCProgram> m_pShader;
    std::shared_ptr<CCProgram> m_pShaderCov;

    GLuint m_RecordTex[NUM_RECORD_FBO];
    GLDoubleBuffer* m_LastCovDoubleBuffer;

    CCFrameBuffer *m_pFBO;
    CCFrameBuffer *m_pFBOLastResult;

    int m_nWidth,m_nHeight;
    int m_nFrame = 0;
};

#endif // CDENOSIEFILTER_H
