#include "win/shared_tex.h"
#include "win/win32.h"
#include "Misc/App.h"
#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3d11on12.h>
#include "RHI/Public/RHI.h"
#include "RHI/Public/RHIResources.h"
#include "Runtime/RHI/Public/DynamicRHI.h"
#include "Runtime/Core/Public/Templates/RefCounting.h"
#include "win/shared_memory.h"
#include "win/dll.h"
#include "core/system.h"
#include "win/dxgicommon.h"

DXGI_FORMAT EnsureNotTypeless(DXGI_FORMAT fmt)
{
	// Assumes UNORM or FLOAT; doesn't use UINT or SINT
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case DXGI_FORMAT_R32G32B32_TYPELESS:    return DXGI_FORMAT_R32G32B32_FLOAT;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_UNORM;
	case DXGI_FORMAT_R32G32_TYPELESS:       return DXGI_FORMAT_R32G32_FLOAT;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return DXGI_FORMAT_R10G10B10A2_UNORM;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_R16G16_TYPELESS:       return DXGI_FORMAT_R16G16_UNORM;
	case DXGI_FORMAT_R32_TYPELESS:          return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R8G8_TYPELESS:         return DXGI_FORMAT_R8G8_UNORM;
	case DXGI_FORMAT_R16_TYPELESS:          return DXGI_FORMAT_R16_UNORM;
	case DXGI_FORMAT_R8_TYPELESS:           return DXGI_FORMAT_R8_UNORM;
	case DXGI_FORMAT_BC1_TYPELESS:          return DXGI_FORMAT_BC1_UNORM;
	case DXGI_FORMAT_BC2_TYPELESS:          return DXGI_FORMAT_BC2_UNORM;
	case DXGI_FORMAT_BC3_TYPELESS:          return DXGI_FORMAT_BC3_UNORM;
	case DXGI_FORMAT_BC4_TYPELESS:          return DXGI_FORMAT_BC4_UNORM;
	case DXGI_FORMAT_BC5_TYPELESS:          return DXGI_FORMAT_BC5_UNORM;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:     return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:     return DXGI_FORMAT_B8G8R8X8_UNORM;
	case DXGI_FORMAT_BC7_TYPELESS:          return DXGI_FORMAT_BC7_UNORM;
	default:                                return fmt;
	}
}

struct SharedTexP
{
	TRefCountPtr<ID3D11Device> mDX11Device;
	TRefCountPtr<ID3D11DeviceContext> mDeviceContext;
	TRefCountPtr<ID3D11Texture2D>	 mSharedTex;
	TRefCountPtr<ID3D11Texture2D>	mDX11MultiSampleBackbuffer;

	TRefCountPtr<ID3D12Device> mDX12Device;
	TRefCountPtr<ID3D11On12Device>    mDevice11on12;
	TRefCountPtr<ID3D11Resource> mD3d11On12Res;

	HANDLE mSharedHandle;
	win32::dll mD3D11DLL;

	void release()
	{
		mDX11Device.SafeRelease();
		mDeviceContext.SafeRelease();
		mSharedTex.SafeRelease();
		mDX11MultiSampleBackbuffer.SafeRelease();
		mDX12Device.SafeRelease();
		mDevice11on12.SafeRelease();
		mD3d11On12Res.SafeRelease();
		//mSharedHandle.close();
		mD3D11DLL.release();
	}
};

SharedTex::SharedTex()
	:mData(new SharedTexP())
{
	FGuid guid;
	FGenericPlatformMisc::CreateGuid(guid);
	mName = TCHAR_TO_UTF8(*guid.ToString());

	FString apiName = FApp::GetGraphicsRHI();
	if (apiName == TEXT("DirectX 11"))
	{
		mAPI = DirectX_11;
	}
	else if (apiName == TEXT("DirectX 12"))
	{
		mAPI = DirectX_12;
	}
}

SharedTex::~SharedTex()
{
	if (mData)
	{
		mData->release();
	}
	delete mData;
}

bool SharedTex::init(void *nativeRes)
{
	if (mAPI == DirectX_11)
	{
		return initDX11(nativeRes);
	}
	else if (mAPI == DirectX_12)
	{
		return initDX12(nativeRes);
	}
	return false;
}

void SharedTex::doSharedCopy(void *nativeRes)
{
	if (mAPI == DirectX_11)
	{
		doDX11SharedCopy(nativeRes);
	}
	else if (mAPI == DirectX_12)
	{
		doDX12SharedCopy(nativeRes);
	}
}

