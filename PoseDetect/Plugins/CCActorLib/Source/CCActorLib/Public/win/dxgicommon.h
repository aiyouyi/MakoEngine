#pragma once
#include "core/inc.h"
#include "core/vec2.h"
#include "win/win32.h"
#include <d3d11.h>
#include <Runtime/Core/Public/CoreMinimal.h>

struct ID3D11Resource;
struct ID3D11Texture2D;

namespace DXGI
{
	void CCACTORLIB_API CopyGraphicDXGI(const byte_t* srcBits, byte_t *targetBuffer, int srcPitch, int targetPitch, int format, core::vec2i size);
	//DXGI_FORMAT CCACTORLIB_API FixCopyTextureFormat(DXGI_FORMAT format);
	HMODULE CCACTORLIB_API GetSystemModule(const wchar_t *module);
	std::wstring CCACTORLIB_API GetSystemDLLPath();
	std::tuple<std::shared_ptr<byte_t>, core::vec2i> CCACTORLIB_API Dx11TexCpuRead(ID3D11Texture2D* Tex2D);
	std::tuple<std::shared_ptr<byte_t>, core::vec2i> CCACTORLIB_API Dx11TexCpuRead(ID3D11Resource* Res);
	void CCACTORLIB_API DumpDX11Res(const wchar_t* path, ID3D11Texture2D* Tex2D, uint32_t Interval);
	void CCACTORLIB_API DumpDX11Res(const wchar_t* path, ID3D11Resource* Res, uint32_t Interval);
}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
	#include "Windows/HideWindowsPlatformTypes.h"
#endif