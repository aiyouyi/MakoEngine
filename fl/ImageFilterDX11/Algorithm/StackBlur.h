
#pragma once

#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
class  CStackBlur
{
public:
	CStackBlur(void);
	~CStackBlur(void);

public:
	void Run(BYTE* pImage, int nWidth, int nHeight, int nStride, int nRadius);
	void OneChannelAvg(BYTE *pDmain, int nWidth, int nHeight, int nRaid);
	void OneChannelAvg(Vector2 *pDmain, int nWidth, int nHeight, int nRaid);

private:
	void RunChannel4(BYTE* pImage, int nWidth, int nHeight, int nRadius);
	void RunChannel1(BYTE* pImage, int nWidth, int nHeight, int nRadius);
};
void ExpansionErosionBinary(BYTE *pMask, int nWidth, int nHeight, int nRaid, bool Expansion);