bool SharedTex::isValid()
{
	return mData->mSharedTex.IsValid();
}

uint64_t SharedTex::getSharedHandle()
{
	return (uint64_t)mData->mSharedHandle;
}

void SharedTex::Dump(void* nativeRes)
{
	if (mAPI == DirectX_12)
	{
		return;
	}

	DXGI::DumpDX11Res(L"D:/vadump", static_cast<ID3D11Resource*>(nativeRes), 5 * 1000);
}



bool SharedTex::initDX11(void *nativeRes)
{
	mData->mDX11Device = static_cast<ID3D11Device*>(GDynamicRHI->RHIGetNativeDevice());
	ID3D11Resource* d3d11Res = static_cast<ID3D11Resource*>(nativeRes);

	if (!mData->mDX11Device || !d3d11Res)
	{
		return false;
	}

	mData->mDX11Device->GetImmediateContext(mData->mDeviceContext.GetInitReference());

	TRefCountPtr<ID3D11Texture2D> d3d11Tex;
	d3d11Res->QueryInterface(d3d11Tex.GetInitReference());
	if (!d3d11Tex.IsValid())
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	d3d11Tex->GetDesc(&desc);

	core::vec2i size;
	size.cx = desc.Width;
	size.cy = desc.Height;

	return createSharedTexture(size,desc);
}

bool SharedTex::initDX12(void *nativeRes)
{
	mData->mDX12Device = static_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
	ID3D12Resource* d3d12Res = static_cast<ID3D12Resource*>(nativeRes);

	if (!mData->mDX12Device || !d3d12Res)
	{
		return false;
	}

	if (!initD3D11On12())
	{
		return false;
	}

	return createSharedTexture(d3d12Res);
}

bool SharedTex::createSharedTexture(core::vec2i size, const D3D11_TEXTURE2D_DESC& desc)
{
	HRESULT hErr;

	D3D11_TEXTURE2D_DESC State = desc;
	State.Format = EnsureNotTypeless(desc.Format);
	State.Width = size.cx;
	State.Height = size.cy;
	State.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	State.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

	if (FAILED(hErr = mData->mDX11Device->CreateTexture2D(&State, NULL, mData->mSharedTex.GetInitReference())))
	{
		//LOGERROR(__FUNCTIONW__ L" creation of intermediary texture failed, result = 0x%x", hErr);
		return false;
	}

	TRefCountPtr<IDXGIResource> res;
	if (FAILED(hErr = mData->mSharedTex->QueryInterface(IID_IDXGIResource, (void**)&res)))
	{
		//LOGERROR(__FUNCTIONW__ L" d3d11Tex->QueryInterface(IID_IDXGIResource) failed, result = 0x%x", hErr);
		return false;
	}

	mData->mSharedHandle = nullptr;
	if (FAILED(hErr = res->GetSharedHandle(&mData->mSharedHandle)))
	{
		//LOGERROR(__FUNCTIONW__ L" res->GetSharedHandle failed, result = 0x%x", hErr);
		return false;
	}

	//mData->mSharedHandle.access(mName, sizeof(HANDLE), win32::shared_mode::readwrite);
	//if (mData->mSharedHandle.valid())
	//{
	//	*mData->mSharedHandle = sharedHandle;
	//}

	return true;
}


bool SharedTex::createSharedTexture(ID3D12Resource* d3d12Res)
{
	HRESULT hr = S_OK;

	D3D12_RESOURCE_DESC desc = d3d12Res->GetDesc();

	core::vec2i size;
	size.cx = desc.Width;
	size.cy = desc.Height;

	D3D11_RESOURCE_FLAGS rf11 = {};

	hr = mData->mDevice11on12->CreateWrappedResource(d3d12Res, &rf11,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_PRESENT,
		IID_PPV_ARGS(mData->mD3d11On12Res.GetInitReference()));
	if (FAILED(hr)) {
		return false;
	}


	D3D11_TEXTURE2D_DESC desc11 = {};
	desc11.Width = size.cx;
	desc11.Height = size.cy;
	desc11.MipLevels = 1;
	desc11.ArraySize = 1;
	desc11.Format = EnsureNotTypeless(desc.Format);
	desc11.SampleDesc.Count = 1;
	desc11.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc11.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

	hr = mData->mDX11Device->CreateTexture2D(&desc11, nullptr, mData->mSharedTex.GetInitReference());
	if (FAILED(hr)) {
		return false;
	}

	mData->mDevice11on12->ReleaseWrappedResources((ID3D11Resource**)&mData->mD3d11On12Res, 1);

	IDXGIResource *dxgi_res;
	hr = mData->mSharedTex->QueryInterface(__uuidof(IDXGIResource),
		(void**)&dxgi_res);
	if (FAILED(hr)) {
		return false;
	}

	mData->mSharedHandle = NULL;
	hr = dxgi_res->GetSharedHandle(&mData->mSharedHandle);
	dxgi_res->Release();
	if (FAILED(hr)) {
		return false;
	}

	//mData->mSharedHandle.access(mName, sizeof(HANDLE), win32::shared_mode::readwrite);
	//if (mData->mSharedHandle.valid())
	//{
	//	*mData->mSharedHandle = sharedHandle;
	//}

	return true;
}

