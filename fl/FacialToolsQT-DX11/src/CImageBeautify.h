#pragma once

#define CC_RED 0
#define CC_GREEN 1
#define CC_BLUE 2
#define CC_ALPHA 3

class CImageBeautify
{
public:
	CImageBeautify();
	~CImageBeautify();
	void FilterLUT(unsigned char* pImage, unsigned char* pRefMap, int nWidth, int nHeight);

	void SmoothImage(unsigned char* pImage, int nWidth, int nHeight,float sigma_s, float sigma_r, int num_iterations);

	void  SmoothImage2(unsigned char* Input, int Width, int Height, float sigmaSpatial, float sigmaRange);

private:

	static void RFBoxFilter_Horizontal(unsigned char* imgData, int width, int height, int stride, int* xform_domain_position, unsigned char* Table);
	static void RFBoxFilter_Vertical(unsigned char* imgData, int width, int height, int stride, int* yform_domain_position, unsigned char* Table);


	int   getDiffFactor(const unsigned char* color1, const unsigned char* color2);
	void  CRB_HorizontalFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float * range_table_f, float inv_alpha_f, float* left_Color_Buffer, float* left_Factor_Buffer, float* right_Color_Buffer, float* right_Factor_Buffer);

	void  CRB_VerticalFilter(unsigned char* Input, unsigned char* Output, int Width, int Height, int Channels, float * range_table_f, float inv_alpha_f, float* down_Color_Buffer, float* down_Factor_Buffer, float* up_Color_Buffer, float* up_Factor_Buffer);

	int m_nWidth;
	int m_nHeight;
	float* m_leftColorBuffer ;
	float* m_leftFactorBuffer ;
	float* m_rightColorBuffer ;
	float* m_rightFactorBuffer ;
	unsigned char *m_Output;
};

