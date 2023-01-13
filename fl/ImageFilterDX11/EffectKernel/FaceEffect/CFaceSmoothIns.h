#ifndef CFACESMOOTHINS_H
#define CFACESMOOTHINS_H

#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include "EffectKernel/DXBasicSample.h"

class DX11Texture;
class DX11DoubleBuffer;
class DX11FBO;

class CFaceSmoothIns: public CEffectPart ,protected DXBaicSample
{
public:
    CFaceSmoothIns();
    virtual ~CFaceSmoothIns();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string& path);
	virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);
	float m_TestAlpha[3] = { 1.0,1.0,1.0 };

	void setAnim(AnimInfo& info) override;
	inline void setBackAlpha(float value) noexcept { m_BackAlpha = value; }
	inline float getBackAlpha() { return m_BackAlpha; }
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
private:

    void FilterToSkinFBO(std::shared_ptr<CC3DTextureRHI> tex,int nWidth, int nHeight);

    void FilterToFaceFBO(BaseRenderParam &RenderParam,int nWidth, int nHeight);

	void  MergeVertex(float * pVertex, float * pUV, int nVertex);

	DX11Shader *m_pShaderguide;
	DX11Shader *m_pShaderSmooth;
	DX11Shader *m_pShaderMean;
	DX11Shader *m_pShaderFace;
	DX11DoubleBuffer *m_DoubleBuffer;

	DX11FBO *m_pFBO;
	DX11FBO *m_pFBOFace;
	DX11Shader *m_pShaderSkin;
    int m_nWidth;
    int m_nHeight;

    mt3dface::MTFace2DInterFace *m_2DInterFace;
	DX11Texture *m_material;

	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;

	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_IndexBuffer;


	float *m_pMergeVertex;



	float m_ScaleHW = 0.25;

	float m_rateW = 1.0;
	float m_rateH = 1.0;

	float m_BackAlpha = 0.5;
};

#endif // CFACESMOOTHINS_H
