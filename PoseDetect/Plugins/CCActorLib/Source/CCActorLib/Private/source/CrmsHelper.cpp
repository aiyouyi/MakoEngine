// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/Paths.h"
#include "common/CrmsHelper.h"

#define K_SoundDriver_SimpleRate 44100  /*音频采样率 44100 或 11025*/
#define K_SoundDriver_Channels 2   /*通道的数目,1单声道,2立体声*/
#define K_SoundDriver_BitsPerSample 16   /*8 & 16 & 32*/
#define K_SoundDriver_Scale 4.0f  /*放缩倍数，用于标记语音驱动的强弱*/
//#define K_SoundDriver_Bytenumber 1764
#define K_SoundDriver_Bytenumber 4096

CrmsHelper::CrmsHelper()
{
}

CrmsHelper::~CrmsHelper()
{
	if (crms_close && mCrmsInstance)
	{
		crms_close(mCrmsInstance);
		mCrmsInstance = NULL;
	}
}

bool CrmsHelper::init()
{
	std::string projDir = std::string(TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(FPaths::ProjectDir())));
	std::string dllPath = projDir + "innersdks/crms/hy_crms_lib.dll";
	if (auto err = _dll.load(dllPath, win32::dll_load_flag::altered_search_path))
		return false;

	crms_param_set = _dll.get<decltype(hy_crms_param_set) *>("hy_crms_param_set");
	if (!crms_param_set)
		return false;

	crms_init = _dll.get<decltype(hy_crms_init) *>("hy_crms_init");
	if (!crms_init)
		return false;

	crms_process = _dll.get<decltype(hy_crms_process) *>("hy_crms_process");
	if (!crms_process)
		return false;

	crms_close = _dll.get<decltype(hy_crms_close) *>("hy_crms_close");
	if (!crms_close)
		return false;

	if (!mCrmsInstance)
	{
		mCrmsInstance = crms_init(K_SoundDriver_Channels, K_SoundDriver_Bytenumber);
	}

	if (mCrmsInstance)
	{
		hy_crms_param parm;
		parm.scale = K_SoundDriver_Scale;
		crms_param_set(mCrmsInstance, &parm);
	}

	//std::string projDir = std::string(TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(FPaths::ProjectDir())));
	//std::string dllPath = projDir + "innersdks/crms/voicedrivelip_lib.dll";
	//if (auto err = _dll.load(dllPath, win32::dll_load_flag::altered_search_path))
	//	return false;

	//_hy_voicedrive_init = _dll.get<decltype(hy_voicedrive_init) *>("hy_voicedrive_init");
	//if (!_hy_voicedrive_init)
	//	return false;

	//_hy_voicedrive_set_enable_param = _dll.get<decltype(hy_voicedrive_set_enable_param) *>("hy_voicedrive_set_enable_param");
	//if (!_hy_voicedrive_set_enable_param)
	//	return false;

	//_hy_voicedrive_set_scale_param = _dll.get<decltype(hy_voicedrive_set_scale_param) *>("hy_voicedrive_set_scale_param");
	//if (!_hy_voicedrive_set_scale_param)
	//{
	//	return false;
	//}

	//_hy_voicedrive_set_smothcoff_param = _dll.get<decltype(hy_voicedrive_set_smothcoff_param) *>("hy_voicedrive_set_smothcoff_param");
	//if (!_hy_voicedrive_set_smothcoff_param)
	//{
	//	return false;
	//}

	//_hy_voicedrive_process = _dll.get<decltype(hy_voicedrive_process) *>("hy_voicedrive_process");
	//if (!_hy_voicedrive_process)
	//{
	//	return false;
	//}

	//_hy_voicedrive_close = _dll.get<decltype(hy_voicedrive_close) *>("hy_voicedrive_close");
	//if (!_hy_voicedrive_close)
	//{
	//	return false;
	//}

	//mCrmsInstance = _hy_voicedrive_init(K_SoundDriver_Channels, K_SoundDriver_Bytenumber);


	return true;
}

float CrmsHelper::processPcm(char* buffer, unsigned int len)
{
	float soundVal = 0;
	if (mCrmsInstance && crms_process)
	{
		//soundVal = _hy_voicedrive_process(mCrmsInstance, buffer, len);
		soundVal = crms_process(mCrmsInstance, buffer);
		if (soundVal < 0.2)
		{
			soundVal = 0;
			return soundVal;
		}
		if (std::abs(soundVal - mCurSoundVal) >= 0.1)
		{
			mCurSoundVal = soundVal;
		}
		else
		{
			soundVal = mCurSoundVal;
		}
	}
	return soundVal;
}
