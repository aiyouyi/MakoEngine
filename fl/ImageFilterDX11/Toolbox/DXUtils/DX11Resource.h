#pragma once
#include "DX11IndexBuffer.h"
#include "DX11VertexBuffer.h"
#include "DX11BlendState.h"
#include "DX11BlendState.h"
#include "DX11DepthStencilState.h"
#include "DX11SampleState.h"
#include "DX11RasterizerState.h"
#include "DX11ContanstBuffer.h"
#include "DX11Texture2D.h"
#include "DX11RenderTarget.h"
#include "DX11CubeMapRHI.h"
#include "Toolbox/DXUtils/DX11DoubleBuffer.h"

template<class T>
struct TD3D11ResourceTraits
{
};

template<>
struct TD3D11ResourceTraits<CC3DVertexBuffer>
{
	typedef DX11VertexBuffer TConcreteType;
};

template<>
struct TD3D11ResourceTraits<CC3DIndexBuffer>
{
	typedef DX11IndexBuffer TConcreteType;
};

template<>
struct TD3D11ResourceTraits<CC3DDepthStencilState>
{
	typedef DX11DepthStencilState TConcreteType;
};

template<> 
struct TD3D11ResourceTraits<CC3DBlendState>
{
	typedef DX11BlendState TConcreteType;
};

template<>
struct TD3D11ResourceTraits<CC3DSamplerState>
{
	typedef DX11SampleState TConcreteType;
};

template<>
struct TD3D11ResourceTraits<CC3DRasterizerState>
{
	typedef DX11RasterizerState TConcreteType;
};

template<>
struct TD3D11ResourceTraits<CC3DConstantBuffer>
{
	typedef DX11ConstantBuffer TConcreteType;
};

template<>
struct TD3D11ResourceTraits<CC3DTextureRHI>
{
	typedef DX11Texture2D TConcreteType;
};

template<>
struct TD3D11ResourceTraits<CC3DRenderTargetRHI>
{
	typedef DX11RenderTarget TConcreteType;
};

template<>
struct TD3D11ResourceTraits<CC3DCubeMapRHI>
{
	typedef DX11CubeMapRHI TConcreteType;
};

template<>
struct TD3D11ResourceTraits<DoubleBufferRHI>
{
	typedef DX11DoubleBuffer TConcreteType;
};

template<typename TRHIType>
static FORCEINLINE typename TD3D11ResourceTraits<TRHIType>::TConcreteType* RHIResourceCast(TRHIType* Resource)
{
	return static_cast<typename TD3D11ResourceTraits<TRHIType>::TConcreteType*>(Resource);
}