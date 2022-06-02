#include "BaseRenderParam.h"


BaseRenderParam::BaseRenderParam()
{
	m_pDoubleBuffer = NULL;
	m_nFaceCount = 0;
	m_pBodyMask = NULL;
	m_pHairMask = NULL;
}


BaseRenderParam::~BaseRenderParam()
{
	SAFE_DELETE(m_pBGR_Src);
}

void BaseRenderParam::SetFacePoint130(Vector2 * pPoint130,int nFaceIndex, int nWidth, int nHeight)
{
	if (nFaceIndex>= m_nFaceCount)
	{
		return;
	}
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	memcpy(m_pPoint130[nFaceIndex], pPoint130, sizeof(Vector2) * 130);
	m_bHasNormlize[nFaceIndex] = false;
	m_bHasTranslate106[nFaceIndex] = false;
}

void BaseRenderParam::SetHandInfo(ccHGHandRes * handRes)
{
	m_nHandCount = handRes->numHand;
	if (m_nHandCount > MAX_SUPPORT_HAND)
	{
		m_nHandCount = MAX_SUPPORT_HAND;
	}
	m_HeartGesture.clear();
	for (int n = 0; n < m_nHandCount; n++)
	{
		int gestureType = handRes->handInfo[n].gestureType;
		if (gestureType == 7)
		{
			HandInfo m_HandInfo;
			m_HandInfo.handType = handRes->handInfo[n].handType;
			m_HandInfo.gestureType = handRes->handInfo[n].gestureType;
			m_HandInfo.handRect.x = handRes->handInfo[n].handRect.left;
			m_HandInfo.handRect.y = handRes->handInfo[n].handRect.top;
			m_HandInfo.handRect.z = handRes->handInfo[n].handRect.right;
			m_HandInfo.handRect.w = handRes->handInfo[n].handRect.bottom;
			memcpy(m_HandInfo.handPoint, (Vector2*)handRes->handInfo[n].keyPointArr, sizeof(Vector2) * 21);

			m_HeartGesture.push_back(m_HandInfo);
		}
	}
}

std::vector<HandInfo>& BaseRenderParam::GetHeartGestureInfo()
{
	return m_HeartGesture;
}

void BaseRenderParam::SetFaceNum(int nFaceCount)
{
	m_nFaceCount = nFaceCount;
	if (m_nFaceCount>MAX_SUPPORT_PEOPLE)
	{
		m_nFaceCount = MAX_SUPPORT_PEOPLE;
	}
}

void BaseRenderParam::SetDoubleBuffer(DX11DoubleBuffer *pDoubleBuffer)
{
	m_pDoubleBuffer = pDoubleBuffer;
}

void BaseRenderParam::SetFacePosInfo(FacePosInfo * pFacePosInfo, int nFaceIndex)
{
	if (nFaceIndex >= m_nFaceCount)
	{
		return;
	}
	memcpy(m_FacePosInfo[nFaceIndex].arrWMatrix, pFacePosInfo->arrWMatrix, 16 * sizeof(float));
	m_FacePosInfo[nFaceIndex].pitch = pFacePosInfo->pitch;
	m_FacePosInfo[nFaceIndex].roll = pFacePosInfo->roll;
	m_FacePosInfo[nFaceIndex].yaw = pFacePosInfo->yaw;
	m_FacePosInfo[nFaceIndex].x = pFacePosInfo->x;
	m_FacePosInfo[nFaceIndex].y = pFacePosInfo->y;
	m_FacePosInfo[nFaceIndex].z = pFacePosInfo->z;
	m_FacePosInfo[nFaceIndex].faceW = pFacePosInfo->faceW;
	m_FacePosInfo[nFaceIndex].faceH = pFacePosInfo->faceH;

	m_FacePosInfo[nFaceIndex].pFaceRect = pFacePosInfo->pFaceRect;
	m_FacePosInfo[nFaceIndex].pFaceExp = pFacePosInfo->pFaceExp;
}

void BaseRenderParam::SetBodyMaskTexture(DX11Texture * pTex)
{
	m_pBodyMask = pTex;
}

void BaseRenderParam::SetHairMaskTexture(DX11Texture * pTex)
{
	m_pHairMask = pTex;
}

void BaseRenderParam::SetSrcTex(ID3D11ShaderResourceView * tex)
{
	m_SrcTex = tex;
}

