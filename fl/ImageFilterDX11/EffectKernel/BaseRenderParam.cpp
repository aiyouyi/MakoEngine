#include "BaseRenderParam.h"
#include "..\3rdparty\SplitScreenDetectSDK\include\SplitScreenDetectDef.h"

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

void BaseRenderParam::SetHandInfo(ccHandRes* handRes)
{
	int m_nCount = handRes->numHand;
	if (m_nCount > MAX_SUPPORT_HAND)
	{
		m_nCount = MAX_SUPPORT_HAND;
	}
	m_nHandCount = 0;
	for (int n = 0; n < m_nCount; n++)
	{
		int gestureType = handRes->arrHand[n].gestureType;
		if (gestureType > -1 )
		{
			HandInfo m_HandInfo;
			m_HandInfo.handType = handRes->arrHand[n].handType;
			m_HandInfo.gestureType = handRes->arrHand[n].gestureType;
			m_HandInfo.handRect.x = handRes->arrHand[n].handRect.left;
			m_HandInfo.handRect.y = handRes->arrHand[n].handRect.top;
			m_HandInfo.handRect.z = handRes->arrHand[n].handRect.right;
			m_HandInfo.handRect.w = handRes->arrHand[n].handRect.bottom;
			memcpy(m_HandInfo.handPoint, (Vector4*)handRes->arrHand[n].keyPointArr, sizeof(Vector4) * 21);

			m_HeartGesture[m_nHandCount] = m_HandInfo;
			m_nHandCount += 1;
		}
	}
}

HandInfo* BaseRenderParam::GetGestureInfo(int nIndex)
{
	return m_HeartGesture + nIndex;
}

int BaseRenderParam::GetHandCount()
{
	return m_nHandCount;
}

void BaseRenderParam::SetFaceNum(int nFaceCount)
{
	m_nFaceCount = nFaceCount;
	if (m_nFaceCount>MAX_SUPPORT_PEOPLE)
	{
		m_nFaceCount = MAX_SUPPORT_PEOPLE;
	}
}

void BaseRenderParam::SetSize(int width, int height)
{
	m_nWidth = width;
	m_nHeight = height;
}

void BaseRenderParam::SetBodyPoint(ccBodyRes * bodyRes)
{
	m_nBodyCount = bodyRes->numBody;
	if (m_nBodyCount > MAX_SUPPORT_PEOPLE)
	{
		m_nBodyCount = MAX_SUPPORT_PEOPLE;
	}

	for (int n = 0; n < m_nBodyCount; n++)
	{
		Vector2 bodyPont[16];
		for (int j = 0; j < 16; j++)
		{
			bodyPont[j].x = bodyRes->arrBody[n].keyPointArr[j].x;
			bodyPont[j].y = bodyRes->arrBody[n].keyPointArr[j].y;
		}

		memcpy(m_pBodyPoint16[n], bodyPont, sizeof(Vector2) * 16);

		m_BodyPosInfo[n].left = bodyRes->arrBody[n].bodyRect.left;
		m_BodyPosInfo[n].right = bodyRes->arrBody[n].bodyRect.right;
		m_BodyPosInfo[n].bottom = bodyRes->arrBody[n].bodyRect.bottom;
		m_BodyPosInfo[n].top = bodyRes->arrBody[n].bodyRect.top;
		m_BodyPosInfo[n].score = bodyRes->arrBody[n].bodyRect.score;
	}

}

void BaseRenderParam::SetDoubleBuffer(std::shared_ptr<DoubleBufferRHI> pDoubleBuffer)
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
	m_FacePosInfo[nFaceIndex].pCameraRect = pFacePosInfo->pCameraRect;
}

void BaseRenderParam::SetBodyMaskTexture(std::shared_ptr<CC3DTextureRHI> pTex)
{
	m_pBodyMask = pTex;
}

void BaseRenderParam::SetHairMaskTexture(std::shared_ptr<CC3DTextureRHI> pTex)
{
	m_pHairMask = pTex;
}

void BaseRenderParam::SetSrcTex(std::shared_ptr<CC3DTextureRHI> tex)
{
	m_SrcTex = tex;
}

void BaseRenderParam::SetCardMaskID(std::map<AnchorType, long long>& cardId)
{
	m_CardMask = cardId;
}

void BaseRenderParam::SetSplitScreenNum(int SplitType)
{
	switch (SplitType)
	{
	case SSD_SCREEN_TYPE_FULL:
		m_SplitScreenNum = 1.0;
		b_SplitMirror = false;
		break;
	case SSD_SCREEN_TYPE_TWO_REPLICATE:
		m_SplitScreenNum = 2.0;
		b_SplitMirror = false;
		break;
	case SSD_SCREEN_TYPE_TWO_MIRROR:
		b_SplitMirror = true;
		m_SplitScreenNum = 2.0;
		break;
	case SSD_SCREEN_TYPE_THREE_REPLICATE:
		m_SplitScreenNum = 3.0;
		b_SplitMirror = false;
		break;
	default:
		break;
	}
}

int BaseRenderParam::GetSplitScreenNum(bool& bMirror)
{
	bMirror = b_SplitMirror;
	return m_SplitScreenNum;
}

std::map<AnchorType, long long>& BaseRenderParam::GetCardMaskID()
{
	return m_CardMask;
}

void BaseRenderParam::SetExpressionCoffes(std::vector<float>& coffes)
{
	m_Coffes = coffes;
}

std::vector<float>& BaseRenderParam::GetExpressionCoffes()
{
	return m_Coffes;
}

Vector2 * BaseRenderParam::GetBodyPoint(int nBodyIndex)
{
	if (nBodyIndex > m_nBodyCount)
	{
		return NULL;
	}

	return m_pBodyPoint16[nBodyIndex];
}

BodyPosInfo * BaseRenderParam::GetBodyPosInfo(int nBodyIndex)
{
	return m_BodyPosInfo + nBodyIndex;
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

std::shared_ptr<DoubleBufferRHI> BaseRenderParam::GetDoubleBuffer()
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

int BaseRenderParam::GetBodyCount()
{
	return m_nBodyCount;
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

std::shared_ptr<CC3DTextureRHI> BaseRenderParam::GetBodyMaskTexture()
{
	return m_pBodyMask;
}

std::shared_ptr<CC3DTextureRHI> BaseRenderParam::GetHairMaskTexture()
{
	return m_pHairMask;
}

std::shared_ptr<CC3DTextureRHI> BaseRenderParam::GetSrcTex()
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
