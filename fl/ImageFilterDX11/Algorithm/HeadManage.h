#pragma once
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"
#include <memory>


class HeadManage
{
public:
	HeadManage();
	~HeadManage();
	void Init();
	void GenHeadMask(Vector2 *pFacePoint, int nWidth, int nHeight);

	BYTE* m_pHeadMask;
	int m_nMaskWidth;
	int m_nMaskHeight;
private:
	std::shared_ptr<mt3dface::MTFace2DInterFace> m_2DInterFace;

};

