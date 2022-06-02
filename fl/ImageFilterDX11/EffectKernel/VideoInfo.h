#ifndef __VIDEO_INFO_H__
#define __VIDEO_INFO_H__
#include <vector>

#include "ImageInfo.h"
#include "EffectKernel/Video/VideoCaptureMapper.hpp"

class VideoInfo
{
public:
	int width;
	int height;
	float fps;
	double duration; //视频时长,单位 ms
	std::string dir;	// abs dir like "D:/dir0"
	std::string relative_filepath; // it may be "dir1/dir2/filename.png"
};

class Video
{
public:
	long long id;
	VideoInfo info;
	Image* current_frame = nullptr;
	int cur_frame_idx = 0;
	bool bRestart = false;
	std::shared_ptr<VideoCaptureMapper> video_mapper;
};

#endif