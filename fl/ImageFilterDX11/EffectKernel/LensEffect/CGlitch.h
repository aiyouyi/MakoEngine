#ifndef CGiltch_H____
#define CGiltch_H____

#include "EffectKernel/CEffectPart.h"
class MAGICSTUDIO_EXPORTS_CLASS CGlitch: public CEffectPart
{
public:
	CGlitch();
    virtual ~CGlitch();
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
