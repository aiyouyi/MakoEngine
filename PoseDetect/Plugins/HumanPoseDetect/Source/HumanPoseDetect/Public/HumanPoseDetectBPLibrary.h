// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "HumanPoseDetectBPLibrary.generated.h"






UCLASS()
class HUMANPOSEDETECT_API UHumanPoseDetectBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Sample function", Keywords = "HumanPoseDetect sample test testing"), Category = "HumanPoseDetectTesting")
	static float HumanPoseDetectSampleFunction(float Param);


public:

	UFUNCTION(BlueprintPure, Category = "Utilities", meta = (Keywords = "unproject"))
		static bool GetInvViewProjectionMatrix(APlayerController const* Player, FLinearColor& M0, FLinearColor& M1, FLinearColor& M2, FLinearColor& M3,FVector2D& ViewRect);


	static void InitPoseDetect();
	static bool bInited;
	static bool bProcessing;

	static void DoPOSEProcess(unsigned char* ptr, int height, int width, int format, float* mesh_ptr);

	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (Keywords = "unproject"))
	static void SetPoseDetectHand(bool bDetectHand);

	static void DoPOSEClose();

	static TArray<FString> BoneNames;
};
