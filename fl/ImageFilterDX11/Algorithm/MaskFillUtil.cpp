#include "MaskFillUtil.h"
bool CMaskFillUtil::FillMask(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTPolygonType polygonType, MTFillColor inside, MTFillColor outside) {
	switch (polygonType) {
	case MT_ConcavePolygon:
		return DirectFillPolygon(pMask, iWidth, iHeight, pPolygon, iCount, inside, outside);
	case MT_HorizontalBinaryPolygon:
		return HorizontalFillPolygon(pMask, iWidth, iHeight, pPolygon, iCount, inside, outside);
	case MT_VerticalBinaryPolygon:
		return VerticalFillPolygon(pMask, iWidth, iHeight, pPolygon, iCount, inside, outside);
	case MT_ConvexPolygon:
		return HorizontalFillPolygon(pMask, iWidth, iHeight, pPolygon, iCount, inside, outside);
	default:
		return false;
	}
}

bool CMaskFillUtil::DirectFillPolygon(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor inside, MTFillColor outside) {
	if (inside == MT_INVALID_COLOR && outside != MT_INVALID_COLOR) {
		return DirectFillPolygonOutside(pMask, iWidth, iHeight, pPolygon, iCount, outside);
	}
	if (pMask == NULL || iCount < 3 || (inside == MT_INVALID_COLOR && outside == MT_INVALID_COLOR)) {
		return false;
	}
	int iPointCount = iCount;
	Vector2* pCirclePoints = GetCirclePoints(pPolygon, iPointCount);
	CMaskFillUtil::MTRect rect = OutsideRect(pCirclePoints, iPointCount, iWidth, iHeight);
	int iStride = iWidth, iOffsetX = rect.iLeftX;

	byte insideColor = inside;
	if (outside != MT_INVALID_COLOR) {
		byte outsideColor = (byte)outside;
		memset(pMask, outsideColor, sizeof(byte)*iWidth*iHeight);
	}
	for (int y = rect.iLeftY; y <= rect.iRightY; y++) {
		Vector2 p(0, y);
		byte *pPixels = pMask + (y * iStride + iOffsetX);
		for (int x = rect.iLeftX; x < rect.iRightX; x++) {
			p.x = (float)(x);
			if (InsidePolygon(pCirclePoints, iPointCount, p)) {
				*(pPixels) = insideColor;
			}
			pPixels++;
		}
	}
	SAFE_DELETE_ARRAY(pCirclePoints);
	return true;
}

bool CMaskFillUtil::DirectFillPolygonOutside(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor fillColor) {
	if (pMask == NULL || iCount < 3 || fillColor == MT_INVALID_COLOR) {
		return false;
	}
	int iPointCount = iCount;
	byte outsideColor = (byte)fillColor;

	Vector2* pCirclePoints = GetCirclePoints(pPolygon, iPointCount);
	CMaskFillUtil::MTRect rect = OutsideRect(pCirclePoints, iPointCount, iWidth, iHeight);
	int iStride = iWidth, iOffsetX = rect.iLeftX;
	int iByteCount = iStride * iHeight;

	byte* pCopyMask = new byte[iByteCount];
	memcpy(pCopyMask, pMask, iByteCount);
	memset(pMask, outsideColor, iByteCount);

	for (int y = rect.iLeftY; y <= rect.iRightY; y++) {
		Vector2 p(0, (float)(y));
		byte *pPixels = pMask + (y * iStride + iOffsetX);
		byte *pCopyPixels = pCopyMask + (y * iStride + iOffsetX);
		for (int x = rect.iLeftX; x < rect.iRightX; x++) {
			p.x = (float)(x);
			if (InsidePolygon(pCirclePoints, iPointCount, p)) {
				*(pPixels) = *(pCopyPixels);
			}
			pPixels++;
			pCopyPixels++;
		}
	}
	SAFE_DELETE_ARRAY(pCirclePoints);
	return true;
}

bool CMaskFillUtil::VerticalFillPolygon(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor inside, MTFillColor outside) {
	if (inside == MT_INVALID_COLOR && outside != MT_INVALID_COLOR) {
		return VerticalFillPolygonOutside(pMask, iWidth, iHeight, pPolygon, iCount, outside);
	}
	if (pMask == NULL || iCount < 3 || (inside == MT_INVALID_COLOR && outside == MT_INVALID_COLOR)) {
		return false;
	}
	int iPointCount = iCount;
	Vector2* pCirclePoints = GetCirclePoints(pPolygon, iPointCount);
	CMaskFillUtil::MTRect rect = OutsideRect(pCirclePoints, iPointCount, iWidth, iHeight);
	int iStride = iWidth, iByteCount = iStride * iHeight;

	byte insideColor = inside;
	if (outside != MT_INVALID_COLOR) {
		byte outsideColor = (byte)outside;
		memset(pMask, outsideColor, sizeof(byte)*iByteCount);
	}

	byte* pPixels = pMask;
	MTRange vRange(rect.iLeftY, rect.iRightY);
	for (int x = rect.iLeftX; x < rect.iRightX; x++) {
		MTRange range = VerticalDichotomize(pCirclePoints, iCount, vRange, x);
		if (range.m_iStart <= range.m_iEnd) {
			pPixels = pMask + iStride * range.m_iStart + x;
			for (int y = range.m_iStart; y <= range.m_iEnd; y++) {
				*pPixels = insideColor;
				pPixels += iStride;
			}
		}
	}
	SAFE_DELETE_ARRAY(pCirclePoints);
	return true;
}

