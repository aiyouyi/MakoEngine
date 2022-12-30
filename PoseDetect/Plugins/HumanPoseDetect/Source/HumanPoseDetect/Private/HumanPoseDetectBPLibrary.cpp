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

bool UHumanPoseDetectBPLibrary::GetInvViewProjectionMatrix(APlayerController const* Player, FLinearColor& M0, FLinearColor& M1, FLinearColor& M2, FLinearColor& M3, FVector2D& ViewRect)
{
	ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
	if (LP && LP->ViewportClient)
	{
		FSceneViewProjectionData ProjectionData;
		if (LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_FULL, /*out*/ ProjectionData))
		{
			ViewRect.X=ProjectionData.GetConstrainedViewRect().Width();
			ViewRect.Y = ProjectionData.GetConstrainedViewRect().Height();
			FMatrix const InvViewProjMatrix = ProjectionData.ComputeViewProjectionMatrix().InverseFast();
			M0.R = InvViewProjMatrix.M[0][0];
			M0.G = InvViewProjMatrix.M[1][0];
			M0.B = InvViewProjMatrix.M[2][0];
			M0.A = InvViewProjMatrix.M[3][0];

			M1.R = InvViewProjMatrix.M[0][1];
			M1.G = InvViewProjMatrix.M[1][1];
			M1.B = InvViewProjMatrix.M[2][1];
			M1.A = InvViewProjMatrix.M[3][1];

			M2.R = InvViewProjMatrix.M[0][2];
			M2.G = InvViewProjMatrix.M[1][2];
			M2.B = InvViewProjMatrix.M[2][2];
			M2.A = InvViewProjMatrix.M[3][2];

			M3.R = InvViewProjMatrix.M[0][3];
			M3.G = InvViewProjMatrix.M[1][3];
			M3.B = InvViewProjMatrix.M[2][3];
			M3.A = InvViewProjMatrix.M[3][3];
			return true;
		}
	}
	return false;
}

void UHumanPoseDetectBPLibrary::InitPoseDetect()
{
	POSEInit();
}

void UHumanPoseDetectBPLibrary::DoPOSEProcess(unsigned char* ptr, int height, int width, int format, float* mesh_ptr)
{
	POSEProcess(ptr,  height,  width, SDKFormat(format), mesh_ptr);
}

