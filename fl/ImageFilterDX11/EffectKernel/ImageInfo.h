#ifndef __IMAGE_INFO_H__
#define __IMAGE_INFO_H__

#include "../Toolbox/DXUtils/DX11Texture.h"
#include <vector>

class ImageInfo
{
public:
	int width;
	int height;
	int channel;
	std::string dir;	// abs dir like "D:/dir0"
	std::string relative_filepath; // it may be "dir1/dir2/filename.png"
};

class Image
{
public:
	long long id;
	ImageInfo info;
	DX11Texture* tex;
};

#endif