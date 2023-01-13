#pragma once
#include <vector>
#include <memory>
#include "EffectKernel/CEffectPart.h"
#include "BaseDefine/Vectors.h"
#include "Algorithm/InterFacePointDeform.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"

class CCProgram;
class Drawable;

class CFaceMeshChangeGL :public CEffectPart
{
	struct FaceChangeFramne
	{
		int key;
		float alpha;
	};
public:
	CFaceMeshChangeGL();
	virtual ~CFaceMeshChangeGL();
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
	bool DrawTriLineToFBO(GLfloat* points, int count, unsigned short* indexs);


private:
	int m_nWidth;
	int m_nHeight;

    Drawable *m_2DAnimation[2];
	int m_nAllFrame;
	int m_nKeyFrame;
	int m_nFPS;
	std::vector<FaceChangeFramne>m_keyValue;
	mt3dface::MTFace2DInterFace *m_2DInterFace;

	int m_nVertsSize;
	long m_runTime = 0;
	std::shared_ptr<CCProgram> m_pShaderMat;
	std::shared_ptr<CCProgram> m_pShader;
};

