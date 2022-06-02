#ifndef CFACEREMOVEPOUCHFALIN_H
#define CFACEREMOVEPOUCHFALIN_H

#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"

class CFaceRemovePouchFalin : public CEffectPart
{
public:
    CFaceRemovePouchFalin();
    virtual ~CFaceRemovePouchFalin();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode & childNode, const std::string & path);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);
	float m_pouchAlpha = 0.0;
	float m_BrightEyeAlpha = 0.0;
	float m_FilterRadis = 2.5;
private:
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	void  MergeVertex(float * pVertex, float * pUV, int nVertex);
	DX11Shader *m_pShaderSmooth = nullptr;
	DX11Shader *m_pShaderMean = nullptr;
	DX11DoubleBuffer *m_DoubleBuffer = nullptr;

	ID3D11Buffer *m_rectVerticeBuffer = nullptr;
	ID3D11Buffer *m_rectIndexBuffer = nullptr;
	ID3D11Buffer* m_pConstantBuffer = nullptr;

	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_IndexBuffer = nullptr;

	DX11FBO *m_pFBO = nullptr;
    int m_nWidth;
    int m_nHeight;

    mt3dface::MTFace2DInterFace *m_2DInterFace = nullptr;
	float *m_pMergeVertex = nullptr;

	float m_ScaleHW = 0.25;
};

#endif // CFACESMOOTHINS_H