Vector2 * BaseRenderParam::GetFacePoint(int nFaceIndex, FacePointType eFaceType, bool bNormalize)
{
	if (nFaceIndex >= m_nFaceCount)
	{
		return NULL;
	}
	if (eFaceType == FACE_POINT_106)
	{
		if (!m_bHasTranslate106[nFaceIndex])
		{
			FacePointIndexTransfer(m_pPoint130[nFaceIndex], m_pPoint106[nFaceIndex]);
		}
		if (bNormalize)
		{
			if (!m_bHasNormlize[nFaceIndex])
			{
				NoramlizePoint(m_pPoint106[nFaceIndex], m_pPoint106_Normal[nFaceIndex],106);
			}
			return m_pPoint106_Normal[nFaceIndex];
		}
		else
		{
			return m_pPoint106[nFaceIndex];
		}
	}
	if (eFaceType == FACE_POINT_130)
	{
		if (bNormalize)
		{
			if (!m_bHasNormlize[nFaceIndex])
			{
				NoramlizePoint(m_pPoint130[nFaceIndex], m_pPoint130_Normal[nFaceIndex], 130);
			}
			return m_pPoint130_Normal[nFaceIndex];
		}
		else
		{
			return m_pPoint130[nFaceIndex];
		}
	}
}

DX11DoubleBuffer * BaseRenderParam::GetDoubleBuffer()
{
	return m_pDoubleBuffer;
}

int BaseRenderParam::GetWidth()
{
	return m_pDoubleBuffer->GetWidth();
}

int BaseRenderParam::GetHeight()
{
	return m_pDoubleBuffer->GetHeight();
}

int BaseRenderParam::GetHandCount()
{
	return m_nHandCount;
}

int BaseRenderParam::GetFaceCount()
{
	return m_nFaceCount;
}

BYTE * BaseRenderParam::GetBGR_SRC()
{
	if (m_pBGR_Src==NULL)
	{
		m_pBGR_Src = new BYTE[m_nWidth*m_nHeight * 3];
	}
	int nSize = m_nWidth * m_nHeight;
	for (int i =0;i<nSize;i++)
	{
		m_pBGR_Src[i * 3] = m_pBGRA_Src[i * 4+0];
		m_pBGR_Src[i * 3+1] = m_pBGRA_Src[i * 4+1];
		m_pBGR_Src[i * 3+2] = m_pBGRA_Src[i * 4+2];
	}

	return m_pBGR_Src;
}

FacePosInfo * BaseRenderParam::GetFaceInfo(int nFaceIndex)
{
	return m_FacePosInfo + nFaceIndex;
}

DX11Texture * BaseRenderParam::GetBodyMaskTexture()
{
	return m_pBodyMask;
}

DX11Texture * BaseRenderParam::GetHairMaskTexture()
{
	return m_pHairMask;
}

ID3D11ShaderResourceView * BaseRenderParam::GetSrcTex()
{
	return m_SrcTex;
}

void BaseRenderParam::FacePointIndexTransfer(Vector2 * pSrcPoint130, Vector2 * pDstPoint106)
{
	memcpy(pDstPoint106, pSrcPoint130, sizeof(Vector2) * 106);
	return;
	//轮廓，眉毛
	memcpy(pDstPoint106, pSrcPoint130, sizeof(Vector2) * 51);
	//左眼睛
	memcpy(pDstPoint106 + 51, pSrcPoint130 + 60, sizeof(Vector2) * 8);
	pDstPoint106[59] = pSrcPoint130[96];
	pDstPoint106[60].x = (pSrcPoint130[60].x + pSrcPoint130[64].x)*0.5;
	pDstPoint106[60].y = (pSrcPoint130[62].y + pSrcPoint130[66].y)*0.5;

	//右眼睛
	memcpy(pDstPoint106 + 61, pSrcPoint130 + 68, sizeof(Vector2) * 8);
	pDstPoint106[69] = pSrcPoint130[97];
	pDstPoint106[70].x = (pSrcPoint130[68].x + pSrcPoint130[72].x)*0.5;
	pDstPoint106[70].y = (pSrcPoint130[70].y + pSrcPoint130[74].y)*0.5;

	//鼻子
	memcpy(pDstPoint106 + 71, pSrcPoint130 + 51, sizeof(Vector2) * 4);
	memcpy(pDstPoint106 + 75, pSrcPoint130 + 110, sizeof(Vector2) * 3);
	memcpy(pDstPoint106 + 78, pSrcPoint130 + 55, sizeof(Vector2) * 5);
	pDstPoint106[83] = pSrcPoint130[115];
	pDstPoint106[84] = pSrcPoint130[114];
	pDstPoint106[85] = pSrcPoint130[113];

	memcpy(pDstPoint106 + 86, pSrcPoint130 + 76, sizeof(Vector2) * 20);
}

void BaseRenderParam::NoramlizePoint(Vector2 * pPoint, Vector2 *pDstPoint,int nPoint)
{
	Vector2 nImgSize(1.0f / m_nWidth, 1.0 / m_nHeight);
	for (int i=0;i<nPoint;i++)
	{
		pDstPoint[i] = pPoint[i] * nImgSize;
	}
}
