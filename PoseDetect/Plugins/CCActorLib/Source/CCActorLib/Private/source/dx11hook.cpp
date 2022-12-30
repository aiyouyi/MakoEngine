#include "dx11hook.h"

#include <Runtime/RenderCore/Public/RenderingThread.h>
#include "checkdata.h"
#include "core/logger.h"
#include "va/va.h"
#ifndef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
#include <d3d11.h>

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif


#pragma comment(lib,"dxguid.lib")

typedef HRESULT(STDMETHODCALLTYPE *DXGISwapPresentHookProc)(IDXGISwapChain *swap, UINT syncInterval, UINT flags);
typedef HRESULT(STDMETHODCALLTYPE *DXGISwapResizeBuffersHookProc)(IDXGISwapChain *swap, UINT bufferCount,
	UINT width, UINT height, DXGI_FORMAT giFormat, UINT flags);

static HRESULT STDMETHODCALLTYPE DX11SwapPresentHook(IDXGISwapChain *swap, UINT syncInterval, UINT flags);
static HRESULT STDMETHODCALLTYPE DX11SwapResizeBuffersHook(IDXGISwapChain *swap, UINT bufferCount,
	UINT width, UINT height, DXGI_FORMAT giFormat, UINT flags);

static HRESULT g_hDX11Present = S_FALSE;

char system_path[MAX_PATH] = { 0 };
static inline HMODULE get_system_module(const char *module)
{
	GetSystemDirectoryA(system_path, MAX_PATH);
	char base_path[MAX_PATH];

	strcpy(base_path, system_path);
	strcat(base_path, "\\");
	strcat(base_path, module);
	return GetModuleHandleA(base_path);
}

DXGI_FORMAT fixCopyTextureFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return format;
}


CDX11Hook::CDX11Hook()
{

}

CDX11Hook::~CDX11Hook()
{
	CCheckData::Release();
}

CDX11Hook& CDX11Hook::getInstance()
{
	// create instance by new and don't delete it
	// if use static variable, the instance will destruct when program exit
	// because we build CCActorLib as a DLL now, the instance will destruct in DllMain
	// then you will crash by:
	// The release of the last IDXGIFactory object should not be called from DllMain
	static CDX11Hook* hook = new CDX11Hook;
	return *hook;
}

bool CDX11Hook::hookFunction(bool canUseSharedCopy)
{
	CCheckData::GetInstance()->StartCheckSharedCopy();


	std::tuple<int64_t, int64_t, int64_t> dxOffset = CCheckData::GetInstance()->GetDXOffset();
	int64_t presentVal = std::get<0>(dxOffset);
	int64_t present1Val = std::get<1>(dxOffset);
	int64_t resizeVal = std::get<2>(dxOffset);
	//mGameModeBase = gameMode;
	mCanUseSharedCopy = canUseSharedCopy;
	HMODULE hModule = get_system_module("dxgi.dll");
	mSwapPresent.Hook((void*)((UINT_PTR)hModule+ presentVal), (void*)DX11SwapPresentHook);
	mSwapResizeBuffers.Hook((void*)((UINT_PTR)hModule + resizeVal), ((void*)DX11SwapResizeBuffersHook));


	mSwapPresent.Rehook();
	mSwapResizeBuffers.Rehook();

	return true;
}


bool CDX11Hook::initDX11(IDXGISwapChain *swap)
{
	TRefCountPtr<IUnknown> deviceUnk;
	TRefCountPtr<IUnknown> device;
	if (SUCCEEDED(swap->GetDevice(__uuidof(IUnknown), (void **)&deviceUnk)))
	{
		if (SUCCEEDED(deviceUnk->QueryInterface(__uuidof(ID3D11Device), (void **)&device)))
		{
			//LOGINFO(__FUNCTIONW__  L" get ID3D11Device success.");

			TRefCountPtr<ID3D11Device> device11;
			swap->GetDevice(__uuidof(ID3D11Device), (void**)device11.GetInitReference());
			mDXDevice = device11;
			mSwapChain = swap;

			TRefCountPtr<ID3D11DeviceContext> context;
			device11->GetImmediateContext(context.GetInitReference());
			mDeviceContext = context;

			if (mCanUseSharedCopy)
			{
				mUseSharedCopy = _initSharedCopy();
			}
			

			if (!mOutputSwapChain)
			{
				mOutputSwapChain = std::make_shared<win::frame_swapchain>();
				mOutputSwapChain->create(core::format(va::VIRTUAL_ACTOR_SWAPCHAIN_NAME, ".", va::client_process_id(), ".", va::client_session_id()));
			}

			return true;
		}
	}
	return false;
}

