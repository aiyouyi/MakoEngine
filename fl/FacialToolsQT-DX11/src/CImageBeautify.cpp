#include "CImageBeautify.h"
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include <algorithm>

#ifndef SAFE_DELETE_PTR
#define SAFE_DELETE_PTR(x) { if (x) delete [] (x); (x) = nullptr; }
#endif

CImageBeautify::CImageBeautify()
{
	m_Output = NULL;
	m_leftFactorBuffer = NULL;
	m_leftColorBuffer = NULL;
	m_rightColorBuffer = NULL;
	m_rightFactorBuffer = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
}


CImageBeautify::~CImageBeautify()
{
}

void CImageBeautify::FilterLUT(unsigned char * pImage, unsigned char * pRefMap, int nWidth, int nHeight)
{
	int nColorBegin = 0; 
	int nColorEnd = 3;
	int pDevMap[256] = { 0,31,63,95,127,159,191,223,255,287,319,351,383,415,447,479,511,543,575,607,639,671,703,735,767,799,831,863,895,927,959,991,1023,1055,1087,1119,1151,1183,1215,1247,1279,1311,1343,1375,1407,1439,1471,1503,1535,1567,1599,1631,1663,1695,1727,1759,1791,1823,1855,1887,1919,1951,1983,2015,2047,2079,2111,2143,2175,2207,2239,2271,2303,2335,2367,2399,2431,2463,2495,2527,2559,2591,2623,2655,2687,2719,2751,2783,2815,2847,2879,2911,2943,2975,3007,3039,3071,3103,3135,3167,3199,3231,3263,3295,3327,3359,3391,3423,3455,3487,3519,3551,3583,3615,3647,3679,3711,3743,3775,3807,3839,3871,3903,3935,3967,3999,4031,4063,4095,4127,4159,4191,4223,4255,4287,4319,4351,4383,4415,4447,4479,4511,4543,4575,4607,4639,4671,4703,4735,4767,4799,4831,4863,4895,4927,4959,4991,5023,5055,5087,5119,5151,5183,5215,5247,5279,5311,5343,5375,5407,5439,5471,5503,5535,5567,5599,5631,5663,5695,5727,5759,5791,5823,5855,5887,5919,5951,5983,6015,6047,6079,6111,6143,6175,6207,6239,6271,6303,6335,6367,6399,6431,6463,6495,6527,6559,6591,6623,6655,6687,6719,6751,6783,6815,6847,6879,6911,6943,6975,7007,7039,7071,7103,7135,7167,7199,7231,7263,7295,7327,7359,7391,7423,7455,7487,7519,7551,7583,7615,7647,7679,7711,7743,7775,7807,7839,7871,7903,7935,7967,7999,8031,8063,8095,8127,8159 };

	// 在基准图的偏移位置
	int pROffset[65] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,252 };
	int pGOffset[65] = { 0, 2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432, 20480, 22528, 24576, 26624, 28672, 30720,32768,34816,36864,38912,40960,43008,45056,47104,49152,51200,53248,55296,57344,59392,61440,63488,65536,67584,69632,71680,73728,75776,77824,79872,81920,83968,86016,88064,90112,92160,94208,96256,98304,100352,102400,104448,106496,108544,110592,112640,114688,116736,118784,120832,122880,124928,126976,129024,129024 };
	int pBOffset[65] = { 0, 256, 512, 768, 1024, 1280, 1536, 1792, 131072, 131328, 131584, 131840, 132096, 132352, 132608, 132864,262144,262400,262656,262912,263168,263424,263680,263936,393216,393472,393728,393984,394240,394496,394752,395008,524288,524544,524800,525056,525312,525568,525824,526080,655360,655616,655872,656128,656384,656640,656896,657152,786432,786688,786944,787200,787456,787712,787968,788224,917504,917760,918016,918272,918528,918784,919040,919296,919296 };

	int superR, superG, superB, rValue, gValue, bValue;
	int r1, g1, b1, r2, g2, b2;
	int rDiff, gDiff, bDiff;
	int MID[4], MID2[2];
	unsigned char* ptrImage = pImage;
	unsigned char* ptrAddr = 0;

	for (int row = 0; row < nHeight; ++row)
	{
		for (int col = 0; col < nWidth; ++col)
		{
			superR = pDevMap[ptrImage[CC_RED]];
			superG = pDevMap[ptrImage[CC_GREEN]];
			superB = pDevMap[ptrImage[CC_BLUE]];
			// 获取整数位
			rValue = superR >> 7;
			gValue = superG >> 7;
			bValue = superB >> 7;
			// 获取在素材图的偏移位置-
			r1 = pROffset[rValue], r2 = pROffset[rValue + 1];
			g1 = pGOffset[gValue], g2 = pGOffset[gValue + 1];
			b1 = pBOffset[bValue], b2 = pBOffset[bValue + 1];
			// 获取小数位
			rDiff = superR & 0x7F;
			gDiff = superG & 0x7F;
			bDiff = superB & 0x7F;

			for (int i = nColorBegin; i < nColorEnd; ++i)
			{
				ptrAddr = pRefMap + b1 + g1;
				MID[0] = (ptrAddr[r1 + i] << 7) + (ptrAddr[r2 + i] - ptrAddr[r1 + i]) * rDiff;
				ptrAddr = pRefMap + b1 + g2;
				MID[1] = (ptrAddr[r1 + i] << 7) + (ptrAddr[r2 + i] - ptrAddr[r1 + i]) * rDiff;
				ptrAddr = pRefMap + b2 + g1;
				MID[2] = (ptrAddr[r1 + i] << 7) + (ptrAddr[r2 + i] - ptrAddr[r1 + i]) * rDiff;
				ptrAddr = pRefMap + b2 + g2;
				MID[3] = (ptrAddr[r1 + i] << 7) + (ptrAddr[r2 + i] - ptrAddr[r1 + i]) * rDiff;

				MID2[0] = MID[0] + (((MID[1] - MID[0]) * gDiff + 64) >> 7);
				MID2[1] = MID[2] + (((MID[3] - MID[2]) * gDiff + 64) >> 7);

				int FilterColor =((MID2[0] + 64) >> 7) + (((MID2[1] - MID2[0]) * bDiff + 128) >> 14);
				if (FilterColor <0)
				{
					FilterColor = 0;
				}
				else if (FilterColor > 255)
				{
					FilterColor =255;
				}

				ptrImage[i] = FilterColor;
			}

			ptrImage += 4;
		}
	}

}

