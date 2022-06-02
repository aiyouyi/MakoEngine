#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <direct.h>
#include <io.h>
#include <windows.h>

#include "xmlParser.h"
#include "zip\zip.h"
#include "zip\unzip.h"

extern std::string g_temp_folder;

std::string getCWD();
void getFileListFromFolder(const std::string path, std::vector<std::string> &list);
void removeFolder(const std::string path);
void zipFolder(HZIP hz, const std::string path);
void resetTempFolder();
std::string getTempFolder();