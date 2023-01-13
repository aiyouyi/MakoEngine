#ifndef __ANIM_INFO_H__
#define __ANIM_INFO_H__
#include <vector>

class AnimInfo
{
public:
	float fps;
	std::string dir;
	std::vector<std::string> relative_filename_list;
	//int loop_times; // how many times it repeat
};

class Anim
{
public:
	long long id;
	AnimInfo info;
	std::vector<long long> images; // image id
};

class WebpAnimInfo
{
public:
	float fps;
	double duration; //每帧时长,单位 ms
	std::string webp_dir;	// abs dir like "D:/dir0"
	std::string webp_relative_filepath; // it may be "dir1/dir2/filename.png"
};

class WebpAnim
{
public:
	long long id;
	WebpAnimInfo info;
	std::vector<long long> images; // image id
};
#endif