void CImageBeautify::SmoothImage(unsigned char * pImage, int nWidth, int nHeight, float sigma_s, float sigma_r, int num_iterations)
{
	int width = nWidth;
	int height = nHeight;
	int channel = 4;
	int size = width * height;
	int stride = width * 4;
	//求X方向和Y方向上的积分。x
	int *dHdx = new int[size];
	int *dVdy = new int[size];

	double ratio = sigma_s / sigma_r;

	//X方向的积分
	int pos = 0;
	unsigned char* PreData, *PosData;
	for (int i = 0; i < height; ++i)
	{
		dHdx[pos] = 0;
		++pos;
		PreData = pImage + i * stride;
		PosData = PreData + 4;

		for (int j = 1; j < width; ++j, ++pos, PreData = PosData, PosData += channel)
		{
			dHdx[pos] = abs(PreData[0] - PosData[0]);
			dHdx[pos] += abs(PreData[1] - PosData[1]);
			dHdx[pos] += abs(PreData[2] - PosData[2]);
		}
	}

	//Y方向的积分
	pos = width;
	memset(dVdy, 0, width);
	for (int i = 1; i < height; ++i)
	{
		PreData = pImage + (i - 1)*stride;
		PosData = PreData + stride;
		for (int j = 0; j < width; ++j, PreData += channel, PosData += channel, ++pos)
		{
			dVdy[pos] = abs(PreData[0] - PosData[0]);
			dVdy[pos] += abs(PreData[1] - PosData[1]);
			dVdy[pos] += abs(PreData[2] - PosData[2]);
		}
	}
	int N = num_iterations;
	double sigma_H = sigma_s;

	unsigned char* Table = new unsigned char[766 * 511];
	for (int i = 0; i < num_iterations; ++i)
	{
		float sigma_H_i = (float)(sigma_H * sqrt(double(3)) * pow(double(2), N - i - 1) / sqrt(pow(double(4), N) - 1));

		float a = exp(-sqrt(2.0f) / sigma_H_i);

		int pos = 0;

		for (int i = 0; i < 766; ++i)
		{
			float tmpRatio = (float)(pow(double(a), int(ratio * i + 1)));

			for (int j = 0; j < 511; ++j, ++pos)
			{
				int nTemp = (int)(tmpRatio*(j - 255) + 0.5f);
				if (nTemp < 0)
				{
					Table[pos] = 256 + nTemp;
				}
				else
				{
					Table[pos] = nTemp;
				}
			}
		}

		RFBoxFilter_Horizontal(pImage, width, height, stride, dHdx, Table);
		RFBoxFilter_Vertical(pImage, width, height, stride, dVdy, Table);
	}
	delete []Table;
	delete[]dHdx;
	delete[]dVdy;
}



