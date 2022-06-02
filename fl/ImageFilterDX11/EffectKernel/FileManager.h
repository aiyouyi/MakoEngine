#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include <map>
#include <string>
#include <vector>
#include "../common.h"
#include "../Toolbox/xmlParser.h"
#include "../Toolbox/md5.h"
#include "../Toolbox/DXUtils/DXUtils.h"
#include "../Toolbox/DXUtils/DXTexturePool.h"
#include "ImageInfo.h"
#include "AnimInfo.h"
#include "VideoInfo.h"

/* usages
	1. SetSaveFolder
	2. AddImage/AddVideo/AddAnim in every part
	3. Finish
*/
class FileManager
{
public:
	static FileManager& Instance()
	{
		static FileManager instance;
		return instance;
	}
	~FileManager();

	// save-folder, where to save configuration file and resource files 
	int SetSaveFolder(std::string abs_dst_folder);

	// it will copy to the save-folder and rename the image file to {md5}+"_"+{filename}, return the renamed abs path
	ImageInfo AddImage(ImageInfo& info, std::string dstDir);
	ImageInfo AddImageRegion(ImageInfo& info, std::string dstDir, float left, float top, float right, float bottom);
	// it will generate a folder named {md5} in save-folder, then copy all anim files into the folder, return the renamed abs paths
	AnimInfo AddAnim(AnimInfo& info);
	AnimInfo AddAnim(AnimInfo& info, float left, float top, float right, float bottom);
	// it will copy to the save-folder and rename the video file to {md5}+"_"+{filename}, return the renamed abs path
	VideoInfo AddVideo(VideoInfo& info);

	//int Finish();
	//
	//int Reset();
	//// clean the save-folder
	//int CleanSaveFolder(std::string abs_folder);
	//// clean the save-folder except the "exceptions", since anim sub folder is complicated, so anim folder will not be in the exceptions
	//int CleanSaveFolderExcept(std::vector<const std::string>& exceptions);

private:
	std::string save_folder;
	//std::string temp_save_folder;
	//std::vector<std::string> used_files;

//public:
//	DX11Texture* GetTexFromFile(std::string abs_filename);
//	void RemoveTex(DX11Texture*);

//private:
//	DXTexturePool* m_texpool;

private:
	FileManager();
};

#endif