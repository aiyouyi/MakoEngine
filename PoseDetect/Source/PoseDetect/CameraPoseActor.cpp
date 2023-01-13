// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPoseActor.h"
#include<fstream>
#include<sstream>

// Sets default values
ACameraPoseActor::ACameraPoseActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VideoSourceComponent = CreateDefaultSubobject<UCCVideoSourceComponent>("VideoSourceComponent");
	bDetectHand = false;
}

// Called when the game starts or when spawned
void ACameraPoseActor::BeginPlay()
{
	Super::BeginPlay();
	CCPoseDetectRunnable = new UCCPoseDetectRunnable();
	UHumanPoseDetectBPLibrary::InitPoseDetect();
	if (bDetectHand)
	{
		UHumanPoseDetectBPLibrary::SetPoseDetectHand(bDetectHand);
	}
	CCPoseDetectRunnable->CameraPoseActor = this;
	RunnableThread = FRunnableThread::Create(CCPoseDetectRunnable, TEXT("CCPoseDetectRunnable"), 0);
}

void ACameraPoseActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHumanPoseDetectBPLibrary::DoPOSEClose();
	CCPoseDetectRunnable->bEnd = true;
	RunnableThread->WaitForCompletion();
	delete CCPoseDetectRunnable;
	CCPoseDetectRunnable = nullptr;
	Super::EndPlay(EndPlayReason);
		
}

// Called every frame
void ACameraPoseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CCPoseDetectRunnable->GetPoseData(NewestPoseData);
	for (size_t i = 0; i < UHumanPoseDetectBPLibrary::BoneNames.Num(); i++)
	{
		FQuat Quat = FQuat(0, 0, 0, 1);
		if (NewestPoseData[0]>0)
		{
			Quat.W = NewestPoseData[i*4+1];
			Quat.X = -NewestPoseData[i * 4 + 2];
			Quat.Y = NewestPoseData[i * 4 + 3];
			Quat.Z = -NewestPoseData[i * 4 + 4];
		}
		FQuat* OldValue = PoseData.Find(UHumanPoseDetectBPLibrary::BoneNames[i]);
		if (OldValue)
		{
			float TempAlpha = GetSlerpAlpha(*OldValue, Quat);
			Quat = FQuat::Slerp(*OldValue, Quat, TempAlpha);
		}
		PoseData.Add(TTuple<FString,FQuat>(UHumanPoseDetectBPLibrary::BoneNames[i], Quat));
	}	
	UpdateBoneRotation();
}

void ACameraPoseActor::UpdateBoneRotation()
{
	BoneRotations.Empty();
	for (auto BoneRel: BoneRelations)
	{
		if (PoseData.Find(BoneRel.StandardBoneName))
		{
			FQuat BaseQuat(BoneRel.OwnBoneBaseRoation);
			FQuat Result = BaseQuat.Inverse() * PoseData[BoneRel.StandardBoneName] * BaseQuat;
			BoneRotations.Add(TTuple<FString,FRotator>(BoneRel.OwnBoneName, Result.Rotator()));
		}
	}
}

float ACameraPoseActor::GetSlerpAlpha(FQuat Quat1, FQuat Quat2)
{
	float Delta = Quat1.AngularDistance(Quat2);	
	if (Delta<=0.08f)
	{
		return 0;
	}
	if (Delta <= 0.2f)
	{
		return 0.2f;
	}
	if (Delta >= 0.6f)
	{
		return 0.6f;
	}
	return Delta;
}

UCCPoseDetectRunnable::UCCPoseDetectRunnable()
{
	
}

UCCPoseDetectRunnable::~UCCPoseDetectRunnable()
{
	
}

bool UCCPoseDetectRunnable::Init()
{
	CriSecItem = new	FCriticalSection();
	return true;
}

uint32 UCCPoseDetectRunnable::Run()
{
	while (!bEnd)
	{
		FFrameData FrameData;
		if (CameraPoseActor->VideoSourceComponent->DeQueue(FrameData))
		{
			if (FrameData.Width <= 1920 && FrameData.Height <= 1080)
			{
				unsigned char* pWrite = &TempData[0];
				unsigned char* pRead = FrameData.DataBuffer.get();
				for (int i = 0; i < FrameData.Height* FrameData.Width; i++)
				{
					*(pWrite++) = *(pRead++);
					*(pWrite++) = *(pRead++);
					*(pWrite++) = *(pRead++);
					pRead++;
				}
				UHumanPoseDetectBPLibrary::DoPOSEProcess((unsigned char*)&TempData[0], FrameData.Height, FrameData.Width, 0, PoseData_Internaluse);
				{
					FScopeLock ScopeLock(CriSecItem);
					memcpy_s(NewestPoseData, 255 * sizeof(float), PoseData_Internaluse, 255 * sizeof(float));
				}				
			}
		}
		Sleep(1);
	}
	return 0;
}

void UCCPoseDetectRunnable::Stop()
{

}

void UCCPoseDetectRunnable::Exit()
{
	delete CriSecItem;
	CriSecItem = nullptr;
}

void UCCPoseDetectRunnable::GetPoseData(float* OutData)
{
	FScopeLock ScopeLock(CriSecItem);
	memcpy_s(OutData, 255 * sizeof(float), NewestPoseData, 255 * sizeof(float));
}
