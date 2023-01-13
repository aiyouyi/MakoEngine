#ifndef __GLResource__
#define __GLResource__

#include "GLIndexBuffer.h"
#include "GLVertexBuffer.h"
#include "GLBlendState.h"
#include "GLDepthStencilState.h"
#include "GLSamplerState.h"
#include "GLRasterizerState.h"
#include "GLConstantBuffer.h"
#include "GLTexture2D.h"
#include "GLRenderTarget.h"
#include "GLCubeMap.h"
#include "DoubleBuffer.h"

template<class T>
struct TGLResourceTraits
{
};


template<>
struct TGLResourceTraits<CC3DIndexBuffer>
{
	typedef GLIndexBuffer TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DVertexBuffer>
{
	typedef GLVertexBuffer TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DBlendState>
{
	typedef GLBlendState TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DDepthStencilState>
{
	typedef GLDepthStencilState TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DSamplerState>
{
	typedef GLSamplerState TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DRasterizerState>
{
	typedef GLRasterizerState TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DConstantBuffer>
{
	typedef GLConstantBuffer TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DTextureRHI>
{
	typedef GLTexture2D TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DRenderTargetRHI>
{
	typedef GLRenderTarget TConcreteType;
};

template<>
struct TGLResourceTraits<CC3DCubeMapRHI>
{
	typedef GLCubeMap TConcreteType;
};

template<>
struct TGLResourceTraits<DoubleBufferRHI>
{
	typedef GLDoubleBuffer TConcreteType;
};

template<typename TRHIType>
static inline typename TGLResourceTraits<TRHIType>::TConcreteType* RHIResourceCast(TRHIType* Resource)
{
	return static_cast<typename TGLResourceTraits<TRHIType>::TConcreteType*>(Resource);
}


#endif