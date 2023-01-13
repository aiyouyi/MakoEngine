#include "fileSystem.h"

std::string g_temp_folder;

std::string getCWD() {
	char buff[1024];
	_getcwd(buff, 1024);
	return std::string(buff);
}

void getFileListFromFolder(const std::string path, std::vector<std::string> &list) {
	struct _finddata_t fileinfo;
	std::string file_path = path + "/*";
	long handle = _findfirst(file_path.c_str(), &fileinfo);
	if (handle == -1) return;
	do {
		std::string name(fileinfo.name);
		if (name == "." || name == "..")
			continue;
		if (fileinfo.attrib & _A_SUBDIR) {
			getFileListFromFolder(path + "/" + name, list);
		}
		else {
			list.push_back(path + "/" + name);
		};
	} while (!_findnext(handle, &fileinfo));
	_findclose(handle);
}

void removeFolder(const std::string path) {
	struct _finddata_t fileinfo;
	std::string file_path = path + "/*";
	long handle = _findfirst(file_path.c_str(), &fileinfo);
	if (handle == -1) return;
	do {
		std::string name(fileinfo.name);
		if (name == "." || name == "..")
			continue;
		std::string fullpath = path + "/" + name;
		if (fileinfo.attrib & _A_SUBDIR) {
			removeFolder(fullpath);
		}
		else {
			remove(fullpath.c_str());
		};
	} while (!_findnext(handle, &fileinfo));
	_findclose(handle);
	_rmdir(path.c_str());
}

void zipFolder(HZIP hz, const std::string& path) {
	int path_npos = path.length() + 1;
	int last_sep = max(path.find_last_of("\\"), path.find_last_of("/"));

	if (hz) {
		std::vector<std::string> file_list;
		getFileListFromFolder(path, file_list);

		for (auto file : file_list) {
			//std::cout << file.substr(path.length() + 1) << std::endl;
			if (ZR_OK != ZipAdd(hz, file.substr(path_npos).c_str(), file.c_str())) {
				return;
			}
		}
	}
}

void resetTempFolder() {
	if (!g_temp_folder.empty()) {
		removeFolder(g_temp_folder);
	}
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	std::string time = std::to_string(lt.wYear) + "-" + std::to_string(lt.wMonth) + "-" \
		+ std::to_string(lt.wDay) + "-" + std::to_string(lt.wHour) + "-" \
		+ std::to_string(lt.wMinute) + "-" + std::to_string(lt.wSecond);
	g_temp_folder = getCWD() + "/zip_temp_" + time;
	removeFolder(g_temp_folder);
	_mkdir(g_temp_folder.c_str());
}

std::string getTempFolder() {
	if (g_temp_folder.empty()) {
		resetTempFolder();
	}
	return g_temp_folder;
}