void CImageBeautify::RFBoxFilter_Horizontal(unsigned char * imgData, int width, int height, int stride, int * xform_domain_position, unsigned char * Table)
{
	int channels = stride / width;

	unsigned char* preIMG, *posIMG;
	int* ratioPos;
	int tmpPos;
	for (int i = 0; i < height; ++i)
	{
		ratioPos = xform_domain_position + i * width + 1;
		preIMG = imgData + i * stride;
		posIMG = preIMG + channels;

		for (int j = 1; j < width; ++j, ++ratioPos, preIMG = posIMG, posIMG += channels)
		{
			tmpPos = *ratioPos * 511 + 255;
			posIMG[0] = posIMG[0] + Table[tmpPos + (preIMG[0] - posIMG[0])];
			posIMG[1] = posIMG[1] + Table[tmpPos + (preIMG[1] - posIMG[1])];
			posIMG[2] = posIMG[2] + Table[tmpPos + (preIMG[2] - posIMG[2])];
		}
	}

	for (int i = 0; i < height; ++i)
	{
		ratioPos = xform_domain_position + (i + 1)*width - 1;
		posIMG = imgData + (i + 1)*stride - channels;
		preIMG = posIMG - channels;
		for (int j = width - 2; j >= 0; --j, --ratioPos, posIMG = preIMG, preIMG -= channels)
		{
			tmpPos = *ratioPos * 511 + 255;
			preIMG[0] = preIMG[0] + Table[tmpPos + (posIMG[0] - preIMG[0])];
			preIMG[1] = preIMG[1] + Table[tmpPos + (posIMG[1] - preIMG[1])];
			preIMG[2] = preIMG[2] + Table[tmpPos + (posIMG[2] - preIMG[2])];
		}
	}
}

void CImageBeautify::RFBoxFilter_Vertical(unsigned char * imgData, int width, int height, int stride, int * yform_domain_position, unsigned char * Table)
{
	int channels = stride / width;
	unsigned char* preIMG, *posIMG;
	int* ratioPos;
	int tmpPos;
	for (int i = 1; i < height; ++i)
	{
		preIMG = imgData + (i - 1)*stride;
		posIMG = preIMG + stride;
		ratioPos = yform_domain_position + i * width;
		for (int j = 0; j < width; ++j, ++ratioPos, preIMG += channels, posIMG += channels)
		{
			tmpPos = *ratioPos * 511 + 255;
			posIMG[0] = posIMG[0] + Table[tmpPos + (preIMG[0] - posIMG[0])];
			posIMG[1] = posIMG[1] + Table[tmpPos + (preIMG[1] - posIMG[1])];
			posIMG[2] = posIMG[2] + Table[tmpPos + (preIMG[2] - posIMG[2])];
		}
	}

	for (int i = height - 2; i >= 0; --i)
	{
		preIMG = imgData + i * stride;
		posIMG = preIMG + stride;

		ratioPos = yform_domain_position + (i + 1)*width;
		for (int j = 0; j < width; ++j, ++ratioPos, preIMG += channels, posIMG += channels)
		{
			tmpPos = *ratioPos * 511 + 255;
			preIMG[0] = preIMG[0] + Table[tmpPos + (posIMG[0] - preIMG[0])];
			preIMG[1] = preIMG[1] + Table[tmpPos + (posIMG[1] - preIMG[1])];
			preIMG[2] = preIMG[2] + Table[tmpPos + (posIMG[2] - preIMG[2])];
		}
	}
}


