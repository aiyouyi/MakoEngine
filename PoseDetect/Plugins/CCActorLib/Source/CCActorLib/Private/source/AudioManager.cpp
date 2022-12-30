#include "common/AudioManager.h"
#include "core/logger.h"
#include "win/win32.h"
#include "common/bass.h"

#define BASS_ERROR(res) if(res == false) this->OutputBassError()

AudioManager::AudioManager()
{
	BASS_SetConfig(BASS_CONFIG_UNICODE,true);

	InitDeviceList();
	
}

AudioManager::~AudioManager()
{
	BASS_ERROR(BASS_Stop());
	BASS_ERROR(BASS_Free());
}

bool AudioManager::InitDevice(const wchar_t* deviceName, int& outDevice)
{
	int a, count = 0;
	BASS_DEVICEINFO info;
	TArray<BASS_DEVICEINFO> aaa;
	for (a = 1; BASS_GetDeviceInfo(a, &info); a++)
	{
		if (info.flags & BASS_DEVICE_ENABLED)
		{
			FString TargetDevice(deviceName);
			FString FullDevice(UTF8_TO_TCHAR(info.name));
			if (FullDevice.Contains(TargetDevice))
			{
				outDevice = a;
				BASS_ERROR(BASS_Init(a, 44100, BASS_DEVICE_DEFAULT, 0, nullptr));
				return true;
			}
		}
	}
	return false;
}

void AudioManager::InitDeviceList()
{
	BASS_DEVICEINFO di;
	for (DWORD i = 0; BASS_GetDeviceInfo(i, &di); i++) {
		if ((di.flags & BASS_DEVICE_ENABLED) ) {
			mDevList.push_back(UTF8_TO_TCHAR(di.name));
		}
	}
}


void AudioManager::Play(uint32_t handle, bool restart)
{
	BASS_ERROR(BASS_ChannelPlay(handle, restart));
}

void AudioManager::Stop(uint32_t handle)
{
	BASS_ERROR(BASS_ChannelStop(handle));
}

void AudioManager::SetVol(uint32_t handle,float vol)
{
	BASS_ERROR(BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, vol));
}

uint32_t AudioManager::CreateStreamFromFile(const wchar_t*file, uint64_t offset, uint64_t length)
{
	uint32_t handle = BASS_StreamCreateFile(false, file, offset, length, 0);
	if (!handle)
	{
		this->OutputBassError();
		return 0;
	}
	return handle;
}

void AudioManager::ChangeIsLoop(uint32_t handle, bool shouldLoop)
{
	if (shouldLoop)
	{
		BASS_ERROR(BASS_ChannelFlags(handle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP));
	}
	else
	{
		BASS_ERROR(BASS_ChannelFlags(handle, 0, BASS_SAMPLE_LOOP));
	}
}

void AudioManager::FreeStream(uint32_t handle)
{
	BASS_ERROR(BASS_StreamFree(handle));
}

bool AudioManager::IsStreamPlaying(uint32_t handle)
{
	return BASS_ChannelIsActive(handle) == BASS_ACTIVE_PLAYING;
}

void AudioManager::OutputBassError()
{
	static wchar_t* error_info[] = {
		L"其他不明问题",					// BASS_ERROR_UNKNOWN  -1						
		L"没有错误"						// BASS_OK				0
		L"内存错误",						// BASS_ERROR_MEM		1
		L"不能打开文件",					// BASS_ERROR_FILEOPEN	2
		L"没有找到驱动",					// BASS_ERROR_DRIVER	3
		L"采样缓冲区丢失",				// BASS_ERROR_BUFLOST	4
		L"无效句柄",						// BASS_ERROR_HANDLE	5
		L"不支持的采样格式",				// BASS_ERROR_FORMAT	6
		L"无效位置",						// BASS_ERROR_POSITION	7
		L"BASS_Init 函数没有成功调用",	// BASS_ERROR_INIT		8
		L"BASS_Start 函数没有成功调用",	// BASS_ERROR_START		9
		L"不支持的 SSL/HTTPS",			// BASS_ERROR_SSL		10
		L"已经初始化/暂停",				// BASS_ERROR_ALREADY	14
		L"不能获取空闲的 channel",		// BASS_ERROR_NOCHAN	18
		L"非法指定类型",					// BASS_ERROR_ILLTYPE	19
		L"非法指定参数",					// BASS_ERROR_ILLPARAM	20
		L"不支持 3D",						// BASS_ERROR_NO3D		21
		L"不支持 EAX",					// BASS_ERROR_NOEAX		22
		L"非法设备",						// BASS_ERROR_DEVICE	23
		L"不能播放",						// BASS_ERROR_NOPLAY	24
		L"非法采样频率",					// BASS_ERROR_FREQ		25
		L"不是一个文件流",				// BASS_ERROR_NOTFILE	27
		L"没有可用的声音硬件",				// BASS_ERROR_NOHW		29
		L"MOD 没有序列数据",				// BASS_ERROR_EMPTY		31
		L"不能打开网络连接",				// BASS_ERROR_NONET		32
		L"不能创建文件",					// BASS_ERROR_CREATE	33
		L"无效音效",						// BASS_ERROR_NOFX		34
		L"请求数据不可用",				// BASS_ERROR_NOTAVAIL	37
		L"channel 不是解码通道",			// BASS_ERROR_DECODE	38
		L"BASS_ERROR_DX",				// BASS_ERROR_DX		39
		L"连接超时",						// BASS_ERROR_TIMEOUT	40
		L"不支持文件格式",				// BASS_ERROR_FILEFORM	41
		L"BASS_ERROR_SPEAKER",		    // BASS_ERROR_SPEAKER	42
		L"无效 BASS 版本",				// BASS_ERROR_VERSION	43
		L"不支持的编解码器",				// BASS_ERROR_CODEC		44
		L"到达文件尾",				    // BASS_ERROR_ENDED		45
		L"设备繁忙"						// BASS_ERROR_BUSY		46
	};

	int res = BASS_ErrorGetCode();
	std::wstring error = error_info[res + 1];
	::OutputDebugStringW(error.c_str());
}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif