#pragma once
#include "EffectKernel/CEffectPart.h"
#include <vector>
#include "BaseDefine/Vectors.h"
#include "Algorithm/InterFacePointDeform.h"
#include <memory>

#define MaxTypeCount 30
class CFaceBeautyGL :public CEffectPart
{
public:
	CFaceBeautyGL();
	virtual ~CFaceBeautyGL();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam& RenderParam);
	virtual void Release();

	void SetAlpha(float alpha, CCEffectType type);
	InterFacePointDeform *m_InterFM;
	
private:
	void Resize(int nWidth, int nHeight);
	void RunFace106To118(Vector2 *pFacePoint, Vector2 *pFacePoint118);

	float CalArea(Vector2 point1, Vector2 point2, Vector2 point3);

	void GetFaceScaleEachPoint(Vector2 * pPoint118, Vector2 * pStandard442Src, unsigned short *pFaceMesh, Vector2 * pScaleValue442);
	//计算获取人脸偏移
	void GetOffsetTransform(Vector2 *pPoint118, int nImgWidth, int nImgHeight, float *pOutMat);

	float GetSideFaceParam(float * pPoint118, float SideParam);

	void initOffestTexture(CCEffectType type, BYTE *pOffestTexture);

	void GenerateTexture();
	
private:
	int m_SmallTextureWidth;
	int m_SmallTextureHeight;
	float *m_pOffestTexture[MaxTypeCount];
	float m_alpha2[MaxTypeCount];

	GLuint m_Texture;
	int m_nWidth;
	int m_nHeight;
	int m_VTPointCount;
	int m_VTTriangleSize = 0;
	
	bool isFirst = true;

	std::shared_ptr<class CCProgram> m_pShader;
};

