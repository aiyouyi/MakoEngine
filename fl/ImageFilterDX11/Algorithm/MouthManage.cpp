#include "MouthManage.h"
#include "Algorithm/MLS.h"
#include "Algorithm/MathUtils.h"
#include "Algorithm/MaskFillUtil.h"
#include "Algorithm/StackBlur.h"
#include <iostream>
#include <algorithm>

MouthManage::MouthManage()
{
	m_pMouthMask = NULL;
	m_nMaskWidth = 0;
	m_nMaskHeight = 0;
	m_fRatio = 1.0;
}

MouthManage::~MouthManage()
{
	SAFE_DELETE_ARRAY(m_pMouthMask);
}

bool isMouthOpened(Vector2 *pFacePoints, float param = 0.25)
{
	//计算上下唇的距离相加取中值
	bool Ist = 0;
	float upans = pFacePoints[89].distance(pFacePoints[100]);
	float downans = pFacePoints[95].distance(pFacePoints[104]);
	float ans = (upans + downans) / 2.0f;
	//嘴唇中间距离
	float midans = pFacePoints[100].distance(pFacePoints[104]);
	//计算嘴唇长度
	float lengthans = pFacePoints[98].distance(pFacePoints[102]) / 4.5f;
	if (lengthans < ans) {
		ans = lengthans;
	}
	//比例
	float ratio = midans / ans;
	if (ratio > param) {
		Ist = 1;
	}
	return Ist;
}

bool isEyeOpened(Vector2 *pEyePoints, float param = 0.25)
{
	//计算上下唇的距离相加取中值
	bool Ist = 0;
	//通过眼角角度计算睁眼闭眼
	Vector2 up = (pEyePoints[3] - pEyePoints[4]).normalize();
	Vector2 down = (pEyePoints[5] - pEyePoints[4]).normalize();

	//眼角角度
	float angle = 1.0 - up.dot(down);

	if (angle > param) {
		Ist = 1;
	}
	return Ist;

}

void MouthManage::GenMouthMask(Vector2 * pFacePoint, int nWidth, int nHeight)
{
	GetMouthCropRectangle(pFacePoint, nWidth, nHeight);
	SAFE_DELETE_ARRAY(m_pMouthMask);
	int nSize = m_nMaskWidth * m_nMaskHeight;
	m_pMouthMask = new BYTE[4*nSize];

	BYTE *pMouthMask = new BYTE[nSize];
	BYTE *pMouthSmall = new BYTE[nSize];

	Vector2 MouthPoint[50];
	//外嘴唇
	{

		MouthPoint[0] = pFacePoint[86];
		MouthPoint[1] = pFacePoint[110];
		MouthPoint[2] = pFacePoint[87];
		MouthPoint[3] = pFacePoint[111];
		MouthPoint[4] = pFacePoint[88];
		MouthPoint[5] = pFacePoint[89];
		MouthPoint[6] = pFacePoint[90];
		MouthPoint[7] = pFacePoint[112];
		MouthPoint[8] = pFacePoint[91];
		MouthPoint[9] = pFacePoint[113];
		MouthPoint[10] = pFacePoint[92];
		MouthPoint[11] = pFacePoint[93];
		MouthPoint[12] = pFacePoint[94];
		MouthPoint[13] = pFacePoint[95];
		MouthPoint[14] = pFacePoint[96];
		MouthPoint[15] = pFacePoint[97];
		MouthPoint[16] = pFacePoint[86];

	}

	Vector2 *pM = MouthPoint + 17;
	//内嘴唇
	{
		pM[0] = pFacePoint[98];
		pM[1] = pFacePoint[98] * 0.5 + pFacePoint[105] * 0.5;
		pM[2] = pFacePoint[105];
		pM[3] = pFacePoint[108];
		pM[4] = pFacePoint[104];
		pM[5] = pFacePoint[109];
		pM[6] = pFacePoint[103];
		pM[7] = pFacePoint[102]*0.5 + pFacePoint[103]*0.5;
		pM[8] = pFacePoint[102];
		pM[9] = (pFacePoint[101] + pFacePoint[102])*0.5;
		pM[10] = pFacePoint[101];
		pM[11] = pFacePoint[107];
		pM[12] = pFacePoint[100];
		pM[13] = pFacePoint[106];
		pM[14] = pFacePoint[99];
		pM[15] = pFacePoint[99] * 0.5 + pFacePoint[98] * 0.5;
		pM[16] = pFacePoint[98];


	}
	for (int i=0;i<34;i++)
	{
		MouthPoint[i] -= Vector2(m_Left, m_Top);
		MouthPoint[i] *= m_fRatio;
	}
	Vector2 pOut[300];
	int outCnt = CMathUtils::SmoothLines2(pOut, MouthPoint, 17, 0.5, 6);

	CMaskFillUtil::FillMask(pMouthMask, m_nMaskWidth, m_nMaskHeight, pOut, outCnt, MT_VerticalBinaryPolygon, 255, 0);
	CStackBlur tempStackBlur;
	int radius = 2;
	tempStackBlur.Run(pMouthMask, m_nMaskWidth, m_nMaskHeight, m_nMaskWidth, radius);

	bool hasTooth = isMouthOpened(pFacePoint,0.4);
	if (hasTooth)
	{
		int outCnt2 = CMathUtils::SmoothLines2(pOut, pM, 17, 0.5, 6);
		CMaskFillUtil::FillMask(pMouthSmall, m_nMaskWidth, m_nMaskHeight, pOut, outCnt, MT_VerticalBinaryPolygon, 255, 0);
		ExpansionErosionBinary(pMouthSmall, m_nMaskWidth, m_nMaskHeight, radius, false);
		tempStackBlur.Run(pMouthSmall, m_nMaskWidth, m_nMaskHeight, m_nMaskWidth, radius);

		for (int i = 0; i < nSize; i++)
		{
//			if (pMouthSmall[i] == 255)
//			{
				pMouthMask[i] -= pMouthSmall[i];
//			}
		}
	}
	else
	{
		memset(pMouthSmall, 0, m_nMaskWidth*m_nMaskHeight);
	}


	for (int i=0;i<nSize;i++)
	{
		m_pMouthMask[i * 4] = pMouthMask[i];
		m_pMouthMask[i * 4 + 1] =  pMouthSmall[i];
		m_pMouthMask[i * 4 + 2] = 0;// pHightLight[i];
		m_pMouthMask[i * 4 + 3] = 255;
	}

	SAFE_DELETE_ARRAY(pMouthMask);
	SAFE_DELETE_ARRAY(pMouthSmall);
}

