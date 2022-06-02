#pragma once
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"


class CFaceEffect2DBlendBG : public CEffectPart
{
public:
	CFaceEffect2DBlendBG();
	virtual ~CFaceEffect2DBlendBG();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);


private:

	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL, const std::string &path = "");
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	void MergeVertex(float *pVertex, float *pUV, int nVertex);

	
	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_IndexBuffer;
	ID3D11Buffer*       m_pConstantBuffer;

	mt3dface::MTFace2DInterFace *m_2DInterFace;


	float *m_pMergeVertex;

	bool m_isFirstInit;
};

