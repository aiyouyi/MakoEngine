#pragma once

#include "CoreMinimal.h"
#include "core/inc.h"
#include "core/path.h"

namespace core
{
    core::filesystem::path  temp_path();
    core::filesystem::path  current_path();
    bool current_path(core::filesystem::path path);
    core::filesystem::path  appdata_path();
	std::wstring getwide_appdata_path(const std::wstring& subPath = L"");
    core::filesystem::path  process_path();
    core::filesystem::path  process_directory();
    core::filesystem::path  module_directory();
    std::string  process_name();
    CCACTORLIB_API uint32_t  thread_id();
    CCACTORLIB_API uint32_t  process_id();
    bool  isFileExist(core::filesystem::path path);

    void  thread_set_name(int thread_id, const char * name);
    enum thread_priority
    {
        thread_priority_idle,
        thread_priority_lowest,
        thread_priority_low,
        thread_priority_normal,
        thread_priority_high,
        thread_priority_highest,
        thread_priority_realtime,
    };
    void  thread_set_priority(thread_priority priority);
	bool saveAsBMP(const char* pData, uint32_t nLen, int w, int h, int bitCount, const wchar_t* pszFileName);
}
FString  CCACTORLIB_API CC_GetRecordCameraPath();

FString  CCACTORLIB_API CC_GetCCPath();