void CImageBeautify::SmoothImage2(unsigned char * Input, int Width, int Height, float sigmaSpatial, float sigmaRange)
{
	int Channels = 4;
	int reserveWidth = Width;
	int reserveHeight = Height;
	int reservePixels = reserveWidth * reserveHeight;
	int numberOfPixels = reservePixels * Channels;

	// basic sanity check
	if (m_nWidth!=Width||m_nHeight!=Height)
	{
		m_nHeight = Height;
		m_nWidth = Width;
		SAFE_DELETE_PTR(m_leftColorBuffer);
		SAFE_DELETE_PTR(m_leftFactorBuffer);
		SAFE_DELETE_PTR(m_rightColorBuffer);
		SAFE_DELETE_PTR(m_rightFactorBuffer);
		SAFE_DELETE_PTR(m_Output);
		m_leftColorBuffer = new float[numberOfPixels];
		m_leftFactorBuffer = new float[reservePixels];
		m_rightColorBuffer = new float[numberOfPixels];
		m_rightFactorBuffer = new float[reservePixels];
		m_Output = new unsigned char[numberOfPixels];
	}


	float* leftColorBuffer = m_leftColorBuffer; 
	float* leftFactorBuffer = m_leftFactorBuffer;
	float* rightColorBuffer = m_rightColorBuffer;
	float* rightFactorBuffer = m_rightFactorBuffer;


	float* downColorBuffer = leftColorBuffer;
	float* downFactorBuffer = leftFactorBuffer;
	float* upColorBuffer = rightColorBuffer;
	float* upFactorBuffer = rightFactorBuffer;
	// compute a lookup table
	float alpha_f = static_cast<float>(exp(-sqrt(2.0) / (sigmaSpatial * 255)));
	float inv_alpha_f = 1.f - alpha_f;


	float range_table_f[255 + 1];
	float inv_sigma_range = 1.0f / (sigmaRange);

	float ii = 0.f;
	for (int i = 0; i <= 255; i++, ii -= 1.f)
	{
		range_table_f[i] = alpha_f * exp(ii * inv_sigma_range);
	}
	
	CRB_HorizontalFilter(Input, m_Output, Width, Height, Channels, range_table_f, inv_alpha_f, leftColorBuffer, leftFactorBuffer, rightColorBuffer, rightFactorBuffer);

	CRB_VerticalFilter(Input, m_Output, Width, Height, Channels, range_table_f, inv_alpha_f, downColorBuffer, downFactorBuffer, upColorBuffer, upFactorBuffer);
	memcpy(Input, m_Output, 4 * Width*Height);
}

int CImageBeautify::getDiffFactor(const unsigned char * color1, const unsigned char * color2)
{
	int final_diff;
	int component_diff[3];

	// find absolute difference between each component
	for (int i = 0; i < 3; i++)
	{
		component_diff[i] = abs(color1[i] - color2[i]);
	}

	final_diff = ((component_diff[0] + component_diff[1] + component_diff[2] ) >> 2);

	return final_diff;
}

void CImageBeautify::CRB_HorizontalFilter(unsigned char * Input, unsigned char * Output, int Width, int Height, int Channels, float * range_table_f, float inv_alpha_f, float * left_Color_Buffer, float * left_Factor_Buffer, float * right_Color_Buffer, float * right_Factor_Buffer)
{
	// Left pass and Right pass 

	int Stride = Width * Channels;
	const unsigned char* src_left_color = Input;
	float* left_Color = left_Color_Buffer;
	float* left_Factor = left_Factor_Buffer;

	int last_index = Stride * Height - 1;
	const unsigned char* src_right_color = Input + last_index;
	float* right_Color = right_Color_Buffer + last_index;
	float* right_Factor = right_Factor_Buffer + Width * Height - 1;

	for (int y = 0; y < Height; y++)
	{
		const unsigned char* src_left_prev = Input;
		const float* left_prev_factor = left_Factor;
		const float* left_prev_color = left_Color;

		const unsigned char* src_right_prev = src_right_color;
		const float* right_prev_factor = right_Factor;
		const float* right_prev_color = right_Color;

		// process 1st pixel separately since it has no previous
		{
			//if x = 0 
			*left_Factor++ = 1.f;
			*right_Factor-- = 1.f;
			for (int c = 0; c < Channels; c++)
			{
				*left_Color++ = *src_left_color++;
				*right_Color-- = *src_right_color--;
			}
		}
		// handle other pixels
		for (int x = 1; x < Width; x++)
		{
			// determine difference in pixel color between current and previous
			// calculation is different depending on number of channels
			int left_diff = getDiffFactor(src_left_color, src_left_prev);
			src_left_prev = src_left_color;

			int right_diff = getDiffFactor(src_right_color, src_right_color - Channels);
			src_right_prev = src_right_color;

			float left_alpha_f = range_table_f[left_diff];
			float right_alpha_f = range_table_f[right_diff];
			*left_Factor++ = inv_alpha_f + left_alpha_f * (*left_prev_factor++);
			*right_Factor-- = inv_alpha_f + right_alpha_f * (*right_prev_factor--);

			for (int c = 0; c < Channels; c++)
			{
				*left_Color++ = (inv_alpha_f * (*src_left_color++) + left_alpha_f * (*left_prev_color++));
				*right_Color-- = (inv_alpha_f * (*src_right_color--) + right_alpha_f * (*right_prev_color--));
			}
		}
	}
	// vertical pass will be applied on top on horizontal pass, while using pixel differences from original image
	// result color stored in 'leftcolor' and vertical pass will use it as source color
	{
		unsigned char* dst_color = Output; // use as temporary buffer  
		const float* leftcolor = left_Color_Buffer;
		const float* leftfactor = left_Factor_Buffer;
		const float* rightcolor = right_Color_Buffer;
		const float* rightfactor = right_Factor_Buffer;

		int width_height = Width * Height;
		for (int i = 0; i < width_height; i++)
		{
			// average color divided by average factor
			float factor = 1.f / ((*leftfactor++) + (*rightfactor++));
			for (int c = 0; c < Channels; c++)
			{

				*dst_color++ = (factor * ((*leftcolor++) + (*rightcolor++)));

			}
		}
	}
}

