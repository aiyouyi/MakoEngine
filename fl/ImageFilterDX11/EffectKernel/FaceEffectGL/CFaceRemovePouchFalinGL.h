#ifndef CFACEREMOVEPOUCHFALIN_H
#define CFACEREMOVEPOUCHFALIN_H

#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include <memory>

class GLDoubleBuffer;
class CCFrameBuffer;

class CFaceRemovePouchFalinGL : public CEffectPart
{
public:
    CFaceRemovePouchFalinGL();
    virtual ~CFaceRemovePouchFalinGL();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);
	GLuint m_material;
	float m_pouchAlpha = 0.0;
	float m_BrightEyeAlpha = 0.0;
private:

    std::shared_ptr<class CCProgram> m_pShaderSmooth;
    std::shared_ptr<class CCProgram> m_pShaderMean;
    std::shared_ptr<class CCProgram> m_pShader;
    GLDoubleBuffer* m_DoubleBuffer;

    CCFrameBuffer *m_pFBO;
    int m_nWidth;
    int m_nHeight;

    mt3dface::MTFace2DInterFace *m_2DInterFace;

	float m_ScaleHW = 0.25;
    
};

#endif // CFACESMOOTHINS_H