void CDX11Hook::runCapture()
{
	if (mUseSharedCopy /*&&  CCheckData::GetInstance()->IsEnableSharedResource()*/)
	{
		core::logger::inf() <<  core::logger_period(__FILE__, __LINE__, 1h) << __FUNCTION__ " shared copy";
		_doSharedCopy();
	}
	else
	{
		core::logger::inf() <<  core::logger_period(__FILE__, __LINE__, 1h) << __FUNCTION__ " normal copy";
		doCpuImageCapture();
	}
	
}

void CDX11Hook::unInit()
{
	mDXDevice.SafeRelease();
	mSwapChain.SafeRelease();
	mDeviceContext.SafeRelease();
	mSharedTex.SafeRelease();

	mGameDataFrame.gameBackBuffer.SafeRelease();
	mGameDataFrame.multiSampleBackbuffer.SafeRelease();
	mGameDataFrame.copyTexture.SafeRelease();

}

bool CDX11Hook::_initSharedCopy()
{
	if (!mSwapChain.IsValid())
	{
		return false;
	}

	TRefCountPtr<ID3D11Texture2D> pBackBuffer;
	HRESULT hRes = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBuffer.GetInitReference());
	if (FAILED(hRes))
	{
		//LOGERROR(__FUNCTIONW__ L" get back buffer fail 0x%x", hRes);
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	pBackBuffer->GetDesc(&desc);

	HRESULT hr = S_OK;
	TRefCountPtr<IDXGIDevice> dxgiDevice;
	if (FAILED(hr = mDXDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.GetInitReference())))
	{
		return false;
	}

	TRefCountPtr<IDXGIAdapter> dxgiAdapter;
	if (FAILED(hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)dxgiAdapter.GetInitReference())))
	{
		return false;
	}

	DXGI_ADAPTER_DESC adapterDesc;
	dxgiAdapter->GetDesc(&adapterDesc);

	core::vec2i size = _getTargetSize();
	HANDLE sharedHandle = NULL;
	if (_createSharedTexture(size, desc, sharedHandle))
	{
		mSharedHandle.access("VirtualActorHandle", sizeof(HANDLE), win32::shared_mode::readwrite);
		if (mSharedHandle.valid())
		{
			*mSharedHandle = sharedHandle;

			return true;
		}
	}
	return false;
}

void CDX11Hook::_doSharedCopy()
{
	mGameDataFrame.gameBackBuffer.SafeRelease();
	HRESULT hRes = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)mGameDataFrame.gameBackBuffer.GetInitReference());
	if (FAILED(hRes))
	{
		//LOGERROR(__FUNCTIONW__ L" m_swapChain->GetBuffer FAIL 0x%x", hRes);
		return;
	}

	D3D11_TEXTURE2D_DESC desc;
	mGameDataFrame.gameBackBuffer->GetDesc(&desc);
	switch (desc.Format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		break;
	default:
		//LOGERROR(__FUNCTIONW__ L" surface format=0x%x not supported.", desc.Format);
		return;
	}

	if (desc.SampleDesc.Count > 1)
	{
		if (!mGameDataFrame.multiSampleBackbuffer.IsValid())
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = 0;
			desc.MipLevels = 1;
			desc.SampleDesc.Count = 1;
			desc.MiscFlags = 0;
			desc.ArraySize = 1;

			hRes = mDXDevice->CreateTexture2D(&desc, NULL, mGameDataFrame.multiSampleBackbuffer.GetInitReference());
			if (FAILED(hRes))
			{
				//LOGERROR(__FUNCTIONW__ L"  FAILED to call CreateTexture2D for pBackBufferResolved. 0x%x", hRes);
				return;
			}
		}

		mDeviceContext->ResolveSubresource(mGameDataFrame.multiSampleBackbuffer, 0, mGameDataFrame.gameBackBuffer, 0, desc.Format);
		mGameDataFrame.gameBackBuffer = mGameDataFrame.multiSampleBackbuffer;
	}

	mDeviceContext->CopyResource(mSharedTex, mGameDataFrame.gameBackBuffer);
	
	sigGpuImageCapture();
	sigSendUseSharedTexCapture("VirtualActorHandle");

	sigSetSharedViewHandle(*mSharedHandle);
}

