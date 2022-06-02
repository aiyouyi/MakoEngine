#ifndef CLIGHT_WAVE_H____
#define CLIGHT_WAVE_H____

#include "EffectKernel/CEffectPart.h"
class MAGICSTUDIO_EXPORTS_CLASS CLightWave: public CEffectPart
{
public:
	CLightWave();
    virtual ~CLightWave();
    virtual void Release();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode & childNode, const std::string & path);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);

	void SetEffectCount(int count);
	float m_Alltime = 3000;
	float m_DuringTime = 1000;
	int m_Loop = 0;
	long m_StartTime = 0;
	float m_ColorSpeed = 0.82;
private:
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;
	ID3D11BlendState *m_pBlendState;

	DX11Shader *m_pShaderSobel;
	DX11Shader * m_pShaderBlur;
	DX11FBO *m_pFBO;

	int m_nWidth = 0;
	int m_nHeight = 0;
	float m_ScaleHW = 0.25;
};

#endif // CLUTFIlter_H
