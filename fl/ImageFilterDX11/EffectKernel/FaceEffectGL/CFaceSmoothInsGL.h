#ifndef CFACESMOOTHINS_H
#define CFACESMOOTHINS_H

#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include <memory>

class GLDoubleBuffer;
class CCFrameBuffer;

class CFaceSmoothInsGL: public CEffectPart
{
public:
    CFaceSmoothInsGL();
    virtual ~CFaceSmoothInsGL();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam& RenderParam);
	float m_TestAlpha[3] = { 1.0,1.0,1.0 };
private:

    void FilterToSkinFBO(GLuint tex,int nWidth, int nHeight);

    void FilterToFaceFBO(BaseRenderParam& RenderParam,int nWidth, int nHeight);

    std::shared_ptr<class CCProgram> m_pShader;
    std::shared_ptr<class CCProgram> m_pShaderguide;
    std::shared_ptr<class CCProgram> m_pShaderSmooth;
    std::shared_ptr<class CCProgram> m_pShaderMean;
    std::shared_ptr<class CCProgram> m_pShaderFace;
    std::shared_ptr<class CCProgram> m_pShaderSkin;

    GLDoubleBuffer *m_DoubleBuffer;

    CCFrameBuffer *m_pFBO;
    CCFrameBuffer *m_pFBOFace;
   
    int m_nWidth;
    int m_nHeight;

    mt3dface::MTFace2DInterFace *m_2DInterFace;
    std::shared_ptr<class MaterialTexRHI> m_material;
	float m_ScaleHW = 0.25;
	float m_rateW = 1.0;
	float m_rateH = 1.0;

	float m_BackAlpha = 0.2;
};

#endif // CFACESMOOTHINS_H
