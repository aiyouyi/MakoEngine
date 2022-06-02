#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include <map>
#include <string>
#include <vector>
#include "../common.h"
#include "../Toolbox/xmlParser.h"
#include "../Toolbox/md5.h"
#include "../Toolbox/DXUtils/DXUtils.h"
#include "../Toolbox/DXUtils/DXTexturePool.h"
#include "../Toolbox/zip/unzip.h"
#include "ImageInfo.h"
#include "AnimInfo.h"
#include "VideoInfo.h"


//1. SetSaveFolder
//2. AddImage/AddVideo/AddAnim
//3. Finish

class ResourceManager
{
public:
	static ResourceManager& Instance()
	{
		static ResourceManager instance;
		instance.Init();
		return instance;
	}
	~ResourceManager();

	int Init();

	// 初始化加载资源，返回id
	long long loadImage(ImageInfo& info);
	long long loadImageFromZip(ImageInfo& info, HZIP hZip);

	// 使用id获取实际texture指针
	Image* getImage(long long image_id);
	// 释放资源
	void freeImage(long long image_id);
	void saveImage(long long image_id);
	
	long long loadAnim(AnimInfo& info);
	long long loadAnimFromZip(AnimInfo& info, HZIP hZip);
	Anim*	getAnim(long long anim_id);
	void    SetAnimFps(long long anim_id,float fps);
	bool SetAnimCrop(long long anim_id);
	Image* getAnimFrame(long long anim_id, int frame_index);
	Image* getAnimFrame(long long anim_id, float time); // time from anim start, in ms
	void freeAnim(long long anim_id);

	//加载视频
	long long loadVideo(VideoInfo& info);
	long long loadVideoFromZip(VideoInfo& info, HZIP hZip);
	Image* getVideoFrame(long long video_id, double time);
	void freeVideo(long long video_id);

	//通用获取接口
	Image* getImageCommon(long long id, float time);

	void freeMaterial(long long id);

public:
	std::map<long long, Image*> m_images;
	std::map<long long, Anim*> m_anims;
	std::map<long long, Video*> m_videos;

	long long m_id=0;

public:
	DX11Texture* GetTexFromFile(std::string abs_filename);


private:
	DXTexturePool* m_texpool;

private:
	ResourceManager();
	long long generateID();
};

#endif