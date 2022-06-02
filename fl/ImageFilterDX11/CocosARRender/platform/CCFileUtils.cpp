/****************************************************************************
Copyright (c) 2010-2013 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "CCFileUtils.h"
#include <stack>
#include "base/CCData.h"
#include "base/ccMacros.h"
#include<fstream>
#include <sstream>

#if defined(_MSC_VER)
#include <Windows.h>
#endif

NS_CC_BEGIN

FileUtils *FileUtils::s_fileUtilsInst = NULL;

string FileUtils::getFileExtension(const string& filePath)
{
    std::string fileExtension;
    size_t pos = filePath.find_last_of('.');
    if (pos != std::string::npos)
    {
        fileExtension = filePath.substr(pos, filePath.length());

        std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
    }

    return fileExtension;
}

FileUtils* FileUtils::getInstance()
{
	if (s_fileUtilsInst == NULL)
	{
		s_fileUtilsInst = new FileUtils();
	}

	return s_fileUtilsInst;
}


void FileUtils::destroyInstance()
{
	if (s_fileUtilsInst != NULL)
	{
		delete s_fileUtilsInst;
		s_fileUtilsInst = NULL;
	}
}

string FileUtils::utf8ToGBK(const string &str)
{
#if defined(_MSC_VER)
	string outGBK = "";
	int n = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	WCHAR *str1 = new WCHAR[n];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_ACP, 0, str1, -1, NULL, 0, NULL, NULL);
	char *str2 = new char[n];
	WideCharToMultiByte(CP_ACP, 0, str1, -1, str2, n, NULL, NULL);
	outGBK = str2;
	delete[] str1;
	str1 = NULL;
	delete[] str2;
	str2 = NULL;
	return outGBK;
#else
	return str;
#endif
}

string FileUtils::getStringFromFile(const std::string& filename)
{
	std::ifstream t(filename);
	stringstream buffer;
	buffer << t.rdbuf();
	return std::string(buffer.str());

}

Data FileUtils::getDataFromFile(const std::string& filename)
{
	FILE *file = fopen(filename.c_str(), "rb");
	if (file != NULL)
	{
		fseek(file, 0, SEEK_END);
		long size = ftell(file);

		unsigned char* buffer = new unsigned char[size];
		fseek(file, 0, SEEK_SET);
		fread(buffer, size, 1, file);
		fclose(file);

		unsigned char *pData = buffer + 13240;
		char dataView[20];
		memcpy(dataView, pData, 20);

		Data res;
		res.fastSet((unsigned char *)buffer, size);

		return res;
	}

	Data res;
	res.fastSet(NULL, 0);
	return res;
}

NS_CC_END
