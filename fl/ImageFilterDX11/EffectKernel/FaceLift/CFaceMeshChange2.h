#pragma once
#include "EffectKernel/CEffectPart.h"
#include <vector>
#include "BaseDefine/Vectors.h"
#include "Algorithm/InterFacePointDeform.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include "Toolbox/Drawable.hpp"


struct FaceChangeFramne2
{
	int key;
	float alpha;
};
class CFaceMeshChange2 :public CEffectPart
{
public:
	CFaceMeshChange2();
	virtual ~CFaceMeshChange2();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	virtual void Release();


	CEffectPart *partMLS = NULL;
private:
	void Resize(int nWidth, int nHeight);
	void RenderMaterial(int nIndex,Vector2 *pFacePoint);
	void CalFaceChangePoint(BaseRenderParam & RenderParam);

private:
	int m_nWidth;
	int m_nHeight;

	Drawable *m_2DAnimation[2];
	int m_nAllFrame;
	int m_nKeyFrame;
	int m_nFPS;
	vector<FaceChangeFramne2>m_keyValue;

	float m_changelpha = 1.5f;

	DX11Shader *m_pShaderMat;

	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;
	ID3D11Buffer *m_rectVerticeBuffer[MAX_SUPPORT_PEOPLE];
};

