#pragma once
#include "CoreMinimal.h"
#include "Templates/RefCounting.h"
#include "win/win32.h"
#include <d3d11.h>
#ifdef WINDOWS_PLATFORM_TYPES_GUARD
	#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include <memory>


class FEffectDX11Wrapper;
class CCACTORLIB_API DX11Texture
{
public:
	DX11Texture(std::shared_ptr<FEffectDX11Wrapper> DX11Wrapper );
	~DX11Texture();

	bool InitTexture(DXGI_FORMAT format, uint32_t BindFlags, int width, int height, uint32_t MiscFlag, uint32_t CpuFlag,bool MsAA = false);
	bool UpdateTextureInfo(void *pBuffer,int left,int top,int right,int bottom, int SrcRowPitch, int SrcDepthPitch);
	bool ReadTextureToCpu(void *pBuffer);
	void CopyResource(TRefCountPtr<ID3D11Texture2D> src);
	void ResloveResource(TRefCountPtr<ID3D11Texture2D> src);

	bool OpenResource(HANDLE SharedHandle);

	TRefCountPtr<ID3D11Texture2D> GetTex();
	TRefCountPtr<ID3D11ShaderResourceView> GetTexShaderView();
	TRefCountPtr<ID3D11RenderTargetView> GetRTV();
	HANDLE GetSharedHandle();

	int GetWidth();
	int GetHeight();

	void Destory();

private:
	TRefCountPtr<ID3D11ShaderResourceView> m_texShaderView;
	TRefCountPtr<ID3D11RenderTargetView> m_RTV;
	TRefCountPtr<ID3D11Texture2D> m_pTexture;
	HANDLE  m_SharedHandle = nullptr;

	int m_nWidth = 0;
	int m_nHeight = 0;
	std::shared_ptr<FEffectDX11Wrapper> mDX11Wrapper;
	bool m_MSAA = false;
};

