#pragma once
//#include "Windows/AllowWindowsPlatformTypes.h"
//#include <d3d11.h>
//#include "Windows/HideWindowsPlatformTypes.h"
#include <Runtime/Core/Public/CoreMinimal.h>
#include "core/event.h"

DECLARE_DELEGATE_OneParam(FParamDelegateSharedViewHandle, bool)

typedef void* HANDLE;

class CCACTORLIB_API DXHook
{
public:
	DXHook();
	~DXHook();

	static DXHook& getInstance();
	bool hookFunction(bool canUseSharedCopy);
	void doCpuImageCapture();

	HANDLE getSharedTexHandle();
	bool getReceiveSharedTexHandle();
	void setReceiveSharedTexHandle(bool bParam);
	void SetCpuHook(bool use);
	void SetMainWindow(void* hWnd);

public:
	FParamDelegateSharedViewHandle  CCActorlibSharedViewHandle;
	//core::event<void()> sigGpuImageCapture;
	//core::event<void(byte_t* color, core::vec2i viewport)> sigCpuImageCapture;
	//core::event<void(const std::string& name)> sigSendUseSharedTexCapture;

	//core::event<void(const HANDLE handle)> sigSetSharedViewHandle;
private:
	void onSigGpuImageCapture();
	void onCpuImageCapture(byte_t* color, core::vec2i viewport);
	void onSigSendUseSharedTexCapture(const std::string& name);

	void onsigSetSharedViewHandle(const HANDLE handle);

	bool bReceiveSharedTexHandle;
	HANDLE sharedTexHandle;
};