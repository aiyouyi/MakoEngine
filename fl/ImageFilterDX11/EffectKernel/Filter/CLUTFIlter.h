#ifndef CLUTFIlter_H
#define CLUTFIlter_H

#include "EffectKernel/CEffectPart.h"
class CLUTFIlter: public CEffectPart
{
public:
    CLUTFIlter();
    virtual ~CLUTFIlter();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode & childNode, const std::string & path);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);
private:
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;
	DX11FBO *m_pFBO =NULL;
	ID3D11Texture2D *m_pTargetTexture = NULL;

};

#endif // CLUTFIlter_H
