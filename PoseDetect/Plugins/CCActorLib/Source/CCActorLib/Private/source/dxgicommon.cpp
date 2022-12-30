#include "win/dxgicommon.h"
#include "RHI/Public/RHI.h"
#include "RHI/Public/RHIResources.h"
#include "Runtime/RHI/Public/DynamicRHI.h"
#include "Runtime/Core/Public/Templates/RefCounting.h"
#include "core/system.h"

inline DXGI_FORMAT FixCopyTextureFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return format;
}


void DXGI::CopyGraphicDXGI(const byte_t* srcBits, byte_t *targetBuffer, int srcPitch, int targetPitch, int format, core::vec2i size)
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
				BYTE r = *pSrcCopy++;
				BYTE g = *pSrcCopy++;
				BYTE b = *pSrcCopy++;

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
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
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

//DXGI_FORMAT DXGI::FixCopyTextureFormat(DXGI_FORMAT format)
//{
//	switch (format)
//	{
//	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;
//	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
//	}
//
//	return format;
//}

HMODULE DXGI::GetSystemModule(const wchar_t *module)
{
	wchar_t system_path[MAX_PATH] = { 0 };
	GetSystemDirectoryW(system_path, MAX_PATH);
	wchar_t base_path[MAX_PATH];

	wcscpy(base_path, system_path);
	wcscat(base_path, L"\\");
	wcscat(base_path, module);
	return GetModuleHandleW(base_path);
}

std::wstring DXGI::GetSystemDLLPath()
{
	wchar_t system_path[MAX_PATH] = { 0 };
	GetSystemDirectoryW(system_path, MAX_PATH);
	return system_path;
}

std::tuple<std::shared_ptr<byte_t>, core::vec2i> DXGI::Dx11TexCpuRead(ID3D11Texture2D* Tex2D)
{
	ID3D11Texture2D* Tex = (ID3D11Texture2D*)Tex2D;
	TRefCountPtr<ID3D11Device> Device;
	Tex->GetDevice(Device.GetInitReference());


	HRESULT hRes = S_FALSE;
	D3D11_TEXTURE2D_DESC desc;
	Tex->GetDesc(&desc);

	TRefCountPtr<ID3D11Texture2D> CopyTexture;

	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.MiscFlags = 0;
	desc.ArraySize = 1;
	desc.SampleDesc.Quality = 0;

	hRes = Device->CreateTexture2D(&desc, NULL, CopyTexture.GetInitReference());
	if (FAILED(hRes))
	{
		return { std::shared_ptr<byte_t>(),core::vec2i() };
	}

	TRefCountPtr<ID3D11DeviceContext> DeviceContex;
	Device->GetImmediateContext(DeviceContex.GetInitReference());

	DeviceContex->CopyResource(CopyTexture, Tex);

	D3D11_MAPPED_SUBRESOURCE mappedSrcData;
	hRes = DeviceContex->Map(CopyTexture, 0, D3D11_MAP_READ, 0, &mappedSrcData);
	if (FAILED(hRes))
	{
		return { std::shared_ptr<byte_t>(),core::vec2i() };
	}

	CopyTexture->GetDesc(&desc);
	core::vec2i Viewport(desc.Width, desc.Height);

	std::shared_ptr<byte_t> color(new byte_t[Viewport.cx * Viewport.cy * 4], [](byte_t* ptr)
		{
			delete[]ptr;
		});

	int targetPitch = Viewport.cx * 4;
	if (color)
	{
		CopyGraphicDXGI((byte_t*)mappedSrcData.pData, color.get(), mappedSrcData.RowPitch, targetPitch, desc.Format, Viewport);
	}

	DeviceContex->Unmap(CopyTexture, 0);

	return { color,Viewport };
}

std::tuple<std::shared_ptr<byte_t>, core::vec2i> CCACTORLIB_API DXGI::Dx11TexCpuRead(ID3D11Resource* Res)
{
	TRefCountPtr<ID3D11Texture2D> Tex;
	Res->QueryInterface(Tex.GetInitReference());
	TRefCountPtr<ID3D11Device> Device;
	Res->GetDevice(Device.GetInitReference());

	HRESULT hRes = S_FALSE;
	D3D11_TEXTURE2D_DESC desc;
	Tex->GetDesc(&desc);

	TRefCountPtr<ID3D11Texture2D> CopyTexture;

	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.MiscFlags = 0;
	desc.ArraySize = 1;
	desc.SampleDesc.Quality = 0;

	hRes = Device->CreateTexture2D(&desc, NULL, CopyTexture.GetInitReference());
	if (FAILED(hRes))
	{
		return { std::shared_ptr<byte_t>(),core::vec2i() };
	}

	TRefCountPtr<ID3D11DeviceContext> DeviceContex;
	Device->GetImmediateContext(DeviceContex.GetInitReference());

	DeviceContex->CopyResource(CopyTexture, Tex);

	D3D11_MAPPED_SUBRESOURCE mappedSrcData;
	hRes = DeviceContex->Map(CopyTexture, 0, D3D11_MAP_READ, 0, &mappedSrcData);
	if (FAILED(hRes))
	{
		return { std::shared_ptr<byte_t>(),core::vec2i() };
	}

	CopyTexture->GetDesc(&desc);
	core::vec2i Viewport(desc.Width, desc.Height);

	std::shared_ptr<byte_t> color(new byte_t[Viewport.cx * Viewport.cy * 4], [](byte_t* ptr)
		{
			delete[]ptr;
		});

	int targetPitch = Viewport.cx * 4;
	if (color)
	{
		CopyGraphicDXGI((byte_t*)mappedSrcData.pData, color.get(), mappedSrcData.RowPitch, targetPitch, desc.Format, Viewport);
	}

	DeviceContex->Unmap(CopyTexture, 0);

	return { color,Viewport };
}

void DXGI::DumpDX11Res(const wchar_t* path, ID3D11Texture2D* Tex, uint32_t Interval)
{
	if (core::isFileExist(path) && Tex)
	{
		static DWORD dwTickStart = 0;
		if (GetTickCount() - dwTickStart > Interval)
		{
			auto [color, Viewport] = Dx11TexCpuRead(Tex);
			if (color)
			{
				std::wstring dir = path;
				std::wostringstream ss;
				ss << dir << L"/tmp_" << dwTickStart << L".bmp";

				core::saveAsBMP((char*)color.get(), Viewport.cx * Viewport.cy * 4, Viewport.cx, Viewport.cy, 32, ss.str().c_str());
			}
			dwTickStart = GetTickCount();
		}
	}
}

void CCACTORLIB_API DXGI::DumpDX11Res(const wchar_t* path, ID3D11Resource* Res, uint32_t Interval)
{
	if (core::isFileExist(path) && Res)
	{
		static DWORD dwTickStart = 0;
		if (GetTickCount() - dwTickStart > Interval)
		{
			auto [color, Viewport] = Dx11TexCpuRead(Res);
			if (color)
			{
				std::wstring dir = path;
				std::wostringstream ss;
				ss << dir << L"/tmp_" << dwTickStart << L".bmp";

				core::saveAsBMP((char*)color.get(), Viewport.cx * Viewport.cy * 4, Viewport.cx, Viewport.cy, 32, ss.str().c_str());
			}
			dwTickStart = GetTickCount();
		}
	}
}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif