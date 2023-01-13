#pragma once
#include "EffectKernel/CEffectPart.h"
#include <vector>
#include "BaseDefine/Vectors.h"
#include "Algorithm/InterFacePointDeform.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include "Toolbox/Drawable.hpp"
#include "EffectKernel/DXBasicSample.h"

struct FaceChangeFramne
{
	int key;
	float alpha;
};
class CFaceMeshChange :public CEffectPart, protected DXBaicSample
{
public:
	CFaceMeshChange();
	virtual ~CFaceMeshChange();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	virtual void Release();


	CEffectPart *partMLS = NULL;
private:
	void Resize(int nWidth, int nHeight);
	void RenderMaterial(int nIndex,Vector2 *pFacePoint);
	void CalFaceChangePoint(Vector2 *pSrcPoint118, Vector2*pDstPoint118);
	void MergeVertex(float * pVertex, float *pMaskUV, int nVertex);

private:
	int m_nWidth;
	int m_nHeight;

	Drawable *m_2DAnimation[2];
	int m_nAllFrame;
	int m_nKeyFrame;
	int m_nFPS;
	std::vector<FaceChangeFramne>m_keyValue;



	DX11Shader *m_pShaderMat;

	ID3D11Buffer *m_IndexBuffer;
	ID3D11Buffer *m_VerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer* m_pConstantBuffer;

	ID3D11Buffer *m_rectVerticeBuffer[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_rectVerticeBuffer2[MAX_SUPPORT_PEOPLE];
	ID3D11Buffer *m_rectIndexBuffer;

	mt3dface::MTFace2DInterFace *m_2DInterFace;



	float *m_pMergeVertex;
	int m_nVertsSize;
};

