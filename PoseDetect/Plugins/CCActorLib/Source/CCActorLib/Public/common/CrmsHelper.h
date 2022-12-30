// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Runtime/Core/Public/CoreMinimal.h>
#include "crms.h"
#include "voicedrive.h"
#include "win/dll.h"
/**
 * 
 */
class CCACTORLIB_API CrmsHelper
{
public:
	CrmsHelper();
	~CrmsHelper();

	bool init();
	float processPcm(char* buffer, unsigned int len);

private:
	decltype(hy_crms_param_set) * crms_param_set;
	decltype(hy_crms_init)* crms_init;
	decltype(hy_crms_process)* crms_process;
	decltype(hy_crms_close)* crms_close;

	decltype(hy_voicedrive_init)* _hy_voicedrive_init;
	decltype(hy_voicedrive_set_enable_param)* _hy_voicedrive_set_enable_param;
	decltype(hy_voicedrive_set_scale_param)* _hy_voicedrive_set_scale_param;
	decltype(hy_voicedrive_set_smothcoff_param)* _hy_voicedrive_set_smothcoff_param;
	decltype(hy_voicedrive_process)* _hy_voicedrive_process;
	decltype(hy_voicedrive_close)* _hy_voicedrive_close;

	win32::dll _dll;

	void * mCrmsInstance = nullptr; /*声音驱动算法库*/
	float mCurSoundVal = 0.0f;
};
