// Copyright Epic Games, Inc. All Rights Reserved.

#include "HumanPoseDetectBPLibrary.h"
#include "HumanPoseDetect.h"
#include "pose_service.hpp"


TArray<FString> UHumanPoseDetectBPLibrary::BoneNames = {
	"m_avg_root",
	"m_avg_Pelvis",
	"m_avg_L_Hip",
	"m_avg_R_Hip",
	"m_avg_Spine1",
	"m_avg_L_Knee",
	"m_avg_R_Knee",
	"m_avg_Spine2",
	"m_avg_L_Ankle",
	"m_avg_R_Ankle",
	"m_avg_Spine3",
	"m_avg_L_Foot",
	"m_avg_R_Foot",
	"m_avg_Neck",
	"m_avg_L_Collar",
	"m_avg_R_Collar",
	"m_avg_Head",
	"m_avg_L_Shoulder",
	"m_avg_R_Shoulder",
	"m_avg_L_Elbow",
	"m_avg_R_Elbow",
	"m_avg_L_Wrist",
	"m_avg_R_Wrist",
	"m_avg_lindex0",
	"m_avg_lindex1",
	"m_avg_lindex2",
	"m_avg_lmiddle0",
	"m_avg_lmiddle1",
	"m_avg_lmiddle2",
	"m_avg_lpinky0",
	"m_avg_lpinky1",
	"m_avg_lpinky2",
	"m_avg_lring0",
	"m_avg_lring1",
	"m_avg_lring2",
	"m_avg_lthumb0",
	"m_avg_lthumb1",
	"m_avg_lthumb2",
	"m_avg_rindex0",
	"m_avg_rindex1",
	"m_avg_rindex2",
	"m_avg_rmiddle0",
	"m_avg_rmiddle1",
	"m_avg_rmiddle2",
	"m_avg_rpinky0",
	"m_avg_rpinky1",
	"m_avg_rpinky2",
	"m_avg_rring0",
	"m_avg_rring1",
	"m_avg_rring2",
	"m_avg_rthumb0",
	"m_avg_rthumb1",
	"m_avg_rthumb2"
};


UHumanPoseDetectBPLibrary::UHumanPoseDetectBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UHumanPoseDetectBPLibrary::HumanPoseDetectSampleFunction(float Param)
{
	return -1;
}

void UHumanPoseDetectBPLibrary::InitPoseDetect()
{
	POSEInit();
}

void UHumanPoseDetectBPLibrary::DoPOSEProcess(unsigned char* ptr, int height, int width, int format, float* mesh_ptr)
{
	POSEProcess(ptr,  height,  width, SDKFormat(format), mesh_ptr);
}