void CDX11Hook::doCpuImageCapture()
{
	mGameDataFrame.gameBackBuffer.SafeRelease();
	HRESULT hRes = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)mGameDataFrame.gameBackBuffer.GetInitReference());
	if (FAILED(hRes))
	{
		//LOGERROR(__FUNCTIONW__ L" m_swapChain->GetBuffer FAIL 0x%x", hRes);
		return;
	}

	D3D11_TEXTURE2D_DESC desc;
	mGameDataFrame.gameBackBuffer->GetDesc(&desc);
	switch (desc.Format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		break;
	default:
		//LOGERROR(__FUNCTIONW__ L" surface format=0x%x not supported.", desc.Format);
		return;
	}

	if (!mGameDataFrame.copyTexture.IsValid())
	{
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.MiscFlags = 0;
		desc.ArraySize = 1;
		desc.SampleDesc.Quality = 0;

		hRes = mDXDevice->CreateTexture2D(&desc, NULL, mGameDataFrame.copyTexture.GetInitReference());
		if (FAILED(hRes))
		{
			//LOGERROR(__FUNCTIONW__ L"  FAILED to call CreateTexture2D(). 0x%x", hRes);
			return;
		}
	}

	mGameDataFrame.gameBackBuffer->GetDesc(&desc);

	if (desc.SampleDesc.Count > 1)
	{
		if (!mGameDataFrame.multiSampleBackbuffer.IsValid())
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = 0;
			desc.MipLevels = 1;
			desc.SampleDesc.Count = 1;
			desc.MiscFlags = 0;
			desc.ArraySize = 1;

			hRes = mDXDevice->CreateTexture2D(&desc, NULL, mGameDataFrame.multiSampleBackbuffer.GetInitReference());
			if (FAILED(hRes))
			{
				//LOGERROR(__FUNCTIONW__ L"  FAILED to call CreateTexture2D for pBackBufferResolved. 0x%x", hRes);
				return;
			}
		}

		mDeviceContext->ResolveSubresource(mGameDataFrame.multiSampleBackbuffer, 0, mGameDataFrame.gameBackBuffer, 0, desc.Format);
		mGameDataFrame.gameBackBuffer = mGameDataFrame.multiSampleBackbuffer;
	}

	mDeviceContext->CopyResource(mGameDataFrame.copyTexture, mGameDataFrame.gameBackBuffer);

	D3D11_MAPPED_SUBRESOURCE mappedSrcData;
	hRes = mDeviceContext->Map(mGameDataFrame.copyTexture, 0, D3D11_MAP_READ, 0, &mappedSrcData);
	if (FAILED(hRes))
	{
		//LOGERROR(__FUNCTIONW__ L"  FAILED to call m_deviceContext->Map(). 0x%x", hRes);
		return;
	}

	mGameDataFrame.gameBackBuffer->GetDesc(&desc);
	core::vec2i Viewport(desc.Width, desc.Height);

	int targetPitch = Viewport.cx * 4;
	byte_t * color = (byte_t *)(mOutputSwapChain->write(Viewport.cx, Viewport.cy));
	if (color)
	{
		_copyGraphicDXGI((byte_t*)mappedSrcData.pData, color, mappedSrcData.RowPitch, targetPitch, desc.Format, Viewport);
	}

	//bool useSharedCopy = mUseSharedCopy && CCheckData::GetInstance()->IsEnableSharedResource();
	//if (!useSharedCopy)
	//{
	//	mOutputSwapChain->present();
	//	sigSendUseSharedTexCapture("");
	//}

	mDeviceContext->Unmap(mGameDataFrame.copyTexture, 0);

	sigCpuImageCapture(color, Viewport);

}

