// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCVideoSourceComponent.h"
#include "HumanPoseDetectBPLibrary.h"
#include "CameraPoseActor.generated.h"

class UCCPoseDetectRunnable : public FRunnable
{
public:
	UCCPoseDetectRunnable();
	~UCCPoseDetectRunnable();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	bool bEnd = false;
	class ACameraPoseActor* CameraPoseActor = nullptr;
	float PoseData_Internaluse[255];
	unsigned char TempData[1920*1080*3];

	void GetPoseData(float* OutData);
private:
	FCriticalSection* CriSecItem=nullptr;
	float NewestPoseData[255];
};



USTRUCT(BlueprintType)
struct FBoneRelation
{
	GENERATED_USTRUCT_BODY()

	// 真实骨骼名字
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString OwnBoneName;

	// 对应标准模型的骨骼名字
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString StandardBoneName;

	// 真实骨骼的T-Pose 基础旋转
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator OwnBoneBaseRoation;
};

	
UCLASS()
class POSEDETECT_API ACameraPoseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACameraPoseActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Category = CameraPose, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCCVideoSourceComponent* VideoSourceComponent;	

	UCCPoseDetectRunnable* CCPoseDetectRunnable= nullptr;
	FRunnableThread* RunnableThread;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString,FQuat> PoseData;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FRotator> BoneRotations;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FBoneRelation> BoneRelations;

	void UpdateBoneRotation();
	float GetSlerpAlpha(FQuat Quat1, FQuat Quat2);

	float NewestPoseData[255];
};