void MouthManage::GetMouthCropRectangle(Vector2 * pFacePoint, int nWidth, int nHeight)
{
	int crop_left = 0, crop_top = 0, crop_right = 0, crop_bottom = 0, crop_width = 0, crop_height = 0;

	CMathUtils::GetOutSideRect(pFacePoint + 86, 20, crop_left, crop_top, crop_right, crop_bottom);

	//预留边界几个像素用于保护
	int padding = 10;
	crop_left -= padding;
	crop_top -= padding;
	crop_right += padding;
	crop_bottom += padding;
	//人脸点会超出原图尺寸，需要边界防护
	crop_left = (std::max)(0, (std::min)(crop_left, nWidth));
	crop_right = (std::max)(0, (std::min)(crop_right, nWidth));
	crop_top = (std::max)(0, (std::min)(crop_top, nHeight));
	crop_bottom = (std::max)(0, (std::min)(crop_bottom, nHeight));

	//计算宽高
	crop_width = crop_right - crop_left;
	crop_height = crop_bottom - crop_top;

	m_Left = crop_left;
	m_Right = crop_right;
	m_Top = crop_top;
	m_Bottom = crop_bottom;

	//最大mask宽有限制，超过要缩放
	int maxWidth = 256;
	if (crop_width > maxWidth) {
		m_fRatio = (float)maxWidth / (float)crop_width;
		crop_width = maxWidth;
		crop_height *= m_fRatio;
	}
	else {
		m_fRatio = 1.0f;
	}

	m_nMaskWidth = crop_width;
	m_nMaskHeight = crop_height;


}

void MouthManage::FillMouthMask(Vector2 * pMouthPoint, BYTE * pMouthMask)
{
}

void MouthManage::GenSmallMask(BYTE * pMouthMask, BYTE * pMouthSmall)
{
	int radius = 4;
	CStackBlur tempStackBlur;
	tempStackBlur.Run(pMouthMask, m_nMaskWidth, m_nMaskHeight, m_nMaskWidth, radius);

}

void MouthManage::GenHightLightMask(Vector2 * pMouthPoint, BYTE * pHightLightMask)
{
	Vector2 center = (pMouthPoint[13] + pMouthPoint[21])*0.5;

	//float radiusY = 0.3*pMouthPoint[13].distance(pMouthPoint[21]);

	float radiusX = 0.66*pMouthPoint[22].distance(pMouthPoint[20]);

	Vector2 c1 = center + 0.25*(pMouthPoint[22] - pMouthPoint[20]);
	Vector2 c2 = center - 0.25*(pMouthPoint[22] - pMouthPoint[20]);

	memset(pHightLightMask, 0, m_nMaskWidth*m_nMaskHeight);
	for (int i=0;i<m_nMaskHeight;i++)
	{
		for (int j=0;j<m_nMaskWidth;j++)
		{
			Vector2 tmp = Vector2(j, i);
			float dist = tmp.distance(c1) + tmp.distance(c2);
			if (dist<radiusX)
			{
				BYTE value = 255 *(radiusX - dist) / radiusX;
// 				if (dist >0.85*radiusX)
// 				{
					
// 					if (rand() % 3 == 0)
// 					{
					value = value * ((rand() % 4)+16) / 20;
//					}
	//			}

				value = value * 2.0;
				value = value < 255 ? value : 255;
				pHightLightMask[i*m_nMaskWidth+j] = value;
			}
		}
	}

}

