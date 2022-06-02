#include <iostream>
#include <algorithm>

#include <cstdint>
#include <numeric>
#include <math.h>
#include <io.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" 

#if defined(_MSC_VER)
#include <windows.h> 

#endif

std::string m_curFilePath;

void loadImage(const char* filename, unsigned char*& Output, int  &Width, int  &Height, int &Channels)
{
	Output = stbi_load(filename, &Width, &Height, &Channels, 0);
}
void saveImage(const char* filename, int  Width, int  Height, int Channels, unsigned char* Output, bool open = true)
{
	std::string saveFile = m_curFilePath;
	saveFile += filename;
	stbi_write_png(saveFile.c_str(), Width, Height, Channels, Output, 0);
}

void getCurrentFilePath(const char* filePath, std::string& curFilePath)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	curFilePath.clear();
	_splitpath_s(filePath, drive, dir, fname, ext);
	curFilePath += drive;
	curFilePath += dir;
	curFilePath += fname;
	curFilePath += "_";
}

//算法处理,这里以一个反色作为例子
void processImage(unsigned char* Input, unsigned char* Output, unsigned int  Width, unsigned int  Height, unsigned int Channels)
{
	int WidthStep = Width*Channels;
	if (Channels == 1)
	{
		for (unsigned int Y = 0; Y < Height; Y++)
		{
			unsigned char*     pOutput = Output + (Y * WidthStep);
			unsigned char*     pInput = Input + (Y * WidthStep);
			for (unsigned int X = 0; X < Width; X++)
			{
				pOutput[0] = 255 - pInput[0];
				pInput += Channels;
				pOutput += Channels;
			}
		}
	}
	else 	if (Channels == 3 || Channels == 4)
	{
		for (unsigned int Y = 0; Y < Height; Y++)
		{
			unsigned char*     pOutput = Output + (Y * WidthStep);
			unsigned char*     pInput = Input + (Y * WidthStep);
			for (unsigned int X = 0; X < Width; X++)
			{
				pOutput[0] = 255 - pInput[0];
				pOutput[1] = 255 - pInput[1];
				pOutput[2] = 255 - pInput[2];
				//通道数为4时，不处理A通道反色(pOutput[3] = 255 - pInput[3];)
				//下一个像素点
				pInput += Channels;
				pOutput += Channels;
			}
		}
	}

}
int main(int argc, char **argv) {

	std::cout << "Image Processing " << std::endl;
	if (argc < 2)
	{
		return 0;
	}

	std::string szfile = argv[1];

	if (_access(szfile.c_str(), 0) == -1)
	{
	}

	getCurrentFilePath(szfile.c_str(), m_curFilePath);

	int Width = 0;
	int Height = 0;
	int Channels = 0;
	unsigned char* inputImage = NULL;

	loadImage(szfile.c_str(), inputImage, Width, Height, Channels);

	if ((Channels != 0) && (Width != 0) && (Height != 0))
	{
		//分配与载入同等内存用于处理后输出结果
		unsigned char* outputImg = (unsigned char*)STBI_MALLOC(Width*Channels*Height*sizeof(unsigned char));
		if (inputImage) {
			memcpy(outputImg, inputImage, Width*Channels*Height);
		}
		else {

		}

		processImage(inputImage, outputImg, Width, Height, Channels);
		saveImage("_done.png", Width, Height, Channels, outputImg);

		if (outputImg)
		{
			STBI_FREE(outputImg);
			outputImg = NULL;
		}

		if (inputImage)
		{
			STBI_FREE(inputImage);
			inputImage = NULL;
		}
	}
	else
	{

	}

	getchar();
	return 0;
}
