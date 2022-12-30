#include "win/EffectDX11Wrapper.h"

FEffectDX11Wrapper::FEffectDX11Wrapper()
{

}

FEffectDX11Wrapper::~FEffectDX11Wrapper()
{

}

bool FEffectDX11Wrapper::CreateDevice()
{
	HRESULT hr = S_OK;

	// 创建D3D设备 和 D3D设备上下文
	UINT createDeviceFlags = 0;
	// 驱动类型数组
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	// 特性等级数组
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel;
	D3D_DRIVER_TYPE d3dDriverType;
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, mDevice.GetInitReference(), &featureLevel, mContext.GetInitReference());

		if (hr == E_INVALIDARG)
		{
			// Direct3D 11.0 的API不承认D3D_FEATURE_LEVEL_11_1，所以我们需要尝试特性等级11.0以及以下的版本
			hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, mDevice.GetInitReference(), &featureLevel, mContext.GetInitReference());
		}

		if (SUCCEEDED(hr))
			break;
	}

	return true;
}

void FEffectDX11Wrapper::InitRenderState()
{
	D3D11_RASTERIZER_DESC rasterizer_desc;
	memset(&rasterizer_desc, 0, sizeof(D3D11_RASTERIZER_DESC));
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.DepthClipEnable = false;
	rasterizer_desc.FrontCounterClockwise = true;
	
	mDevice->CreateRasterizerState(&rasterizer_desc, pRasterizerState.GetInitReference());

	//控制混合状态
	D3D11_BLEND_DESC blend_desc;
	memset(&blend_desc, 0, sizeof(blend_desc));
	blend_desc.AlphaToCoverageEnable = false;
	blend_desc.IndependentBlendEnable = false;
	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	mDevice->CreateBlendState(&blend_desc, pBlendState.GetInitReference());


	//深度状态控制
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	memset(&depth_stencil_desc, 0, sizeof(depth_stencil_desc));
	depth_stencil_desc.DepthEnable = false;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.StencilEnable = 0;
	depth_stencil_desc.StencilReadMask = 0;
	depth_stencil_desc.StencilWriteMask = 0;

	mDevice->CreateDepthStencilState(&depth_stencil_desc, pDepthState.GetInitReference());

}

bool FEffectDX11Wrapper::CreateEventQuery()
{
	D3D11_QUERY_DESC qd;
	ZeroMemory(&qd, sizeof(qd));
	qd.Query = D3D11_QUERY_EVENT;
	HRESULT hr = mDevice->CreateQuery(&qd, mEventQuery.GetInitReference());
	return SUCCEEDED(hr);
}

void FEffectDX11Wrapper::BeginEvent()
{
	if (mEventQuery)
	{
		mContext->Begin(mEventQuery);
	}
}

void FEffectDX11Wrapper::EndEvent()
{
	if (mEventQuery)
	{
		mContext->End(mEventQuery);
	}
}

void FEffectDX11Wrapper::Wait()
{
	if (mEventQuery)
	{
		while (S_FALSE == mContext->GetData(mEventQuery, NULL /*&GPUfinished*/, 0 /*sizeof(GPUfinished)*/, 0));
	}
	
}

void FEffectDX11Wrapper::SetDeviceAndContext(TRefCountPtr<ID3D11Device> device, TRefCountPtr<ID3D11DeviceContext> context)
{
	mDevice = device;
	mContext = context;
}

TRefCountPtr<ID3D11Device> FEffectDX11Wrapper::GetDevice()
{
	return mDevice;
}

TRefCountPtr<ID3D11DeviceContext> FEffectDX11Wrapper::GetDeviceContext()
{
	return mContext;
}

void FEffectDX11Wrapper::SetState()
{
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//mContext->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);
	mContext->RSSetState(pRasterizerState);
	mContext->OMSetDepthStencilState(pDepthState, 0);
	//mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void FEffectDX11Wrapper::SetDepthState()
{
	mContext->OMSetDepthStencilState(pDepthState, 0);
}
