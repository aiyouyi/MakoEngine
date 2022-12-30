#pragma once
#include "CoreMinimal.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <d3d11.h>
#include "Windows/HideWindowsPlatformTypes.h"
#include <Runtime/Core/Public/Templates/RefCounting.h>

class CCACTORLIB_API FEffectDX11Wrapper
{
public:
	FEffectDX11Wrapper();
	~FEffectDX11Wrapper();

	bool CreateDevice();
	void InitRenderState();
	bool CreateEventQuery();
	void BeginEvent();
	void EndEvent();
	void Wait();

	void SetDeviceAndContext(TRefCountPtr<ID3D11Device> device, TRefCountPtr<ID3D11DeviceContext> context);

	TRefCountPtr<ID3D11Device> GetDevice();
	TRefCountPtr<ID3D11DeviceContext> GetDeviceContext();

	void SetState();
	void SetDepthState();

private:
	TRefCountPtr<ID3D11Device> mDevice;
	TRefCountPtr<ID3D11DeviceContext> mContext;
	TRefCountPtr<ID3D11Query> mEventQuery;


	TRefCountPtr<ID3D11RasterizerState> pRasterizerState;
	TRefCountPtr<ID3D11BlendState> pBlendState;
	TRefCountPtr<ID3D11DepthStencilState> pDepthState;
};