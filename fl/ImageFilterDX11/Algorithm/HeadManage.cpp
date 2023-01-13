#include "HeadManage.h"
#include "Algorithm/MLS.h"
#include "Algorithm/MathUtils.h"
#include "Algorithm/MaskFillUtil.h"
#include "Algorithm/StackBlur.h"
#include <iostream>
#include <algorithm>

HeadManage::HeadManage()
{
	m_pHeadMask = NULL;
	m_nMaskWidth = 0;
	m_nMaskHeight = 0;
	Init();
	
}

HeadManage::~HeadManage()
{
	SAFE_DELETE_ARRAY(m_pHeadMask);
}

void HeadManage::Init()
{
	m_2DInterFace = std::make_shared<mt3dface::MTFace2DInterFace>();
}

void HeadManage::GenHeadMask(Vector2 * pFacePoint, int nWidth, int nHeight)
{

	auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, nWidth, nHeight, 0, mt3dface::MT_FACE_2D_BACKGROUND);

	m_nMaskWidth = nWidth * 0.5;
	m_nMaskHeight = nHeight * 0.5;

	SAFE_DELETE_ARRAY(m_pHeadMask);
	int nSize = m_nMaskWidth * m_nMaskHeight;
	m_pHeadMask = new BYTE[4*nSize];

	BYTE *pHeadMask = new BYTE[nSize];

	Vector2 meshPoint[259];
	for (int j = 0; j < FaceMesh->nVertex; j++)
	{
		meshPoint[j].x = (0.5 * FaceMesh->pVertexs[3 * j]+0.5) * nWidth * 0.5;
		meshPoint[j].y = (0.5 * FaceMesh->pVertexs[3 * j + 1]+0.5) * nHeight * 0.5;
	}

	Vector2 HeadPoint[24];
	HeadPoint[0] = meshPoint[235];

	HeadPoint[1] = meshPoint[236];
	HeadPoint[2] = meshPoint[237];
	HeadPoint[3] = meshPoint[238];

	for (int i = 242; i < 259; i++)
	{
		HeadPoint[i - 238] = meshPoint[i];
	}

	HeadPoint[21] = meshPoint[241];
	HeadPoint[22] = meshPoint[240];
	HeadPoint[23] = meshPoint[239];

	
	Vector2 pOut[70];
	int outCnt = CMathUtils::SmoothLines2(pOut, HeadPoint, 24, 0.5, 3);

	CMaskFillUtil::FillMask(pHeadMask, m_nMaskWidth, m_nMaskHeight, HeadPoint, 24, MT_VerticalBinaryPolygon, 255, 0);
	CStackBlur tempStackBlur;
	int radius = 2;
	tempStackBlur.Run(pHeadMask, m_nMaskWidth, m_nMaskHeight, m_nMaskWidth, radius);

	for (int i=0;i<nSize;i++)
	{
		m_pHeadMask[i * 4] = pHeadMask[i];
		m_pHeadMask[i * 4 + 1] =  0;
		m_pHeadMask[i * 4 + 2] = 0;;
		m_pHeadMask[i * 4 + 3] = 255;
	}

	SAFE_DELETE_ARRAY(pHeadMask);
}


