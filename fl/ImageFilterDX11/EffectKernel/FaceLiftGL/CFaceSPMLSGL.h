#pragma once
#include "EffectKernel/CEffectPart.h"
#include "BaseDefine/Vectors.h"
#include "Algorithm/InterFacePointDeform.h"
#include <vector>
#include <memory>

class CCProgram;

class CFaceSPMLSGL :public CEffectPart
{
public:
	CFaceSPMLSGL();
	virtual ~CFaceSPMLSGL();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	virtual void Release();


private:
	void Resize(int nWidth, int nHeight);

	void RunFace106To118(Vector2 *pFacePoint, Vector2 *pFacePoint118);

	float CalArea(Vector2 point1, Vector2 point2, Vector2 point3);

	void GetFaceScaleEachPoint(Vector2 * pPoint118, Vector2 * pStandard442Src, unsigned short *pFaceMesh, Vector2 * pScaleValue442);
	
	void GetOffsetTransform(Vector2 *pPoint118, int nImgWidth, int nImgHeight, float *pOutMat);

	float GetSideFaceParam(float * pPoint118, float SideParam);

private:
	int m_SmallTextureWidth;
	int m_SmallTextureHeight;
    std::shared_ptr<class MaterialTexRHI> m_pOffestTexture;
	byte_t* m_OffestData = nullptr;

	int m_nWidth;
	int m_nHeight;
	InterFacePointDeform *m_InterFM;
	std::shared_ptr<CCProgram> m_pShader;
};