void SharedTex::doDX11SharedCopy(void *nativeRes)
{
	ID3D11Resource* d3d11Res = static_cast<ID3D11Resource*>(nativeRes);
	TRefCountPtr<ID3D11Texture2D> d3d11Tex;
	d3d11Res->QueryInterface(d3d11Tex.GetInitReference());

	if (!d3d11Tex.IsValid())
	{
		return;
	}

	D3D11_TEXTURE2D_DESC desc;
	d3d11Tex->GetDesc(&desc);

	if (desc.SampleDesc.Count > 1)
	{
		if (!mData->mDX11MultiSampleBackbuffer.IsValid())
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = 0;
			desc.MipLevels = 1;
			desc.SampleDesc.Count = 1;
			desc.MiscFlags = 0;
			desc.ArraySize = 1;

			HRESULT hRes = mData->mDX11Device->CreateTexture2D(&desc, NULL, mData->mDX11MultiSampleBackbuffer.GetInitReference());
			if (FAILED(hRes))
			{
				//LOGERROR(__FUNCTIONW__ L"  FAILED to call CreateTexture2D for pBackBufferResolved. 0x%x", hRes);
				return;
			}
		}

		mData->mDeviceContext->ResolveSubresource(mData->mDX11MultiSampleBackbuffer, 0, d3d11Tex, 0, desc.Format);
		d3d11Tex = mData->mDX11MultiSampleBackbuffer;
	}

	mData->mDeviceContext->CopyResource(mData->mSharedTex, d3d11Tex);
}

void SharedTex::doDX12SharedCopy(void *nativeRes)
{
	if (!mData->mDevice11on12.IsValid())
	{
		return;
	}


	mData->mDevice11on12->AcquireWrappedResources((ID3D11Resource**)&mData->mD3d11On12Res, 1);

	mData->mDeviceContext->CopyResource(mData->mSharedTex, mData->mD3d11On12Res);

	mData->mDevice11on12->ReleaseWrappedResources((ID3D11Resource**)&mData->mD3d11On12Res, 1);
	mData->mDeviceContext->Flush();

}

bool SharedTex::initD3D11On12()
{
	if (mData->mDevice11on12.IsValid())
	{
		return true;
	}

	std::wstring strPath = DXGI::GetSystemDLLPath();
	strPath += L"\\d3d11.dll";

	typedef PFN_D3D11ON12_CREATE_DEVICE create_11_on_12_t;
	static create_11_on_12_t create_11_on_12 = nullptr;
	static bool initialized_func = false;

	if (core::error_ok != mData->mD3D11DLL.load(core::ucs2_u8(strPath)))
	{
		return false;
	}


	if (!initialized_func && !create_11_on_12) {
		create_11_on_12 = (create_11_on_12_t)mData->mD3D11DLL.proc("D3D11On12CreateDevice");
		if (!create_11_on_12) {

		}

		initialized_func = true;
	}

	if (!create_11_on_12) {
		return false;
	}

	HRESULT hr = create_11_on_12(mData->mDX12Device.GetReference(), 0, nullptr, 0,
		nullptr, 0, 0,
		mData->mDX11Device.GetInitReference(), mData->mDeviceContext.GetInitReference(), nullptr);
	if (FAILED(hr)) {
		return false;
	}


	mData->mDX11Device->QueryInterface(IID_PPV_ARGS(mData->mDevice11on12.GetInitReference()));
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif
