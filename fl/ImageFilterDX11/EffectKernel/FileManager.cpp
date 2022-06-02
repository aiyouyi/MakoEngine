#include "FileManager.h"
#include "io.h"
#include "../Toolbox/stb_image.h"
#include "../Toolbox/stb_image_write.h"
#ifdef _WIN32
#include "windows.h"
#endif

FileManager::FileManager()
{
}
FileManager::~FileManager()
{

}

// it will copy to the save-folder and rename the image file to {md5}+"_"+{filename}, return the renamed rel path(relative to save-folder)
// dstDir only use for AddAnim, so plz keep it empty
ImageInfo FileManager::AddImage(ImageInfo& external_info, std::string dstDir)
{
	std::string external_abs_filepath = external_info.dir + std::string("/") + external_info.relative_filepath;
	FILE* fp = fopen(external_abs_filepath.c_str(), "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		void* p = malloc(size);
		fread(p, 1, size, fp);
		fclose(fp);

		X265_NS::MD5 md5;
		md5.update((unsigned char*)p, size);
		unsigned char digest[16];
		md5.finalize(digest);
		std::string md5_val((char*)digest);

		// get old filename
		std::string filename;
		int index = external_info.relative_filepath.find_last_of("/");
		int slash_index = external_info.relative_filepath.find_last_of("\\");
		if (index > slash_index)
		{
			filename = external_info.relative_filepath.substr(index + 1, -1);
		}
		else
		{
			filename = external_info.relative_filepath.substr(slash_index + 1, -1);
		}

		// get new filename and filepath
		bool start_with_md5 = filename.compare(0, 16, md5_val);
		std::string new_filename = filename;
		std::string new_filepath;
		if (!start_with_md5)
		{
			new_filename = md5_val + std::string("_") + new_filename;
		}
		if (!dstDir.empty())
		{
			new_filepath = dstDir + "/" + new_filename;
		}

		std::string abs_temp_new_filename = save_folder + std::string("/") + new_filepath;
#ifdef _WIN32
		CopyFile(external_abs_filepath.c_str(), abs_temp_new_filename.c_str(), 1);
#else
		//TODO
#endif
		ImageInfo new_image_info;
		new_image_info.relative_filepath = new_filepath;
		return new_image_info;
	}
	else
	{
		ImageInfo new_image_info;
		return new_image_info;
	}
}

ImageInfo FileManager::AddImageRegion(ImageInfo& external_info, std::string dstDir, float left, float top, float right, float bottom)
{
	std::string external_abs_filepath = external_info.dir + std::string("/") + external_info.relative_filepath;
	FILE* fp = fopen(external_abs_filepath.c_str(), "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		void* p = malloc(size);
		fread(p, 1, size, fp);
		fclose(fp);

		X265_NS::MD5 md5;
		md5.update((unsigned char*)p, size);
		unsigned char digest[16];
		md5.finalize(digest);
		std::string md5_val((char*)digest);

		// get old filename
		std::string filename;
		int index = external_info.relative_filepath.find_last_of("/");
		int slash_index = external_info.relative_filepath.find_last_of("\\");
		if (index > slash_index)
		{
			filename = external_info.relative_filepath.substr(index + 1, -1);
		}
		else
		{
			filename = external_info.relative_filepath.substr(slash_index + 1, -1);
		}

		index = filename.find_last_of(".");
		std::string basename = filename.substr(0, index);
		std::string extname = filename.substr(index, -1);
		int image_type; // 0 for png, 1 for jpg
		if (extname == std::string(".png"))
		{
			image_type = 0;
		}
		else if (extname == std::string(".jpg"))
		{
			image_type = 1;
		}
		else
		{
			image_type = 0;
		}

		// get new filename and filepath
		bool start_with_md5 = filename.compare(0, 16, md5_val);
		std::string new_filename = filename;
		std::string new_filepath;
		if (!start_with_md5)
		{
			new_filename = md5_val + std::string("_") + new_filename;
		}
		if (!dstDir.empty())
		{
			new_filepath = dstDir + "/" + new_filename;
		}

		std::string abs_temp_new_filename = save_folder + std::string("/") + new_filepath;

		int w, h, channels;
		unsigned char* input_data = stbi_load(external_abs_filepath.c_str(), &w, &h, &channels, 0);
		if (input_data == NULL)
		{
			ImageInfo new_image_info;
			return new_image_info;
		}
		float width_percent = right - left;
		float height_percent = bottom - top;
		int abs_left = int(w * left);
		int abs_top = int(h * top);
		int new_w = (int)(w * width_percent);
		int new_h = (int)(h * height_percent);
		unsigned char* output_data = (unsigned char*)malloc(new_w * new_h * channels * sizeof(unsigned char));
		for (int j = 0; j < new_h; j++)
		{
			memcpy(output_data + new_w * channels * j, input_data + w*channels*(abs_top + j) + abs_left*channels, new_w*channels);
		}
		if (0 == image_type)
		{
			stbi_write_png(new_filepath.c_str(), new_w, new_h, channels, output_data, new_w*channels);
		}
		if (1 == image_type)
		{
			stbi_write_jpg(new_filepath.c_str(), new_w, new_h, channels, output_data, new_w*channels);
		}
		stbi_image_free(input_data);
		free(output_data);

		ImageInfo new_image_info;
		new_image_info.relative_filepath = new_filepath;
		return new_image_info;
	}
	else
	{
		ImageInfo new_image_info;
		return new_image_info;
	}
}

// it will generate a folder named {md5} in save-folder, then copy all anim files into the folder, return the renamed abs paths
AnimInfo FileManager::AddAnim(AnimInfo& info)
{
	// concat all md5 and then generate a new md5
	std::string dir = info.dir;

	std::vector<std::string> old_abs_file_paths;
	std::vector<std::string> new_filenames;
	for (int i = 0; i < info.relative_filename_list.size(); i++)
	{
		std::string relative_file_path = info.relative_filename_list[i];
		std::string abs_file_path = dir + "/" + info.relative_filename_list[i];

		// get filename
		std::string filename;
		int index = relative_file_path.find_last_of("/");
		int slash_index = relative_file_path.find_last_of("\\");
		if (index > slash_index)
		{
			filename = relative_file_path.substr(index + 1, -1);
		}
		else
		{
			filename = relative_file_path.substr(slash_index + 1, -1);
		}

		index = filename.find_last_of(".");
		std::string basename = filename.substr(0, index);
		std::string extname = filename.substr(index, -1);

		char newbasename[256];
		sprintf(newbasename, "%06d", i);
		std::string new_filename = std::string(newbasename) + extname;

		old_abs_file_paths.push_back(abs_file_path);
		new_filenames.push_back(new_filename);
	}
	std::string anim_folder;
	std::string sub_dir;
	for (int j = 0; j < 100; j++)
	{
		char sub_dir_name[256];
		sprintf(sub_dir_name, "%02d", j);
		sub_dir = sub_dir_name;
		// absolute anim folder {temp_save_folder}/{folder-md5}
		anim_folder = save_folder + "/" + sub_dir;
		// exist == 0 means it exists
		int exist = _access(anim_folder.c_str(), 0);
		if (0 != exist)
		{
			CreateDirectory(anim_folder.c_str(), NULL);
			break;
		}
		else
		{
			continue;
		}
	}

	for (auto& new_filename : new_filenames)
	{
		new_filename = sub_dir + std::string("/") + new_filename;
	}

	assert(old_abs_file_paths.size() == new_filenames.size());

	for (int i = 0; i < new_filenames.size(); i++)
	{
#ifdef _WIN32
		CopyFile(old_abs_file_paths[i].c_str(), (save_folder + std::string("/") + new_filenames[i]).c_str(), 1);
#else
		//TODO
#endif
	}
	AnimInfo anim_info;
	anim_info.dir = save_folder;
	anim_info.relative_filename_list = new_filenames;
	return anim_info;

}

