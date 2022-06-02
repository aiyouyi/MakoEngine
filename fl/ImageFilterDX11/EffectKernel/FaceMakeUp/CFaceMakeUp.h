#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/FaceMeshManage.h"
#include "Toolbox/Drawable.hpp"

struct MakeUpInfo
{
	FaceMeshType m_meshType;
	Drawable *m_drawable;

	float alpha = 1.0;
};

class DX11IMAGEFILTER_EXPORTS_CLASS CFaceMakeUp :	public CEffectPart
{
public:
	CFaceMakeUp();
	~CFaceMakeUp();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);
	virtual bool WriteConfig(XMLNode& root, HZIP dst, HZIP src = 0) override;
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

	vector<MakeUpInfo>m_vMeshType;

	vector<MakeUpInfo>& getMakeUpInfo() {
		return m_vMeshType;
	}

	static CFaceMakeUp* createEffect();

private:
	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL, const std::string &path = "");

	void ReleaseMakeUpInfo();
	void MergeVertex(float *pVertex, float *pUV, int nVertex);

	ID3D11Buffer *m_VerticeBuffer[MAX_FACE_TYPE_NUM][MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_IndexBuffer[MAX_FACE_TYPE_NUM];
	ID3D11Buffer*       m_pConstantBuffer;

	ID3D11BlendState *m_pBlendStateMulity;

	bool m_isCrop;

	FaceMeshManage *m_FaceMeshManage;

	float *m_pMergeVertex;

	bool m_isFirstInit;

};

