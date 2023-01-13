#ifndef COMMONFUNC_H
#define COMMONFUNC_H

#include "common.h"
#include "Toolbox/inc.h"
#ifndef _WIN64
#include "Toolbox/GL/openglutil.h"
#endif
#include "Toolbox/zip/unzip.h"

unsigned char *CreateImgFromZIP(HZIP hZip, const char *szImagePath,int &nWidth,int &nHeight);
unsigned char* LoadImageFromFile(const std::string& fileName, int& nWidth, int& nHeight);
void StringSplit(std::string str, const const char split, std::vector<std::string>& res);

std::shared_ptr<float> CreateHDRFromZIP(HZIP hZip, const char *szImagePath, int &nWidth, int &nHeight);
#ifndef _WIN64
GLuint CreateTexFromZIP(HZIP hZip, const char *szImagePath,int &nWidth,int &nHeight);
#endif

#if (defined _WIN64) || (defined _WIN32)
std::wstring DX11IMAGEFILTER_EXPORTS_CLASS char2TChar(const char* text);
#else
std::string DX11IMAGEFILTER_EXPORTS_CLASS char2TChar(const char* text);
#endif

#endif // COMMONFUNC_H
