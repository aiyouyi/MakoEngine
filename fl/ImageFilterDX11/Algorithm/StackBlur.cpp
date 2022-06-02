#include "StackBlur.h"

#include <algorithm>

	CStackBlur::CStackBlur(void)
	{
	}


	CStackBlur::~CStackBlur(void)
	{
	}
	
	typedef struct _StackBlurThreadParameterOptimized
	{
		BYTE* pData; int nWidth; int nHeight;
		int nIndex;
		int nRadius;
		BYTE* dv;
		int* stackPointerTable;
		int(*sir_r1_tab)[256];

		_StackBlurThreadParameterOptimized()
		{
			pData = NULL;
			dv = NULL;
			stackPointerTable = NULL;
			sir_r1_tab = NULL;
		}
	}StackBlurThreadParameterOptimized;


	void* BlurOneChannelExOptimized(void* pValue)
	{
		StackBlurThreadParameterOptimized* pStackBlur = (StackBlurThreadParameterOptimized*)pValue;
		int w = pStackBlur->nWidth;
		int h = pStackBlur->nHeight;
		int radius = pStackBlur->nRadius;
		BYTE* pImage = pStackBlur->pData;


		int wm = w - 1;
		int hm = h - 1;
		int wh = w * h;
		int div = radius + radius + 1;
		int r1 = radius + 1;

		BYTE *r_tmp = NULL;
		BYTE *r = (BYTE*)malloc(wh * sizeof(BYTE));
		//memset(r,0,wh*sizeof(BYTE));

		int divsum = (div + 1) >> 1;
		divsum *= divsum;

		BYTE *dv = pStackBlur->dv;

		BYTE *stack = (BYTE*)malloc(sizeof(BYTE)*(div));
		//memset(stack,0,sizeof(BYTE)*div);

		int rsum, x, y, i, yp, yw;
		int stackpointer;
		BYTE *sir = NULL;
		int rbs;
		int routsum;
		int rinsum;
		int stack_index = 0;
		yw = 0;
		unsigned char*pImage_tmp = pImage;

		// 存储((stackpointer + r1)%div)的值
		int* stackPointerTable = pStackBlur->stackPointerTable;

		// 存储 rbs * sir[] 的值
		int(*sir_r1_tab)[256] = pStackBlur->sir_r1_tab;

		///////////////////////////////////////////////////////////////////////////
		// horizontal blur

		int *vmin = (int*)malloc(sizeof(int)*(std::max)(w, h));
		//memset(vmin,0,sizeof(int)*(std::max)(w,h));
		for (int i = 0; i < w; ++i)
		{
			vmin[i] = (std::min)(i + r1, wm);
		}

		int *vmin2 = (int*)malloc(sizeof(int)*(radius + 1));
		//memset(vmin2,0,sizeof(int)*(radius + 1));
		for (int i = 1; i <= radius; ++i)
		{
			vmin2[i] = (std::min)(i, wm);
		}

		int count = (((r1 + 1) * r1) >> 1);
		for (y = 0; y < h; y++)
		{
			rinsum = routsum = rsum = 0;
			stack_index = 0;

			pImage_tmp = pImage + yw;
			for (int i = -radius; i <= 0; ++i)
			{
				sir = &stack[stack_index];
				++stack_index;

				sir[0] = pImage_tmp[0];
			}
			// 归并求和
			{
				rsum += sir[0] * count;
				routsum += sir_r1_tab[r1][sir[0]];
			}

			for (int i = 1; i <= radius; ++i)
			{
				sir = &stack[stack_index];
				++stack_index;

				sir[0] = pImage_tmp[vmin2[i]];

				rbs = r1 - i;
				rsum += sir_r1_tab[rbs][sir[0]];
				rinsum += sir[0];
			}

			stackpointer = radius;

			pImage_tmp = pImage + yw;
			r_tmp = r + yw;
			for (x = 0; x < w; x++)
			{
				r_tmp[0] = dv[rsum];
				++r_tmp;

				rsum -= routsum;

				sir = &stack[stackPointerTable[stackpointer]];

				routsum -= sir[0];

				sir[0] = pImage_tmp[vmin[x]];

				rinsum += sir[0];
				rsum += rinsum;

				stackpointer = stackpointer + 1 == div ? 0 : stackpointer + 1;
				sir = &stack[stackpointer];

				routsum += sir[0];
				rinsum -= sir[0];
			}
			yw += w;
		}  // end horizontal


		///////////////////////////////////////////////////////////////////////////
		// vertical blur

		for (y = 0; y < h; y++)
		{
			vmin[y] = (std::min)(y + r1, hm) * w;
		}

		for (x = 0; x < w; x++)
		{
			rinsum = routsum = rsum = 0;

			yp = -radius * w;
			stack_index = 0;

			r_tmp = r + (std::max)(0, yp) + x;
			for (i = -radius; i <= 0; ++i)
			{
				sir = &stack[stack_index];
				++stack_index;

				sir[0] = r_tmp[0];

				rbs = r1 + i;
				rsum += sir_r1_tab[rbs][sir[0]];
				routsum += sir[0];

				if (i < hm)
				{
					yp += w;
					r_tmp = r + (std::max)(0, yp) + x;
				}
			}

			r_tmp = r + yp + x;
			for (i = 1; i <= radius; ++i)
			{
				sir = &stack[stack_index];
				++stack_index;

				sir[0] = r_tmp[0];

				rbs = r1 - i;
				rsum += sir_r1_tab[rbs][sir[0]];
				rinsum += sir[0];

				if (i < hm)
				{
					yp += w;
					r_tmp = r + yp + x;
				}
			}

			stackpointer = radius;
			r_tmp = r + x;
			pImage_tmp = pImage + x;
			for (y = 0; y < h; y++)
			{
				pImage_tmp[0] = dv[rsum];
				pImage_tmp += w;

				rsum -= routsum;
				sir = &stack[stackPointerTable[stackpointer]];
				routsum -= sir[0];

				sir[0] = r_tmp[vmin[y]];
				rinsum += sir[0];
				rsum += rinsum;

				stackpointer = stackpointer + 1 == div ? 0 : stackpointer + 1;
				sir = &stack[stackpointer];

				routsum += sir[0];
				rinsum -= sir[0];
			}
		}

		free(r);
		free(vmin);
		free(vmin2);
		free(stack);
		return ((void*)100);
	}

	void BlurOneChannelOptimized(BYTE* pImage, int w, int h, int radius, int thread)
	{
		// 初始化信息
		int div = radius + radius + 1;
		int r1 = radius + 1;

		int divsum = (div + 1) >> 1;
		divsum *= divsum;

		BYTE *dv = (BYTE*)malloc(sizeof(BYTE)*(256 * divsum));
		int cur_offset = 0;
		for (int i = 0; i < 256; ++i)
		{
			memset(dv + cur_offset, i, divsum);
			cur_offset += (divsum);
		}

		// 存储((stackpointer + r1)%div)的值
		int* stackPointerTable = new int[div];
		for (int i = 0; i < div; ++i)
		{
			stackPointerTable[i] = ((i + r1) % div);
		}

		// 存储 rbs * sir[] 的值
		int *sir_r1_tab_real = new int[256 * (r1 + 1)];
		int(*sir_r1_tab)[256] = (int(*)[256])sir_r1_tab_real;
		for (int i = 0; i < r1 + 1; ++i)
		{
			for (int j = 0; j < 256; ++j)
			{
				sir_r1_tab[i][j] = i * j;
			}
		}

		StackBlurThreadParameterOptimized pParameter;

		pParameter.nWidth = w;
		pParameter.nHeight = h;
		pParameter.pData = pImage;
		pParameter.nRadius = radius;
		pParameter.dv = dv;
		pParameter.stackPointerTable = stackPointerTable;
		pParameter.sir_r1_tab = sir_r1_tab;
		BlurOneChannelExOptimized(&pParameter);
		

		free(dv);
		SAFE_DELETE(stackPointerTable);
		SAFE_DELETE_ARRAY(sir_r1_tab_real);
	}



	void CStackBlur::Run(BYTE* pImage, int nWidth, int nHeight, int nStride, int nRadius)
	{
		if (nRadius <= 0 || nWidth == 0 || nHeight == 0)
		{
			return;
		}

		int nChannelCount = nStride / nWidth;

		if (nChannelCount == 1)
		{
			//this->RunChannel1(pImage, nWidth, nHeight, nRadius);
			BlurOneChannelOptimized(pImage, nWidth, nHeight, nRadius, 1);
		}
		else
		{
			//this->RunChannel4(pImage, nWidth, nHeight, nRadius);
			//SFDSP::stackBlurOptimizedThreadN(pImage, nWidth, nHeight, nRadius, SFDSP::GetCPUCount());
		}
	}

	void CStackBlur::RunChannel1(BYTE* pImage, int nWidth, int nHeight, int nRadius)
	{
		int nPixelCount = nWidth * nHeight;

		int nMaxW = nWidth - 1;
		int nMaxH = nHeight - 1;
		int nDiameter = 2 * nRadius + 1;
		int r1 = nRadius + 1;

		BYTE* pTblR = new BYTE[nPixelCount];
		BYTE* ptrTblR = pTblR;

		int rSum, rInSum, rOutSum;
		BYTE* ptrImage = pImage, *ptrImage2 = pImage, *ptrImage3 = pImage;
		BYTE* ptrHeader = pImage;
		BYTE rValue;

		// 计算点乘表
		int var1 = r1 * (r1 + 1) / 2;
		int var1MulTbl[256], r1MulTbl[256];
		int sumVar1 = 0, sumR1 = 0;
		for (int i = 0; i < 256; ++i)
		{
			var1MulTbl[i] = sumVar1;
			r1MulTbl[i] = sumR1;

			sumVar1 += var1;
			sumR1 += r1;
		}

		// 除运算查找表
		int nDivSum = r1 * r1;   // 权重累加和 (r + 1) + 2 * (1 + r)/2 * r)
		BYTE* dv = new BYTE[256 * nDivSum];
		int curOffset = 0;
		for (int i = 0; i < 256; ++i)
		{
			memset(dv + curOffset, i, nDivSum);
			curOffset += nDivSum;
		}

		// 偏移查找表
		int nVTblLen = (std::max)(nWidth, nHeight);
		int* vForwardR1 = new int[nVTblLen];
		int* vBackwardR = new int[nVTblLen];

		for (int x = 0; x < nWidth; ++x)
		{
			vForwardR1[x] = (std::min)(x + r1, nMaxW);  // 水平方向向前
			vBackwardR[x] = (std::max)(x - nRadius, 0);   // 水平方向向后
		}

		// 横向
		for (int y = 0; y < nHeight; ++y)
		{
			ptrImage = ptrHeader;
			rValue = ptrImage[0];

			// 2r +1范围内统计
			// [-r, 0]
			rSum = var1MulTbl[rValue];  // rValue * var1;
			rOutSum = r1MulTbl[rValue];  // rValue * r1;

			// [1, r]
			rInSum = 0;
			for (int r = 1; r <= nRadius; ++r)
			{
				if (r < nWidth)
				{
					ptrImage++;
				}

				rValue = ptrImage[0];
				rInSum += rValue;
				rSum += rValue * (r1 - r);
			}

			ptrImage3 = ptrHeader;
			for (int x = 0; x < nMaxW; ++x)
			{
				*ptrTblR++ = dv[rSum];

				ptrImage = ptrHeader + vForwardR1[x];
				ptrImage2 = ptrHeader + vBackwardR[x];
				ptrImage3++;

				rInSum += ptrImage[0];
				rSum += (rInSum - rOutSum);
				rOutSum += (ptrImage3[0] - ptrImage2[0]);
				rInSum -= ptrImage3[0];
			}

			*ptrTblR++ = dv[rSum];
			ptrImage = ptrHeader + vForwardR1[nMaxW];
			ptrImage2 = ptrHeader + vBackwardR[nMaxW];
			ptrImage3 = ptrHeader + nMaxW;

			rInSum += ptrImage[0];
			rSum += (rInSum - rOutSum);
			rOutSum += (ptrImage3[0] - ptrImage2[0]);
			rInSum -= ptrImage3[0];

			ptrHeader += nWidth;
		}

		// 纵向
		ptrHeader = pImage;
		int index = 0, index2 = 0, index3 = 0;

		// 更新索引表
		for (int y = 0; y < nHeight; ++y)
		{
			vForwardR1[y] = nWidth * (std::min)(y + r1, nMaxH);
			vBackwardR[y] = nWidth * (std::max)(y - nRadius, 0);   // 水平方向向后
		}

		for (int x = 0; x < nWidth; ++x)
		{
			index = x;
			rValue = pTblR[index];

			// 2r +1范围内统计
			// [-r, 0]
			rSum = var1MulTbl[rValue];
			rOutSum = r1MulTbl[rValue];

			// [1, r]
			rInSum = 0;
			for (int r = 1; r <= nRadius; ++r)
			{
				if (r < nHeight)
				{
					index += nWidth;
				}

				rValue = pTblR[index];
				rInSum += rValue;
				rSum += rValue * (r1 - r);
			}

			index2 = x;
			ptrImage = ptrHeader;
			for (int y = 0; y < nMaxH; ++y)
			{
				ptrImage[0] = dv[rSum];
				index = x + vForwardR1[y];
				index2 += nWidth;
				index3 = x + vBackwardR[y];

				rInSum += pTblR[index];
				rSum += (rInSum - rOutSum);
				rOutSum += (pTblR[index2] - pTblR[index3]);
				rInSum -= pTblR[index2];

				ptrImage += nWidth;
			}

			ptrImage[0] = dv[rSum];
			index = x + vForwardR1[nMaxH];
			// index2指向末行，不下移
			index3 = x + vBackwardR[nMaxH];

			rInSum += pTblR[index];
			rSum += (rInSum - rOutSum);
			rOutSum += (pTblR[index2] - pTblR[index3]);
			rInSum -= pTblR[index2];

			ptrImage += nWidth;
			ptrHeader++;
		}

		SAFE_DELETE_ARRAY(pTblR);
		SAFE_DELETE_ARRAY(vBackwardR);
		SAFE_DELETE_ARRAY(vForwardR1);
		SAFE_DELETE_ARRAY(dv);
	}

	/*************************************************************************
	** FunctionName：       ExpansionErosionBinary
	** @Param pMask：	    二值图像，单通道，0或255
	** @Param nWidth：	    图像的宽
	** @Param nHeight：	    图像的高
	** @Param nRaid：	    结构体的半径，结构体为全1数组
	** @Param Expansion：   true为膨胀，false为腐蚀
	** Comment:             根据积分图的思路对指定半径的模板进行腐蚀或膨胀操作
	** return：             void
	** Creator：            Lym
	** Date：		        2015-01-19
	** Modifier：
	** ModifyDate：
	** Version：
	*************************************************************************/
	void ExpansionErosionBinary(BYTE *pMask, int nWidth, int nHeight, int nRaid, bool Expansion)
	{
		if (!Expansion)
		{
			int nSize = nWidth * nHeight;
			for (int i = 0; i < nSize; i++)
			{
				if (pMask[i] == 255)
				{
					pMask[i] = 0;
				}
				else
				{
					pMask[i] = 255;
				}
			}
		}
		BYTE value = 0;
		BYTE midValue = 100;
		int TempWidth = nWidth + 1, TempHeight = nHeight + 1;
		int TempArea = TempWidth * TempHeight;
		int *SumTable = new int[TempArea];
		int *rowTable = new int[TempWidth];
		memset(SumTable, 0, sizeof(int)*TempWidth);
		BYTE *ptrImage = pMask;
		int *ptrSum = SumTable;
		for (int ii = 1; ii < TempHeight; ii++)
		{
			int *ptrS = ptrSum;
			ptrSum += TempWidth;
			*ptrSum = 0;
			*rowTable = 0;
			for (int jj = 1; jj < TempWidth; jj++)
			{
				rowTable[jj] = rowTable[jj - 1] + ptrImage[jj - 1];
				ptrSum[jj] = ptrS[jj] + rowTable[jj];
			}
			ptrImage += nWidth;
		}

		ptrImage = pMask;
		ptrSum = SumTable;
		for (int i = 0; i < nHeight; i++)
		{
			int beginI = i - nRaid;
			beginI = beginI > 0 ? beginI : 0;
			int endI = i + 1 + nRaid;
			endI = endI < nHeight ? endI : nHeight;
			beginI *= TempWidth;
			endI *= TempWidth;
			for (int j = 0; j < nWidth; j++)
			{
				if (ptrImage[j] == 0)
				{
					int beginJ = j - nRaid;
					beginJ = beginJ > 0 ? beginJ : 0;
					int endJ = j + 1 + nRaid;
					endJ = endJ < nWidth ? endJ : nWidth;
					int Apos = beginI + beginJ;
					int Bpos = beginI + endJ;
					int Cpos = endI + beginJ;
					int Dpos = endI + endJ;
					int tmp = (ptrSum[Dpos] - ptrSum[Cpos] - ptrSum[Bpos] + ptrSum[Apos]);
					if (tmp > 0)
					{
						ptrImage[j] = 255;
					}
				}
			}
			ptrImage += nWidth;
		}
		if (!Expansion)
		{
			int nSize = nWidth * nHeight;
			for (int i = 0; i < nSize; i++)
			{
				if (pMask[i] == 255)
				{
					pMask[i] = 0;
				}
				else
				{
					pMask[i] = 255;
				}
			}
		}

		SAFE_DELETE_ARRAY(SumTable);
		SAFE_DELETE_ARRAY(rowTable);
	}


	void CStackBlur::OneChannelAvg(BYTE *pDmain, int nWidth, int nHeight, int nRaid) {
		int TempWidth = nWidth + 1, TempHeight = nHeight + 1;
		int TempArea = TempWidth * TempHeight;
		int *SumTable = new int[TempArea];
		int *rowTable = new int[TempWidth];
		int nborderW = nWidth - 1, nborderH = nHeight - 1;
		int *ptrS1 = SumTable;
		memset(SumTable, 0, sizeof(int)*TempWidth);
		BYTE *ptrD = pDmain;
		for (int ii = 1; ii < TempHeight; ii++)
		{
			int *ptrS2 = ptrS1;
			ptrS1 += TempWidth;
			*ptrS1 = 0;
			*rowTable = 0;
			for (int jj = 1; jj < TempWidth; jj++)
			{
				rowTable[jj] = rowTable[jj - 1] + ptrD[jj - 1];
				ptrS1[jj] = ptrS2[jj] + rowTable[jj];
			}
			ptrD += nWidth;
		}
		int scale = (nRaid * 2 + 1)*(nRaid * 2 + 1);
		int R2 = nRaid * 2 + 1;
		int R2W = (nRaid * 2 + 1)*TempWidth;
		for (int ii = 0; ii < nHeight; ii++)
		{
			BYTE *outptr = pDmain + ii * nWidth;
			int beginI = ii - nRaid;
			beginI = beginI > 0 ? beginI : 0;
			int endI = ii + 1 + nRaid;
			endI = endI < nHeight ? endI : nHeight;
			int numI = (endI - beginI);
			beginI *= TempWidth;
			endI *= TempWidth;
			for (int jj = 0; jj < nWidth; jj++)
			{

				int beginJ = jj - nRaid;
				beginJ = beginJ > 0 ? beginJ : 0;
				int endJ = jj + 1 + nRaid;
				endJ = endJ < nWidth ? endJ : nWidth;
				int num = numI * (endJ - beginJ);
				int Apos = beginI + beginJ;
				int Bpos = beginI + endJ;
				int Cpos = endI + beginJ;
				int Dpos = endI + endJ;
				BYTE tmp = (SumTable[Dpos] - SumTable[Cpos] - SumTable[Bpos] + SumTable[Apos]) / num;
				outptr[jj] = tmp;	
			}
		}	
		SAFE_DELETE_ARRAY(SumTable);
		SAFE_DELETE_ARRAY(rowTable);
	}

	void CStackBlur::OneChannelAvg(Vector2 * pDmain, int nWidth, int nHeight, int nRaid)
	{

		int TempWidth = nWidth + 1, TempHeight = nHeight + 1;
		int TempArea = TempWidth * TempHeight;
		Vector2 *SumTable = new Vector2[TempArea];
		Vector2 *rowTable = new Vector2[TempWidth];
		Vector2 *ptrS1 = SumTable;
		memset(SumTable, 0, sizeof(Vector2)*TempWidth);
		Vector2 *ptrD = pDmain;
		for (int ii = 1; ii < TempHeight; ii++)
		{
			Vector2 *ptrS2 = ptrS1;
			ptrS1 += TempWidth;
			*ptrS1 = 0;
			*rowTable = 0;
			for (int jj = 1; jj < TempWidth; jj++)
			{
				rowTable[jj] = rowTable[jj - 1] + ptrD[jj - 1];
				ptrS1[jj] = ptrS2[jj] + rowTable[jj];
			}
			ptrD += nWidth;
		}
		for (int ii = 0; ii < nHeight; ii++)
		{
			Vector2 *outptr = pDmain + ii * nWidth;
			int beginI = ii  - nRaid;
			beginI = beginI > 0 ? beginI : 0;
			int endI = ii  +1+ nRaid;
			endI = endI < nHeight ? endI : nHeight;
			int numI = (endI - beginI);
			beginI *= TempWidth;
			endI *= TempWidth;
			for (int jj = 0; jj < nWidth; jj++)
			{

				int beginJ = jj  - nRaid;
				beginJ = beginJ > 0 ? beginJ : 0;
				int endJ = jj + 1 + nRaid;
				endJ = endJ < nWidth ? endJ : nWidth;
				int num = numI * (endJ - beginJ);
				int Apos = beginI + beginJ;
				int Bpos = beginI + endJ;
				int Cpos = endI + beginJ;
				int Dpos = endI + endJ;
				Vector2 tmp = (SumTable[Dpos] - SumTable[Cpos] - SumTable[Bpos] + SumTable[Apos]) / num;
				outptr[jj] = tmp;
			}
		}
		SAFE_DELETE_ARRAY(SumTable);
		SAFE_DELETE_ARRAY(rowTable);
	}