void CDX11Hook::setCpuHook(bool use)
{
	mCanUseSharedCopy = !use;
	mUseSharedCopy = !use;
}

void CDX11Hook::SetMainWindow(void* hMain)
{
	mHMainWindow = hMain;
}

void* CDX11Hook::GetMainWindow() const
{
	return mHMainWindow;
}

core::vec2i CDX11Hook::_getTargetSize()
{
	core::vec2i size = { 0,0 };
	if (!mSwapChain.IsValid())	// PresentFrameBegin not called yet!
	{
		return size;
	}

	// get the backbuffer and size
	TRefCountPtr<ID3D11Texture2D> pBackBuffer;
	HRESULT hRes = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBuffer.GetInitReference());
	if (FAILED(hRes))
	{
		//LOGERROR(__FUNCTIONW__ L" get back buffer fail 0x%x", hRes);
		return size;
	}

	D3D11_TEXTURE2D_DESC desc;
	pBackBuffer->GetDesc(&desc);
	size.cx = desc.Width;
	size.cy = desc.Height;

	return { size.cx,size.cy };
}

void CDX11Hook::_copyGraphicDXGI(const byte_t* srcBits, byte_t *targetBuffer, int srcPitch, int targetPitch, int format, core::vec2i size)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		//		LOGINFO(L"GetFrameFullSize: source format = DXGI_FORMAT_R8G8B8A8_UNORM or DXGI_FORMAT_R8G8B8A8_UNORM_SRGB");
		// 32-bit entries: discard alpha
		// swap R and B channels (RGBA to BGRA) DX9 used little endianness	
	{
		BYTE *pSrcBegin = const_cast<BYTE *>(srcBits);
		BYTE *pDest = targetBuffer;

		for (int height = 0; height < size.cy; height++)
		{
			BYTE *pSrcCopy = pSrcBegin;
			BYTE *pDestWrite = pDest;
			for (int width = 0; width < size.cx; ++width)
			{
				BYTE b = *pSrcCopy++;
				BYTE g = *pSrcCopy++;
				BYTE r = *pSrcCopy++;

				*pDestWrite++ = r;
				*pDestWrite++ = g;
				*pDestWrite++ = b;
				*pDestWrite++ = *pSrcCopy++;
			}
			pSrcBegin = pSrcBegin + srcPitch;
			pDest = pDest + targetPitch;
		}
	}
	break;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	{
		LPBYTE src = (LPBYTE)srcBits;
		LPBYTE dst = (LPBYTE)targetBuffer;
		for (int i = 0; i < size.cy; i++)
		{
			memcpy(dst, src, targetPitch);
			src += srcPitch;
			dst += targetPitch;
		}
	}
	break;

	case DXGI_FORMAT_R10G10B10A2_UNORM:
		for (int i = 0, k = 0; i < size.cy && k < size.cy; i++, k++)
		{
			for (int j = 0; j < size.cx; j++)
			{
				WORD w1 = MAKEWORD(srcBits[k*srcPitch + j * 4], srcBits[k*srcPitch + j * 4 + 1]);
				WORD w2 = MAKEWORD(srcBits[k*srcPitch + j * 4 + 2], srcBits[k*srcPitch + j * 4 + 3]);
				DWORD dw = MAKELONG(w1, w2);

				UINT b = (dw & 0x000003ff);
				UINT g = ((dw >> 10) & 0x000003ff);
				UINT r = ((dw >> 20) & 0x000003ff);

				targetBuffer[i*targetPitch + j * 4] = (BYTE)(r >> 2);
				targetBuffer[i*targetPitch + j * 4 + 1] = (BYTE)(g >> 2);
				targetBuffer[i*targetPitch + j * 4 + 2] = (BYTE)(b >> 2);
				targetBuffer[i*targetPitch + j * 4 + 3] = 255;
			}
		}
		break;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	{
		BYTE *pSrcStart = const_cast<BYTE *>(srcBits);
		BYTE *pDest = targetBuffer;

		for (int i = 0; i < size.cy; i++)
		{
			BYTE *pSrcCopy = pSrcStart;
			BYTE *pDestWrite = pDest;
			for (int width = 0; width < size.cx; ++width)
			{
				*pDestWrite++ = *pSrcCopy++;
				*pDestWrite++ = *pSrcCopy++;
				*pDestWrite++ = *pSrcCopy++;
				*pDestWrite++ = 255;
				++pSrcCopy;
			}
			pSrcStart = pSrcStart + srcPitch;
			pDest = pDest + targetPitch;
		}
	}
	break;
	}
}

