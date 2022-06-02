#pragma once
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include <vector>

bool isMouthOpened(Vector2 *pFacePoints, float param);
bool isEyeOpened(Vector2 *pEyePoints, float param);
class MouthManage
{
public:
	MouthManage();
	~MouthManage();

	void GenMouthMask(Vector2 *pFacePoint, int nWidth, int nHeight);

	
	BYTE *m_pMouthMask;
	int m_nMaskWidth;
	int m_nMaskHeight;
	int m_Left;
	int m_Right;
	int m_Top;
	int m_Bottom;
private:

	void GetMouthCropRectangle(Vector2 *pFacePoint, int nWidth, int nHeight);
	void FillMouthMask(Vector2 *pMouthPoint, BYTE *pMouthMask);
	void GenSmallMask(BYTE *pMouthMask, BYTE *pMouthSmall);
	void GenHightLightMask(Vector2 *pMouthPoint, BYTE *pHightLightMask);


	float m_fRatio;
};