AnimInfo FileManager::AddAnim(AnimInfo& info, float left, float top, float right, float bottom)
{
		// concat all md5 and then generate a new md5
	std::string dir = info.dir;

	std::vector<std::string> old_abs_file_paths;
	std::vector<std::string> new_filenames;
	for (int i = 0; i < info.relative_filename_list.size(); i++)
	{
		std::string relative_file_path = info.relative_filename_list[i];
		std::string abs_file_path = dir + "/" + info.relative_filename_list[i];

		// get filename
		std::string filename;
		int index = relative_file_path.find_last_of("/");
		int slash_index = relative_file_path.find_last_of("\\");
		if (index > slash_index)
		{
			filename = relative_file_path.substr(index + 1, -1);
		}
		else
		{
			filename = relative_file_path.substr(slash_index + 1, -1);
		}

		index = filename.find_last_of(".");
		std::string basename = filename.substr(0, index);
		std::string extname = filename.substr(index, -1);

		char newbasename[256];
		sprintf(newbasename, "%06d", i);
		std::string new_filename = std::string(newbasename) + extname;

		old_abs_file_paths.push_back(abs_file_path);
		new_filenames.push_back(new_filename);
	}
	std::string anim_folder;
	std::string sub_dir;
	for (int j = 0; j < 100; j++)
	{
		char sub_dir_name[256];
		sprintf(sub_dir_name, "%02d", j);
		sub_dir = sub_dir_name;
		// absolute anim folder {temp_save_folder}/{folder-md5}
		anim_folder = save_folder + "/" + sub_dir;
		// exist == 0 means it exists
		int exist = _access(anim_folder.c_str(), 0);
		if (0 != exist)
		{
			CreateDirectory(anim_folder.c_str(), NULL);
			break;
		}
		else
		{
			continue;
		}
	}

	for (auto& new_filename : new_filenames)
	{
		new_filename = sub_dir + std::string("/") + new_filename;
	}

	assert(old_abs_file_paths.size() == new_filenames.size());

	for (int i = 0; i < new_filenames.size(); i++)
	{
		int w, h, channels;
		unsigned char* input_data = stbi_load(old_abs_file_paths[i].c_str(), &w, &h, &channels, 0);
		if (input_data == NULL)
		{
			continue;
		}
		float ratio = float(w) / h;
		if (ratio == 930.0 / 1240)
		{
			int index = new_filenames[i].find_last_of(".");
			std::string basename = new_filenames[i].substr(0, index);
			std::string extname = new_filenames[i].substr(index, -1);
			int image_type; // 0 for png, 1 for jpg
			if (extname == std::string(".png"))
			{
				image_type = 0;
			}
			else if (extname == std::string(".jpg"))
			{
				image_type = 1;
			}
			else
			{
				image_type = 0;
			}
			float width_percent = right - left;
			float height_percent = bottom - top;
			int abs_left = int(w * left);
			int abs_top = int(h * top);
			int new_w = (int)(w * width_percent);
			int new_h = (int)(h * height_percent)+1;
			unsigned char* output_data = (unsigned char*)malloc(new_w * new_h * channels * sizeof(unsigned char));
			for (int j = 0; j < new_h; j++)
			{
				memcpy(output_data + new_w * channels * j, input_data + w * channels*(abs_top + j) + abs_left * channels, new_w*channels);
			}
			if (0 == image_type)
			{
				stbi_write_png((save_folder + std::string("/") + new_filenames[i]).c_str(), new_w, new_h, channels, output_data, new_w*channels);
			}
			if (1 == image_type)
			{
				stbi_write_jpg((save_folder + std::string("/") + new_filenames[i]).c_str(), new_w, new_h, channels, output_data, new_w*channels);
			}
			stbi_image_free(input_data);
			free(output_data);
		}
		else
		{
			stbi_image_free(input_data);

		#ifdef _WIN32
					CopyFile(old_abs_file_paths[i].c_str(), (save_folder + std::string("/") + new_filenames[i]).c_str(), 1);
		#else
					//TODO
		#endif

		}

	}
	AnimInfo anim_info;
	anim_info.dir = save_folder;
	anim_info.relative_filename_list = new_filenames;
	return anim_info;
}

// it will copy to the save-folder and rename the video file to {md5}+"_"+{filename}, return the renamed abs path
VideoInfo FileManager::AddVideo(VideoInfo& info)
{
	// TODO
	VideoInfo empty;
	return empty;
}

// clean the save-folder
//int FileManager::CleanSaveFolder(std::string abs_folder)
//{
//	return 1;
//}

// clean the save-folder except the "exceptions", since anim sub folder is complicated, so anim folder will not be in the exceptions
//int FileManager::CleanSaveFolderExcept(std::vector<const std::string>& exceptions)
//{
//	return 1;
//}

// save-folder, where to save configuration file and resource files 
int FileManager::SetSaveFolder(std::string abs_dst_folder)
{
	save_folder = abs_dst_folder;
	//temp_save_folder = abs_dst_folder + std::string("~");

	//SHFILEOPSTRUCT DirRemove;
	//DirRemove.fFlags = FOF_NOCONFIRMATION;
	//DirRemove.hNameMappings = NULL;
	//DirRemove.hwnd = NULL;
	//DirRemove.lpszProgressTitle = NULL;
	//DirRemove.pFrom = temp_save_folder.c_str();
	//DirRemove.pTo = NULL;
	//DirRemove.wFunc = FO_DELETE;
	//SHFileOperation(&DirRemove);

	//CreateDirectory(temp_save_folder.c_str(), NULL);
	return 1;
}

//int FileManager::Finish()
//{
//	rename(temp_save_folder.c_str(), save_folder.c_str());
//	return 1;
//}
//
//
//DX11Texture* FileManager::GetTexFromFile(std::string abs_filename)
//{
//	return NULL;
//}