bool CMaskFillUtil::VerticalFillPolygonOutside(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor fillColor) {
	if (pMask == NULL || iCount < 3 || fillColor == MT_INVALID_COLOR) {
		return false;
	}
	int iPointCount = iCount;
	byte outsideColor = fillColor;

	Vector2* pCirclePoints = GetCirclePoints(pPolygon, iPointCount);
	CMaskFillUtil::MTRect rect = OutsideRect(pCirclePoints, iPointCount, iWidth, iHeight);
	int iStride = iWidth, iByteCount = iStride * iHeight;

	byte* pCopyMask = new byte[iByteCount];
	memcpy(pCopyMask, pMask, iByteCount);
	memset(pMask, outsideColor, iByteCount);

	byte* pPixels = pMask;
	MTRange vRange(rect.iLeftY, rect.iRightY);
	for (int x = rect.iLeftX; x < rect.iRightX; x++) {
		MTRange range = VerticalDichotomize(pCirclePoints, iCount, vRange, x);
		if (range.m_iStart <= range.m_iEnd) {
			pPixels = pMask + iStride * range.m_iStart + x;
			byte* pCopyPixels = pCopyMask + iStride * range.m_iStart + x;
			for (int y = range.m_iStart; y <= range.m_iEnd; y++) {
				*pPixels = *pCopyPixels;
				pPixels += iStride;
				pCopyPixels += iStride;
			}
		}
	}
	SAFE_DELETE_ARRAY(pCopyMask);
	SAFE_DELETE_ARRAY(pCirclePoints);
	return true;
}

bool CMaskFillUtil::HorizontalFillPolygon(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor inside, MTFillColor outside) {
	if (inside == MT_INVALID_COLOR && outside != MT_INVALID_COLOR) {
		return HorizontalFillPolygonOutside(pMask, iWidth, iHeight, pPolygon, iCount, outside);
	}
	if (pMask == NULL || iCount < 3 || (inside == MT_INVALID_COLOR && outside == MT_INVALID_COLOR)) {
		return false;
	}
	int iPointCount = iCount;
	byte insideColor = inside;

	Vector2* pCirclePoints = GetCirclePoints(pPolygon, iPointCount);
	CMaskFillUtil::MTRect rect = OutsideRect(pCirclePoints, iPointCount, iWidth, iHeight);
	int iStride = iWidth, iByteCount = iStride * iHeight;

	if (outside != MT_INVALID_COLOR) {
		byte outsideColor = (byte)outside;
		memset(pMask, outsideColor, sizeof(byte)*iByteCount);
	}

	byte* pPixels = pMask;
	MTRange hRange(rect.iLeftX, rect.iRightX);
	for (int y = rect.iLeftY; y <= rect.iRightY; y++) {
		MTRange range = HorizontalDichotomize(pCirclePoints, iCount, hRange, y);
		if (range.m_iStart <= range.m_iEnd) {
			pPixels = pMask + y * iStride + range.m_iStart;
			for (int x = range.m_iStart; x <= range.m_iEnd; x++) {
				*pPixels++ = insideColor;
			}
		}
	}
	SAFE_DELETE_ARRAY(pCirclePoints);
	return true;
}

bool CMaskFillUtil::HorizontalFillPolygonOutside(byte* pMask, int iWidth, int iHeight, Vector2* pPolygon, int iCount, MTFillColor fillColor) {
	if (pMask == NULL || iCount < 3 || fillColor == MT_INVALID_COLOR) {
		return false;
	}
	int iPointCount = iCount;
	byte outsideColor = fillColor;

	Vector2* pCirclePoints = GetCirclePoints(pPolygon, iPointCount);
	CMaskFillUtil::MTRect rect = OutsideRect(pCirclePoints, iPointCount, iWidth, iHeight);
	int iStride = iWidth, iByteCount = iStride * iHeight;

	byte* pCopyMask = new byte[iByteCount];
	memcpy(pCopyMask, pMask, iByteCount);
	memset(pMask, outsideColor, iByteCount);

	byte* pPixels = pMask;
	MTRange hRange(rect.iLeftX, rect.iRightX);
	for (int y = rect.iLeftY; y <= rect.iRightY; y++) {
		MTRange range = HorizontalDichotomize(pCirclePoints, iCount, hRange, y);
		if (range.m_iStart <= range.m_iEnd) {
			pPixels = pMask + y * iStride + range.m_iStart;
			byte* pCopyPixels = pCopyMask + y * iStride + range.m_iStart;
			for (int x = range.m_iStart; x <= range.m_iEnd; x++) {
				*pPixels++ = *pCopyPixels++;
			}
		}
	}
	SAFE_DELETE_ARRAY(pCopyMask);
	SAFE_DELETE_ARRAY(pCirclePoints);
	return true;
}

