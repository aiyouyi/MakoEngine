#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
class CFaceShadowHighLight : public CEffectPart
{
public:
	CFaceShadowHighLight();
	virtual ~CFaceShadowHighLight();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode & childNode, const std::string & path);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
private:
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	long long GetImageID(HZIP hZip, const std::string & path, const char *szDrawableName);
	void  MergeVertex(float * pVertex, float * pUV, int nVertex);

	long long m_NormalId = 0;
	long long m_HighLightID = 0 ;
	long long m_ShadowID = 0;

	mt3dface::MTFace2DInterFace *m_2DInterFace;

	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_IndexBuffer;
	ID3D11Buffer*       m_pConstantBuffer;

	float *m_pMergeVertex;
};

