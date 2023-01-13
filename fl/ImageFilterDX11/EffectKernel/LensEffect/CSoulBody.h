#ifndef CSoulBody_H____
#define CSoulBody_H____

#include "EffectKernel/CEffectPart.h"
#include "EffectKernel/DXBasicSample.h"

class MAGICSTUDIO_EXPORTS_CLASS CSoulBody: public CEffectPart, protected DXBaicSample
{
public:
	CSoulBody();
    virtual ~CSoulBody();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode & childNode, const std::string & path);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);

	void SetEffectCount(int count);
	float m_Alltime = 2000;
	float m_DuringTime = 500;
	int m_Loop = 0;
	long m_StartTime = 0;
private:
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;

};

#endif // CLUTFIlter_H
