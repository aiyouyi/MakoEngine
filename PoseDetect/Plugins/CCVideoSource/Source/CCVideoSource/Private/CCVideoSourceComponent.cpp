// Created by wangpeimin@corp.netease.com


#include "CCVideoSourceComponent.h"
#include <functional>
#include "cameracommfunc.h"
#include <functional>
#include "CaptureVideo.h"
#include "win/win32.h"
#include "Engine/Texture2d.h"
#include "core/strings_utility.h"
#include "core/timer.h"
#include "core/logger.h"
#include "HAL/ThreadingBase.h"
#include "core/system.h"
#include "RHI/Public/RHI.h"
#include "HAL/PlatformFilemanager.h"
#include "Components/ActorComponent.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "win/shared_memory.h"


using namespace std;



void UCCVideoSourceComponent::InnerReceiveData(int Result, unsigned int Width, unsigned int Height, char* Data, const char* ErrorStr, bool fromlocalCamera)
{	
	if (1 == Result)
	{
		if (bFlipX && bFlipY)
		{
			FlipXY((int*)Data, Width, Height);
		}
		else if (bFlipX)
		{
			FlipX((int*)Data, Width, Height);
		}
		else if(bFlipY)
		{
			FlipY((int*)Data, Width, Height);
		}

		if (CameraWidth != Width || CameraHeight != Height)
		{
			CameraWidth = Width;
			CameraHeight = Height;
		}
		UpdateSharedCameraData(Data,Width,Height);
		FFrameData FrameData;
		FrameData.DataBuffer = std::shared_ptr<uint8_t>((uint8_t*)Data, [fromlocalCamera](uint8_t* p) {
			if (fromlocalCamera)
			{
				free(p);
			}
		});
		static int32 StaticFrameIndex = 0;
		FrameData.Width = Width;
		FrameData.Height = Height;
		FrameData.FrameIndex = StaticFrameIndex++;
		FrameData.ReceiveTimeMillSec = FDateTime::Now().GetTimeOfDay().GetTotalMilliseconds();
		FGraphEventRef TaskRef = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
			{

				UpdateCameraPicture(FrameData);
			}, TStatId(), nullptr, ENamedThreads::GameThread);
		EnQueue(FrameData);
	}
}

void UCCVideoSourceComponent::FlipX(int* Data, int32 Width, int32 Height)
{
	int Temp;
	int* A = Data;
	int* B = Data + Width;
	for (int i = 0; i < Height; ++i)
	{
		for (int j = 0; j < Width / 2; j++)
		{
			Temp = *A;
			*A++ = *B;
			*B-- = Temp;
		}
		A += (Width / 2);
		B += (Width + Width / 2);
	}
}

void UCCVideoSourceComponent::FlipY(int* Data, int32 Width, int32 Height)
{
	int* A = Data;
	int* B = &Data[Width * Height - Width];
	int* Temps =new int[Width];
	for (int i = 0; i < Height/2; ++i)
	{
		FMemory::Memcpy(Temps, A, Width * 4);
		FMemory::Memcpy(A, B, Width * 4);
		FMemory::Memcpy(B, Temps, Width * 4);
		A += Width;
		B -= Width;
	}
	delete[] Temps;
}

void UCCVideoSourceComponent::FlipXY(int* Data, int32 Width, int32 Height)
{
	int Temp;
	int* A = Data;
	int* B = &Data[Width * Height];
	for (int i = 0; i < Height/2; ++i)
	{
		for (int j = 0; j < Width; j++)
		{
			Temp = *A;
			*A++ = *B;
			*B-- = Temp;
		}
	}
}

// Sets default values for this component's properties
UCCVideoSourceComponent::UCCVideoSourceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// ...
	CameraWidth = 1920;
	CameraHeight = 1080;
	CameraFPS = 60;
	CameraFOURCC = "MJPG";		
}


// Called when the game starts
void UCCVideoSourceComponent::BeginPlay()
{
	Super::BeginPlay();
	FString ParamName = "";
	if (FParse::Value(FCommandLine::Get(), TEXT("-CameraName="), ParamName))
	{
		CameraName = ParamName;
	}
	// ...	
}

void UCCVideoSourceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCamera();
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UCCVideoSourceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (CameraPicture)
	{
		CameraPicture->UpdateResource();
	}	
	// ...
}


int32 UCCVideoSourceComponent::GetSize()
{
	return FPlatformAtomics::AtomicRead((int32 volatile*)&BufferSize);
}