void CImageBeautify::CRB_VerticalFilter(unsigned char * Input, unsigned char * Output, int Width, int Height, int Channels, float * range_table_f, float inv_alpha_f, float * down_Color_Buffer, float * down_Factor_Buffer, float * up_Color_Buffer, float * up_Factor_Buffer)
{

	int Stride = Width * Channels;
	const unsigned char* src_color_first_hor = Output; // result of horizontal pass filter 
	const unsigned char* src_down_color = Input;
	float* down_color = down_Color_Buffer;
	float* down_factor = down_Factor_Buffer;

	const unsigned char* src_down_prev = src_down_color;
	const float* down_prev_color = down_color;
	const float* down_prev_factor = down_factor;


	int last_index = Stride * Height - 1;
	const unsigned char* src_up_color = Input + last_index;
	const unsigned char* src_color_last_hor = Output + last_index; // result of horizontal pass filter
	float* up_color = up_Color_Buffer + last_index;
	float* up_factor = up_Factor_Buffer + (Width * Height - 1);

	const float* up_prev_color = up_color;
	const float* up_prev_factor = up_factor;

	// 1st line done separately because no previous line
	{
		//if y=0 
		for (int x = 0; x < Width; x++)
		{
			*down_factor++ = 1.f;
			*up_factor-- = 1.f;
			for (int c = 0; c < Channels; c++)
			{
				*down_color++ = *src_color_first_hor++;
				*up_color-- = *src_color_last_hor--;
			}
			src_down_color += Channels;
			src_up_color -= Channels;
		}
	}
	// handle other lines 
	for (int y = 1; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			// determine difference in pixel color between current and previous
			// calculation is different depending on number of channels
			int down_diff = getDiffFactor(src_down_color, src_down_prev);
			src_down_prev += Channels;
			src_down_color += Channels;
			src_up_color -= Channels;
			int up_diff = getDiffFactor(src_up_color, src_up_color + Stride);
			float down_alpha_f = range_table_f[down_diff];
			float up_alpha_f = range_table_f[up_diff];

			*down_factor++ = inv_alpha_f + down_alpha_f * (*down_prev_factor++);
			*up_factor-- = inv_alpha_f + up_alpha_f * (*up_prev_factor--);

			for (int c = 0; c < Channels; c++)
			{
				*down_color++ = inv_alpha_f * (*src_color_first_hor++) + down_alpha_f * (*down_prev_color++);
				*up_color-- = inv_alpha_f * (*src_color_last_hor--) + up_alpha_f * (*up_prev_color--);
			}
		}
	}

	// average result of vertical pass is written to output buffer
	{
		unsigned char *dst_color = Output;
		const float* downcolor = down_Color_Buffer;
		const float* downfactor = down_Factor_Buffer;
		const float* upcolor = up_Color_Buffer;
		const float* upfactor = up_Factor_Buffer;

		int width_height = Width * Height;
		for (int i = 0; i < width_height; i++)
		{
			// average color divided by average factor
			float factor = 1.f / ((*upfactor++) + (*downfactor++));
			for (int c = 0; c < Channels; c++)
			{
				*dst_color++ = (factor * ((*upcolor++) + (*downcolor++)));
			}
		}
	}
}