bool CMaskFillUtil::InsidePolygon(const Vector2 *pPolygon, int iCount, Vector2 point) {
	int iCounter = 0;
	for (int i = 1; i <= iCount; i++) {
		Vector2 p1 = pPolygon[i - 1], p2 = pPolygon[i];
		if (point.y > min(p1.y, p2.y) && point.y <= max(p1.y, p2.y) && point.x <= max(p1.x, p2.x) && p1.y != p2.y) {
			float fXInters = (point.y - p1.y)*(p2.x - p1.x) / (p2.y - p1.y) + p1.x;
			if (p1.x == p2.x || point.x <= fXInters) {
				iCounter++;
			}
		}
	}
	return (iCounter & 1) ? true : false;
}

CMaskFillUtil::MTRect CMaskFillUtil::OutsideRect(const Vector2 *pPolygon, int iCount, int iBoderX, int iBorderY) {
	float fMaxX = pPolygon[0].x;
	float fMinX = pPolygon[0].x;
	float fMaxY = pPolygon[0].y;
	float fMinY = pPolygon[0].y;
	for (int i = iCount; i > 0; i--) {
		if (pPolygon[i].x < fMinX) {
			fMinX = pPolygon[i].x;
		}
		else if (pPolygon[i].x > fMaxX) {
			fMaxX = pPolygon[i].x;
		}
		if (pPolygon[i].y < fMinY) {
			fMinY = pPolygon[i].y;
		}
		else if (pPolygon[i].y > fMaxY) {
			fMaxY = pPolygon[i].y;
		}
	}
	CMaskFillUtil::MTRect rect;
	rect.iLeftX = max(0, (int)fMinX);
	rect.iRightX = min((int)fMaxX + 1, iBoderX - 1);
	rect.iLeftY = max(0, (int)fMinY);
	rect.iRightY = min((int)fMaxY + 1, iBorderY - 1);
	return rect;
}

Vector2* CMaskFillUtil::GetCirclePoints(const Vector2 *pPoints, int iCount) {
	Vector2* pCirclePoints = new Vector2[iCount + 1];
	memcpy(pCirclePoints, pPoints, sizeof(Vector2)*iCount);
	pCirclePoints[iCount].x = pPoints[0].x;
	pCirclePoints[iCount].y = pPoints[0].y;
	return pCirclePoints;
}

CMaskFillUtil::MTRange CMaskFillUtil::VerticalDichotomize(const Vector2 *pPolygon, int iCount, const MTRange& range, int iX) {
	Vector2 p((float)(iX), 0.0f);
	int iLeft = range.m_iStart, iRight = range.m_iEnd;
	while (iLeft <= iRight) {
		p.y = (float)(iLeft);
		if (InsidePolygon(pPolygon, iCount, p)) {
			break;
		}
		iLeft++;
	}

	if (iRight >= iLeft) {
		int iStart = iLeft, iEnd = iRight, iMid = (iLeft + iRight) >> 1;
		int iKey = iRight;
		while (iStart <= iEnd) {
			iMid = (iStart + iEnd) >> 1;
			p.y = (float)(iMid);
			if (InsidePolygon(pPolygon, iCount, p)) {
				iStart = iMid + 1;
				iKey = iMid;
			}
			else {
				iEnd = iMid - 1;
			}
		}
		iRight = iKey;
	}
	return MTRange(iLeft, iRight);
}

CMaskFillUtil::MTRange CMaskFillUtil::HorizontalDichotomize(const Vector2 *pPolygon, int iCount, const MTRange& range, int iY) {
	Vector2 p(0.0f, (float)(iY));
	int iLeft = range.m_iStart, iRight = range.m_iEnd;
	while (iLeft <= iRight) {
		p.x = (float)(iLeft);
		if (InsidePolygon(pPolygon, iCount, p)) {
			break;
		}
		iLeft++;
	}

	if (iRight >= iLeft) {
		int iStart = iLeft, iEnd = iRight, iMid = (iLeft + iRight) >> 1;
		int iKey = iRight;
		while (iStart <= iEnd) {
			iMid = (iStart + iEnd) >> 1;
			p.x = (float)(iMid);
			if (InsidePolygon(pPolygon, iCount, p)) {
				iStart = iMid + 1;
				iKey = iMid;
			}
			else {
				iEnd = iMid - 1;
			}
		}
		iRight = iKey;
	}
	return MTRange(iLeft, iRight);
}
