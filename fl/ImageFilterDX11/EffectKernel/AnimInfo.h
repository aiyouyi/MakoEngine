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

#endif