#pragma once
#include "CoreMinimal.h"
#include "Templates/RefCounting.h"
#include "core/inc.h"
#include "core/vec2.h"
#include "funchook.h"
#include "win/frame_swapchain.h"
//#include "common/Bitmap.h"
#include "win/shared_memory.h"



typedef void* HANDLE;
struct ID3D11Device;
struct IDXGISwapChain;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
class AhuyavirtualactorGameModeBase;
struct D3D11_TEXTURE2D_DESC;

class  CDX11Hook
{
public:
	CDX11Hook();
	~CDX11Hook();

	static CDX11Hook& getInstance();

	bool hookFunction(bool canUseSharedCopy);
	bool initDX11(IDXGISwapChain *swap);

	ObsStudioHook& swapResizeBuffer()
	{
		return mSwapResizeBuffers;
	}

	ObsStudioHook& swapPresent()
	{
		return mSwapPresent;
	}

	ID3D11Device* currentDXDevice()
	{
		return mDXDevice.GetReference();
	}

	IDXGISwapChain* currentSwapChain()
	{
		return mSwapChain.GetReference();
	}

	void runCapture();
	void unInit();

	void doCpuImageCapture();
	void setCpuHook(bool use);

	void SetMainWindow(void* hMain);
	void* GetMainWindow() const;

public:
	core::event<void()> sigGpuImageCapture;
	core::event<void(byte_t * color,core::vec2i viewport)> sigCpuImageCapture;
	core::event<void(const std::string& name)> sigSendUseSharedTexCapture;


	core::event<void(const HANDLE handle)> sigSetSharedViewHandle;

private:
	bool _initSharedCopy();
	void _doSharedCopy();
	
	core::vec2i _getTargetSize();
	void _copyGraphicDXGI(const byte_t* srcBits, byte_t *targetBuffer, int srcPitch, int targetPitch, int format, core::vec2i size);
	bool _createSharedTexture(core::vec2i size, const D3D11_TEXTURE2D_DESC& desc, HANDLE& sharedHandle);
private:

	ObsStudioHook mSwapResizeBuffers;
	ObsStudioHook mSwapPresent;

	TRefCountPtr<ID3D11Device> mDXDevice;
	TRefCountPtr<IDXGISwapChain> mSwapChain;
	TRefCountPtr<ID3D11DeviceContext> mDeviceContext;
	TRefCountPtr<ID3D11Texture2D>	 mSharedTex;

	struct dxGameDataFrame
	{
		TRefCountPtr<ID3D11Texture2D>	gameBackBuffer;
		TRefCountPtr<ID3D11Texture2D>	multiSampleBackbuffer;
		TRefCountPtr<ID3D11Texture2D>	copyTexture;
	};
	dxGameDataFrame		mGameDataFrame;
	std::shared_ptr<win::frame_swapchain> mOutputSwapChain;
	win32::shared_memory<HANDLE> mSharedHandle;
	bool mUseSharedCopy = false;
	bool mCanUseSharedCopy = true;
	void* mHMainWindow = nullptr;

};

