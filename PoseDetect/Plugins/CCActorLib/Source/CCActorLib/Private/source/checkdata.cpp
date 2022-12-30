
#include "checkdata.h"
#include "core/logger.h"
#include "pipe-windows.h"
#include  <Runtime/Core/Public/Misc/ConfigCacheIni.h>
#include "win/win32.h"

struct huyaSharedData
{
	BOOL _isCanUseDX11;
	BOOL _isCanUseDX10;
	BOOL _isCanUseDX9;
	BOOL _isCanUseNVAVCDecode;
	BOOL _isEnableSharedResource;

	huyaSharedData()
		:_isCanUseDX11(false)
		, _isCanUseDX10(false)
		, _isCanUseDX9(false)
		, _isCanUseNVAVCDecode(false)
		, _isEnableSharedResource(false)
	{

	}
};

static CCheckData* g_instance = NULL;

CCheckData::CCheckData()
{
}

CCheckData::~CCheckData()
{
	m_sharedData.close();
}

CCheckData* CCheckData::GetInstance()
{
	if (g_instance == NULL)
	{
		g_instance = new CCheckData();
	}
	return g_instance;
}

void CCheckData::Release()
{
	if (g_instance)
	{
		delete g_instance;
		g_instance  = NULL;
	}
}

void CCheckData::ServerInit()
{
	core::error_e eror = m_sharedData.create("kHuyaClientSharedData", sizeof(huyaSharedData), win32::shared_mode::readwrite);
	if (eror == core::error_exists)
	{
		m_sharedData.open("kHuyaClientSharedData", win32::shared_mode::readwrite);
		m_alreadyCheck = true;
	}
	else
	{
		m_sharedData->_isCanUseDX10 = false;
		m_sharedData->_isCanUseDX11 = false;
		m_sharedData->_isCanUseDX9 = false;
		m_sharedData->_isCanUseNVAVCDecode = false;
		m_sharedData->_isEnableSharedResource = false;
	}
}

void CCheckData::CreateCheckProcess(const wchar_t * cmd)
{
	std::wstring projDir = *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	projDir += L"innersdks/dxchecker";

	wchar_t cmdLine[MAX_PATH];
	wsprintfW(cmdLine, L"\"%s/hysandbox.exe\" %s ", projDir.c_str(), cmd);

	STARTUPINFO   si;
	::ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION   pi;

	if (::CreateProcessW(NULL, cmdLine,
		NULL, NULL, false, 0, NULL, NULL, &si, &pi))
	{
		::CloseHandle(pi.hThread);
	}
}

bool CCheckData::IsEnableSharedResource()
{
	if (!m_sharedData)
	{
		return false;
	}
	//return TRUE == m_sharedData->_isEnableSharedResource;
	return 1 == m_sharedData->_isEnableSharedResource;
}

void CCheckData::StartCheckSharedCopy()
{
	ServerInit();
	if (m_alreadyCheck)
	{
		return;
	}
	CreateCheckProcess(L"TEST_DX11_SHARED");
}

std::tuple<int64_t, int64_t, int64_t> CCheckData::GetDXOffset()
{
	std::string projDir = TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()));
	std::string offset_exe_path = projDir + "Content/ConfigINI/get-graphics-offsets64.exe";
	os_process_pipe_t *pp = os_process_pipe_create(offset_exe_path.c_str(), "r");
	char data[128] = {};
	std::string strResult;
	for (;;) {
		size_t len = os_process_pipe_read(pp, (uint8_t*)data, 128);
		if (!len)
			break;

		strResult.append(data, len);
	}

	FConfigFile iniFile;
	iniFile.CombineFromBuffer(UTF8_TO_TCHAR(strResult.c_str()));
	int64_t presentVal = 0;
	int64_t present1Val = 0;
	int64_t resizeVal = 0;

	FString tempVAL ;
	iniFile.GetString(L"dxgi", L"present", tempVAL);
	presentVal = FParse::HexNumber64(*tempVAL);
	
	iniFile.GetString(L"dxgi", L"present1", tempVAL);
	present1Val = FParse::HexNumber64(*tempVAL);

	iniFile.GetString(L"dxgi", L"resize", tempVAL);
	resizeVal = FParse::HexNumber64(*tempVAL);

	return std::make_tuple(presentVal, present1Val, resizeVal);
}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
	#include "Windows/HideWindowsPlatformTypes.h"
#endif