// Created by wangpeimin@corp.netease.com

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Windows\AllowWindowsPlatformTypes.h"
#include <memory>
#include "Runtime/Core/Public/HAL/Runnable.h"
#include <mutex>
#include "core/event.h"
#include "Windows\HideWindowsPlatformTypes.h"
#include "CCVideoSourceComponent.generated.h"

class CameraSharedData
{
public:
	char DataContent[10 + 1920 * 1080 * 4];
	void CopyBuffer(char* RawBuffer, int Width, int Height);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecordCameraEnd, bool, End);

USTRUCT()
struct FFrameData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	int32 Width;

	UPROPERTY()
	int32 Height;

	UPROPERTY()
	int32 FrameIndex;

	double ReceiveTimeMillSec;

	std::shared_ptr<uint8_t> DataBuffer;

};


class UTexture2D;
/**
 *
 */

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCVIDEOSOURCE_API UCCVideoSourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCCVideoSourceComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
private:

	std::shared_ptr<class CaptureVideo> pCaptureVideo;
	void InnerReceiveData(int Result, unsigned int Width, unsigned int Height, char* Data, const char* ErrorStr, bool fromlocalCamera);
	static void FlipX(int* Data, int32 Width, int32 Height);
	static void FlipY(int* Data, int32 Width, int32 Height);
	static void FlipXY(int* Data, int32 Width, int32 Height);

	TQueue<FFrameData> BufferData;
	int32 BufferSize = 0;

public:	
	int32 GetSize();
	bool EnQueue(FFrameData& InData);
	bool DeQueue(FFrameData& OutData);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CameraWidth;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CameraHeight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CameraFPS;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString CameraFOURCC;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bFlipX = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bFlipY = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString CameraName = "";

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* CameraPicture = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUpdateToCameraPicture = false;
	void UpdateCameraPicture(const FFrameData& FrameData);

	UFUNCTION(BlueprintCallable, Category = "UCCVideoSourceComponent")
	int32 OpenCamera();

	UFUNCTION(BlueprintCallable, Category = "UCCVideoSourceComponent")
	int32 GetBackgroundFPS();

	UFUNCTION(BlueprintCallable, Category = "UCCVideoSourceComponent")
		void OpenCameraPropertyWnd();

	UFUNCTION(BlueprintCallable, Category = "UCCVideoSourceComponent")
		void InitSetCameraProperty();

	UFUNCTION(BlueprintCallable, Category = "UCCVideoSourceComponent")
		void GetetCameraProperty();


	void OpenSelectCameraName(FString& CameranameParam);

	void StopCamera();
	void UpdateSharedCameraData(char* DataBuffer,int Width,int Height);
};