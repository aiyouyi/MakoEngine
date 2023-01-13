#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/FaceMeshManage.h"
#include "Toolbox/Drawable.hpp"
#include "EffectKernel/DXBasicSample.h"
#include "Toolbox/inc.h"

struct MakeUpBlendInfo
{
	FaceMeshType m_meshType;
	//Drawable *m_drawable;
	BlendType m_blendType;
	long long m_anim_id;
	float alpha = 0.0;
};

class DX11IMAGEFILTER_EXPORTS_CLASS CFaceBlendMakeUp : public CEffectPart, protected DXBaicSample
{
public:
	CFaceBlendMakeUp();
	~CFaceBlendMakeUp();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	virtual void SetAlpha(float Alpha);
	virtual void setAnim(AnimInfo &info);
	virtual void getAnim(AnimInfo &info);
	virtual void setAnimFps(float fps);
	CFaceBlendMakeUp * createEffect();
	std::vector<MakeUpBlendInfo>m_vMeshType;

	std::vector<MakeUpBlendInfo>& getMakeUpBlendInfo() {
		return m_vMeshType;
	}

	bool m_IsEyelidVisable = true;
private:
	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL, const std::string &path = "");
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
	void ReleaseMakeUpInfo();
	void MergeVertex(float *pVertex, float *pUV, int nVertex);

	FaceMeshType GetMeshType(const char *szType);

	ID3D11Buffer *m_VerticeBuffer[MAX_FACE_TYPE_NUM][MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_IndexBuffer[MAX_FACE_TYPE_NUM];
	ID3D11Buffer*       m_pConstantBuffer;

	ID3D11BlendState *m_pBlendStateMulity;

	bool m_isCrop;

	FaceMeshManage *m_FaceMeshManage;

	float *m_pMergeVertex;

	bool m_isFirstInit;

};