bool UCCVideoSourceComponent::EnQueue(FFrameData& InData)
{
	const int32 OriginalBufferSize = FPlatformAtomics::AtomicRead((int32 volatile*)&BufferSize);
	bool Ret = false;
	if (OriginalBufferSize<2)
	{
		Ret = BufferData.Enqueue(InData);
		if (Ret)
		{
			FPlatformAtomics::InterlockedIncrement(&BufferSize);
		}
	}	
	return Ret;
}

bool UCCVideoSourceComponent::DeQueue(FFrameData& OutData)
{
	bool Ret = BufferData.Dequeue(OutData);
	if (Ret)
	{
		FPlatformAtomics::InterlockedDecrement(&BufferSize);
	}
	return Ret;
}

void UCCVideoSourceComponent::UpdateCameraPicture(const FFrameData& FrameData)
{
	if (bUpdateToCameraPicture)
	{
		if (!CameraPicture|| CameraPicture->GetSizeX()!=FrameData.Width|| CameraPicture->GetSizeY()!=FrameData.Height)
		{
			CameraPicture = UTexture2D::CreateTransient(FrameData.Width, FrameData.Height, PF_B8G8R8A8);
		}
		uint8* TextureData = (uint8*)CameraPicture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, FrameData.DataBuffer.get(), FrameData.Width* FrameData.Height*4);
		CameraPicture->PlatformData->Mips[0].BulkData.Unlock();
		//CameraPicture->UpdateResource();
	}
}

int32 UCCVideoSourceComponent::OpenCamera()
{
	if (!pCaptureVideo)
	{
		pCaptureVideo = std::make_shared<CaptureVideo>();
		pCaptureVideo->ReceiveDataCallBack.bind(std::bind(&UCCVideoSourceComponent::InnerReceiveData, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6), this);
		pCaptureVideo->OpenDevice(*CameraName);
	}
	return 0;
}

int32 UCCVideoSourceComponent::GetBackgroundFPS()
{
	return 0;
}


void UCCVideoSourceComponent::OpenCameraPropertyWnd()
{
	if (pCaptureVideo)
	{
		pCaptureVideo->OpenPropertyWndBlocking();
	}
}

void UCCVideoSourceComponent::InitSetCameraProperty()
{
	if (pCaptureVideo)
	{
		CaptureVideo::CCamProperty CamProperty;
		CamProperty.Brightness = 200;
		pCaptureVideo->SetProperty(CamProperty);
	}
}

void UCCVideoSourceComponent::GetetCameraProperty()
{
	if (pCaptureVideo)
	{
		static CaptureVideo::CCamProperty CamProperty;
		CamProperty.Brightness = 200;
		pCaptureVideo->GetProperty(CamProperty);
	}
}



void UCCVideoSourceComponent::OpenSelectCameraName(FString& CameranameParam)
{
	if (CameranameParam.IsEmpty())
	{
		return;
	}
	if (pCaptureVideo)
	{
		pCaptureVideo->OpenDevice(*CameranameParam);
	}
}

void UCCVideoSourceComponent::StopCamera()
{
	if (pCaptureVideo)
	{
		pCaptureVideo->CloseInterface();
		pCaptureVideo.reset();
	}
}

void UCCVideoSourceComponent::UpdateSharedCameraData(char* DataBuffer, int Width, int Height)
{
	static win32::shared_memory<CameraSharedData> SharedCameraBuffer;
	static bool bTryOpen = false;
	static HANDLE H_Mutex = NULL;
	if (!bTryOpen)
	{
		bTryOpen = true;
		SharedCameraBuffer.create("CCVideoSourceComponentSharedMemoryOfCamera", sizeof(CameraSharedData),  win32::shared_mode::readwrite);
		H_Mutex = CreateMutex(NULL, 0, L"CCVideoSourceComponentSharedMemoryOfCameraMutex");
	}
	if (SharedCameraBuffer.valid())
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(H_Mutex, 100))
		{
			SharedCameraBuffer->CopyBuffer(DataBuffer, Width, Height);
			ReleaseMutex(H_Mutex);
		}
	}
}

void CameraSharedData::CopyBuffer(char* RawBuffer, int Width, int Height)
{
	if (Width <= 1920 && Height <= 1080)
	{
		*((int*)(&DataContent[0])) = Width;
		*((int*)(&DataContent[4])) = Height;
		memcpy_s(&DataContent[8], Width * Height * 4, RawBuffer, Width * Height * 4);
	}
}
