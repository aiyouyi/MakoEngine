#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include"commonFunc.h"
#include "CC3DEngine/Common/CC3DUtils.h"
//#include "stb_image_lib/stb_image.h"

#if _MSC_VER
    #include <Windows.h>
#else
    #include <sys/time.h>
#endif

unsigned char *CreateImgFromZIP(HZIP hZip, const char *szImagePath,int &nWidth,int &nHeight)
{
    int index;
    ZIPENTRY ze;

    if (FindZipItem(hZip, szImagePath, true, &index, &ze) == ZR_OK)
    {
        char *pDataBuffer = new char[ze.unc_size];
        ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
        unsigned char *pImageDataDest = NULL;
        if (res == ZR_OK)
        {
            int texW, texH, nChannel;
            unsigned char *pImageDataSrc = ccLoadImageFromBuffer((unsigned char *)pDataBuffer, ze.unc_size, &texW, &texH, &nChannel, 4);
            nWidth = texW;
            nHeight = texH;
            if (pImageDataSrc != NULL && texW*texH > 0)
            {
                pImageDataDest = pImageDataSrc;
                //if (nChannel == 3)
                //{
                //    pImageDataDest = new unsigned char[texW*texH * 4];
                //    unsigned char *pImageSourceTemp = pImageDataSrc;
                //    unsigned char *pImageDestTemp = pImageDataDest;
                //    for (int r = 0; r < texH; ++r)
                //    {
                //        for (int c = 0; c < texW; ++c)
                //        {
                //            pImageDestTemp[0] = pImageSourceTemp[0];
                //            pImageDestTemp[1] = pImageSourceTemp[1];
                //            pImageDestTemp[2] = pImageSourceTemp[2];
                //            pImageDestTemp[3] = 255;

                //            pImageSourceTemp += 3;
                //            pImageDestTemp += 4;
                //        }
                //    }

                //    delete[]pImageDataSrc;
                //}

            }
        }
        delete[]pDataBuffer;
        return pImageDataDest;
    }
    return NULL;
}

unsigned char* LoadImageFromFile(const std::string& fileName, int& nWidth, int& nHeight)
{
    int ch = 4;
	unsigned char* pImageDataSrc = ccLoadImage(fileName.c_str(), &nWidth, &nHeight, &ch, 0);
    if (pImageDataSrc != NULL && nWidth * nHeight > 0)
    {
        unsigned char* pImageDataDest = pImageDataSrc;
        //转换成4通道.
        if (ch == 3)
        {
            pImageDataDest = new unsigned char[nWidth * nHeight * 4];
            unsigned char* pImageSourceTemp = pImageDataSrc;
            unsigned char* pImageDestTemp = pImageDataDest;
            for (int r = 0; r < nHeight; ++r)
            {
                for (int c = 0; c < nWidth; ++c)
                {
                    pImageDestTemp[0] = pImageSourceTemp[0];
                    pImageDestTemp[1] = pImageSourceTemp[1];
                    pImageDestTemp[2] = pImageSourceTemp[2];
                    pImageDestTemp[3] = 255;

                    pImageSourceTemp += 3;
                    pImageDestTemp += 4;
                }
            }

            delete[]pImageDataSrc;
        }
        return pImageDataDest;
    }
    return nullptr;
}

void StringSplit(std::string str, const char split, std::vector<std::string>& res)
{
    std::istringstream iss(str);	// 输入流
	std::string token;			// 接收缓冲区
	while (getline(iss, token, split))	// 以split为分隔符
	{
		res.push_back(token);
	}
}

std::shared_ptr<float> CreateHDRFromZIP(HZIP hZip, const char *szImagePath, int &nWidth, int &nHeight)
{
	int index;
	ZIPENTRY ze;

	if (FindZipItem(hZip, szImagePath, true, &index, &ze) == ZR_OK)
	{
		char *pDataBuffer = new char[ze.unc_size];
		ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
		float *pImageDataDest = NULL;
		if (res == ZR_OK)
		{
			int width, height, nrComponents;
			pImageDataDest = ccLoadImagefFromBuffer((unsigned char *)pDataBuffer, ze.unc_size, &width, &height, &nrComponents, 0);

			nWidth = width;
			nHeight = height;
		}
		delete[]pDataBuffer;
        return std::shared_ptr<float>(pImageDataDest, [](float* p) {free(p); });
	}
	return nullptr;
}
#ifndef _WIN64
GLuint CreateTexFromZIP(HZIP hZip, const char *szImagePath,int &nWidth,int &nHeight)
{
    unsigned char*pData = CreateImgFromZIP(hZip,szImagePath,nWidth,nHeight);
    if(pData!=NULL)
    {
        GLuint Tex ;
        OpenGLUtil::createToTexture(Tex,nWidth,nHeight,pData);
        delete []pData;
        return Tex;
    }
    return 0;

}
#endif


#if (defined _WIN64) || (defined _WIN32)
std::wstring DX11IMAGEFILTER_EXPORTS_CLASS char2TChar(const char* text)
{
	int length = (int32_t)std::strlen(text);
	int32_t nchars = MultiByteToWideChar(CP_UTF8, 0, text, length, NULL, 0);
	wchar_t* u16 = new wchar_t[nchars];
	int32_t nchars2 = MultiByteToWideChar(CP_UTF8, 0, text, length, u16, nchars);
	assert(nchars2 == nchars);
	std::wstring ucs2(u16, nchars);
	delete[] u16;
	return ucs2;
}
#else
std::string DX11IMAGEFILTER_EXPORTS_CLASS char2TChar(const char* text)
{
	std::string str = text;
	return str;
}
#endif

