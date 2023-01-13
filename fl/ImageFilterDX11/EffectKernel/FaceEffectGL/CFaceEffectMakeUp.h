#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/FaceMeshManage.h"
#include <memory>

struct MakeUpInfo
{
	FaceMeshType m_meshType;
    std::shared_ptr<CC3DTextureRHI> m_material;
	BlendType m_blendType;
};


class CFaceEffectMakeUp : public CEffectPart
{
public:
	CFaceEffectMakeUp();
	virtual ~CFaceEffectMakeUp();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

	std::vector<MakeUpInfo>& getMakeUpInfo() {
		return m_vMakeUpBlendInfo;
	}

	//static CFaceEffectMakeUp* createEffect();
	bool DrawTriLineToFBO(GLfloat* points, int count, unsigned short* indexs);
private:

	void ReleaseMakeUpInfo();
	void MergeVertex(float *pVertex, float *pUV, int nVertex);


	FaceMeshManage *m_FaceMeshManage;

	std::vector<MakeUpInfo>m_vMakeUpBlendInfo;
	std::shared_ptr<class CCProgram> m_pShader;
	float *m_pMergeVertex;

	bool m_isFirstInit;
	bool m_isCrop;

};
