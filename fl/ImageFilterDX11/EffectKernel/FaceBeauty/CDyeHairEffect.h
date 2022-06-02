#ifndef CDyeHair_H____
#define CDyeHair_H____

#include "EffectKernel/CEffectPart.h"
class MAGICSTUDIO_EXPORTS_CLASS CDyeHairEffect: public CEffectPart
{
public:
	CDyeHairEffect();
    virtual ~CDyeHairEffect();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode & childNode, const std::string & path);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);

	Vector4 m_HairRGBA = Vector4(0.5f, 0.08f, 1.0f, 0.25f);
private:
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	bool IsEyeOpen(Vector2 *pFacePoint, float param);

	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;
	ID3D11BlendState *m_pBlendState;

	Vector3 m_HairColor[4] = { Vector3(0.5, 0.08, 1.0), Vector3(1.0,0.62745,0.0), Vector3(1.0,0.0,0.0), Vector3(0.0,0.0,1.0)};
	int m_CountColor = 0;
};

#endif // CLUTFIlter_H