bool CDX11Hook::_createSharedTexture(core::vec2i size, const D3D11_TEXTURE2D_DESC& desc, HANDLE& sharedHandle)
{
	HRESULT hErr;

	D3D11_TEXTURE2D_DESC	State = desc;
	State.Width = size.cx;
	State.Height = size.cy;
	State.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	State.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

	if (!mDXDevice.IsValid())
	{
		return false;
	}

	if (FAILED(hErr = mDXDevice->CreateTexture2D(&State, NULL, mSharedTex.GetInitReference())))
	{
		//LOGERROR(__FUNCTIONW__ L" creation of intermediary texture failed, result = 0x%x", hErr);
		return false;
	}

	TRefCountPtr<IDXGIResource> res;
	if (FAILED(hErr = mSharedTex->QueryInterface(IID_IDXGIResource, (void**)&res)))
	{
		//LOGERROR(__FUNCTIONW__ L" d3d11Tex->QueryInterface(IID_IDXGIResource) failed, result = 0x%x", hErr);
		return false;
	}

	if (FAILED(hErr = res->GetSharedHandle(&sharedHandle)))
	{
		//LOGERROR(__FUNCTIONW__ L" res->GetSharedHandle failed, result = 0x%x", hErr);
		return false;
	}

	return true;
}

static HRESULT STDMETHODCALLTYPE DX11SwapPresentHook(IDXGISwapChain *swap, UINT syncInterval, UINT flags)
{
	CDX11Hook& hookDX11 = CDX11Hook::getInstance();

	DXGI_SWAP_CHAIN_DESC desc;
	swap->GetDesc(&desc);

	if (desc.OutputWindow == hookDX11.GetMainWindow())
	{
		if (swap != hookDX11.currentSwapChain())
		{
			hookDX11.unInit();
		}

		if ((flags & DXGI_PRESENT_TEST) == 0)
		{
			if (hookDX11.currentSwapChain() == nullptr)
			{
				hookDX11.initDX11(swap);
			}
			hookDX11.runCapture();
		}
	}

	hookDX11.swapPresent().Unhook();
	HRESULT hRes = ((DXGISwapPresentHookProc)hookDX11.swapPresent().GetCallAddress())(swap, syncInterval, flags);
	hookDX11.swapPresent().Rehook();

	return hRes;
}

static HRESULT STDMETHODCALLTYPE DX11SwapResizeBuffersHook(IDXGISwapChain *swap, UINT bufferCount,
	UINT width, UINT height, DXGI_FORMAT giFormat, UINT flags)
{
	CDX11Hook& hookDX11 = CDX11Hook::getInstance();

	DXGI_SWAP_CHAIN_DESC desc;
	swap->GetDesc(&desc);

	if (desc.OutputWindow == hookDX11.GetMainWindow())
	{
		hookDX11.unInit();
	}
	

	hookDX11.swapResizeBuffer().Unhook();
	HRESULT hRes = ((DXGISwapResizeBuffersHookProc)hookDX11.swapResizeBuffer().GetCallAddress())(swap, bufferCount, width, height, giFormat, flags);
	hookDX11.swapResizeBuffer().Rehook();
	return hRes;
}
