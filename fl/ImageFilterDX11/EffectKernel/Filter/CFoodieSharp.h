#ifndef CFoodieSharp_H
#define CFoodieSharp_H

#include "EffectKernel/CEffectPart.h"
class CFoodieSharp: public CEffectPart
{
public:
    CFoodieSharp();
    virtual ~CFoodieSharp();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode & childNode, const std::string & path);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);

	float m_Radius = 0.002;
	float m_MaxSharp = 0.5;//0.5--5.0

	int m_nWidth;
	int m_nHeight;
private:
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	DX11Shader *m_pShaderSharp;
	DX11FBO *m_pFBO;


	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;
};

#endif // CFoodieSharp_H
