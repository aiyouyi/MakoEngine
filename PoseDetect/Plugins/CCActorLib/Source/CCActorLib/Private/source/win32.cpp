#include "CoreMinimal.h"
#include "win/win32.h"
#include "core/strings_utility.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace win32
{
    struct save_last_error
    {
        save_last_error() : err(GetLastError()) {}
        ~save_last_error() { SetLastError(err); }
        uint32_t err = 0;
    };

	std::string CCexecCmd(const char* cmd)
	{
		// 创建匿名管道
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };
		HANDLE hRead, hWrite;
		if (!CreatePipe(&hRead, &hWrite, &sa, 0))
		{
			return (" ");
		}

		// 设置命令行进程启动信息(以隐藏方式启动命令并定位其输出到hWrite
		STARTUPINFOA si = { sizeof(STARTUPINFO) };
		GetStartupInfoA(&si);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdError = hWrite;
		si.hStdOutput = hWrite;

		// 启动命令行
		PROCESS_INFORMATION pi;
		if (!CreateProcessA(NULL, (char*)cmd, NULL, NULL, true, NULL, NULL, NULL, &si, &pi))
		{
			return ("Cannot create process");
		}
		// 立即关闭hWrite

		CloseHandle(hWrite);

		// 读取命令行返回值
		std::string strRet;
		char buff[1024] = { 0 };
		DWORD dwRead = 0;
		strRet = buff;
		while (ReadFile(hRead, buff, 1024, &dwRead, NULL))
		{
			strRet += buff;
		}
		CloseHandle(hRead);
		return strRet;
	}

	core::error_e winerr()
    {
        return winerr_val(GetLastError());
    }

    core::error_e winerr_val(uint32_t err)
    {
        switch(err & 0xffff)
        {
        case ERROR_SUCCESS:
            return core::error_ok;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return core::error_not_found;
        case ERROR_NOT_ENOUGH_MEMORY:
            return core::error_outofmemory;
        case ERROR_ACCESS_DENIED:
        case ERROR_INVALID_ACCESS:
            return core::error_access;
        case ERROR_ALREADY_EXISTS:
            return core::error_exists;
        case ERROR_IO_PENDING:
            return core::error_pendding;
        case ERROR_BROKEN_PIPE:
            return core::error_broken;
        default:
            return core::error_generic;
        }
    }

    std::string winerr_str()
    {
        return winerr_str(GetLastError());
    }

    std::string winerr_str(uint32_t err)
    {
        save_last_error sle;
        wchar_t buffer[512] = {};
        int32_t nchars0 = swprintf_s(buffer, 512, L"%u: ", err);
        int nchars = ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err & 0xffff, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer + nchars0, 512 - nchars0, NULL);
		if(nchars > 2)
		{
			return core::ucs2_u8(buffer, nchars0 + nchars - 2);
		}
		else
		{
            wcscpy_s(buffer + nchars0, 512 - nchars0, L"unknown error");
            return core::ucs2_u8(buffer, nchars0 + wcslen(L"unknown error"));
		}
    }

    int runLoop()
    {
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            DWORD dwWait = MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
            //switch (dwWait)
            //{
            //default:
            //    break;
            //}

            while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                    break;
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        return (int)msg.wParam;
    }

    std::string GUID2String(const GUID & guid)
    {
        std::array<char, 40> output;
        snprintf(output.data(), output.size(), "{%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            (uint32_t)guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6],
            guid.Data4[7]);
        return std::string(output.data());
    }

    std::string GUID2String(std::array<uint8_t, 16> data)
    {
        const GUID & guid = *(const GUID *)data.data();
        std::array<char, 40> output;
        snprintf(output.data(), output.size(), "{%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            (uint32_t)guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6],
            guid.Data4[7]);
        return std::string(output.data());
    }

    std::tuple<std::shared_ptr<byte_t>, int32_t> load_res(uint32_t id, res_e type)
    {
        return load_res((const char *)id, type);
    }

    std::tuple<std::shared_ptr<byte_t>, int32_t> load_res(const char * res, res_e type)
    {
        static const char RES_TYPES[res_count][16] = { "FILE" };

        int index = std::clamp((int)type, 0, res_count - 1);
        HRSRC hRes = FindResourceA((HMODULE)&__ImageBase, res, RES_TYPES[index]);
        if (hRes)
        {
            HGLOBAL hData = LoadResource((HMODULE)&__ImageBase, hRes);
            if (hData)
            {
                byte_t * ptr = (byte_t *)LockResource(hData);
                uint32_t nbRes = SizeofResource((HMODULE)&__ImageBase, hRes);

                std::shared_ptr<byte_t> data(ptr, [hData](byte_t * ptr)
                {
                    UnlockResource(hData);
                    FreeResource(hData);
                });
                return { data, (int32_t)nbRes };
            }
        }
        return {};
    }

    winversion_t version()
    {
        static winversion_t version = {};
        if (version.major == 0)
        {
            typedef LONG(WINAPI * RtlGetVersionFuncT)(RTL_OSVERSIONINFOEXW *);
            typedef void (WINAPI * NtVersionNumbersFuncT)(DWORD *, DWORD *, DWORD *);

            RTL_OSVERSIONINFOEXW rtlOSVersion = { sizeof(RTL_OSVERSIONINFOEXW) };
            RtlGetVersionFuncT pfnRtlGetVersion = (RtlGetVersionFuncT)(::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));
            if(pfnRtlGetVersion)
            {
                pfnRtlGetVersion(&rtlOSVersion);
            }
            
            if (rtlOSVersion.dwMajorVersion > 5)
            {
                version.major = rtlOSVersion.dwMajorVersion;
                version.minor = rtlOSVersion.dwMinorVersion;
                version.pack = rtlOSVersion.wServicePackMajor;
                version.build = rtlOSVersion.dwBuildNumber;
            }
            else
            {
                NtVersionNumbersFuncT pfnRtlGetNtVersionNumbers = (NtVersionNumbersFuncT)(::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "RtlGetNtVersionNumbers"));
                if (pfnRtlGetNtVersionNumbers)
                {
                    DWORD dwMajor = 0;
                    DWORD dwMinor = 0;
                    DWORD dwBuild = 0;
                    pfnRtlGetNtVersionNumbers(&dwMajor, &dwMinor, &dwBuild);
                    dwBuild &= 0xffff;
                    version.major = dwMajor;
                    version.minor = dwMinor;
                    version.pack = 0;
                    version.build = dwBuild;
                }
                else
                {
                    version.major = rtlOSVersion.dwMajorVersion;
                    version.minor = rtlOSVersion.dwMinorVersion;
                    version.pack = rtlOSVersion.dwMajorVersion;
                    version.build = rtlOSVersion.dwBuildNumber;
                }
            }
        }

        return version;
    }

    std::string version_str(const winversion_t & ver)
    {
        if (ver >= winversion_10)
            return "10";
        else if (ver >= winversion_8_1)
            return "8.1";
        if (ver >= winversion_8)
            return "8";
        if (ver >= winversion_7)
            return "7";
        if (ver >= winversion_vista)
            return "Vista";
        if (ver >= winversion_server_2003)
            return "Server 2003";
        if (ver >= winversion_xp_sp3)
            return "XP SP3";
        if (ver >= winversion_xp_sp2)
            return "XP SP2";
        if (ver >= winversion_xp_sp1)
            return "XP SP1";
        if (ver >= winversion_xp)
            return "XP";
        if (ver >= winversion_2000)
            return "2000";
        return "Unknown";
    }

	bool create_process(const std::wstring& cmdLine)
	{
		PROCESS_INFORMATION pi = { 0 };
		wchar_t* cmd_line_w = NULL;
		STARTUPINFOW si = { 0 };
        si.cb = sizeof(si);
        bool success = false;

		if (cmdLine.length()) {
			success = !!CreateProcessW(NULL, (LPWSTR)cmdLine.c_str(), NULL, NULL, true,
				CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

			if (success) {
                WaitForSingleObject(pi.hProcess,2*1000);
                CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}
        return success;
	}

}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif