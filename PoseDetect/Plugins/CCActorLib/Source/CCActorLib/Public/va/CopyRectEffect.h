#pragma once
#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "RHIStaticStates.h"
#include "CommonRenderResources.h"
#include "ShaderParameterUtils.h"
#include "core/inc.h"

class UTextureRenderTarget2D;


class CCACTORLIB_API FCopyRectLive2dVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCopyRectLive2dVS);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FCopyRectLive2dVS() {};
	FCopyRectLive2dVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{}
};

/** Pixel shader to copy pixels from src to dst performing a format change that works on all platforms. */
class  CCACTORLIB_API FCopyRectPostEffectPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCopyRectPostEffectPS);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FCopyRectPostEffectPS() {}

	FCopyRectPostEffectPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		InGamma.Bind(Initializer.ParameterMap, TEXT("CopyRectGamma"));
		InTexture.Bind(Initializer.ParameterMap, TEXT("CopyRectInputTex"));
		InTextureSampler.Bind(Initializer.ParameterMap, TEXT("CopyInputSampler"));
		InToneMappingType.Bind(Initializer.ParameterMap, TEXT("ToneMappingType"));
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(
		FRHICommandListImmediate& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		FTextureRHIRef ShaderResourceTexture,
		float Gamma,
		float ToneMappingType
	)
	{
		SetShaderValue(RHICmdList, ShaderRHI, InGamma, Gamma);
		SetTextureParameter(RHICmdList, ShaderRHI, InTexture, ShaderResourceTexture);
		SetSamplerParameter(RHICmdList, ShaderRHI, InTextureSampler, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
		SetShaderValue(RHICmdList, ShaderRHI, InToneMappingType, ToneMappingType);

	}

protected:
	LAYOUT_FIELD(FShaderParameter,InGamma);
	LAYOUT_FIELD(FShaderParameter, InToneMappingType);
	LAYOUT_FIELD(FShaderResourceParameter,InTexture);
	LAYOUT_FIELD(FShaderResourceParameter,InTextureSampler);
};

class CCACTORLIB_API FCopyRectToneMappingEffectPS : public FCopyRectPostEffectPS
{
public:
	DECLARE_GLOBAL_SHADER(FCopyRectToneMappingEffectPS);
	
	FCopyRectToneMappingEffectPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FCopyRectPostEffectPS(Initializer)
	{
	}
	FCopyRectToneMappingEffectPS() {}
};

class CCACTORLIB_API FCopyRectEffect
{
public:
	FCopyRectEffect();
	~FCopyRectEffect();

	void Init(UObject* WorldContextObject,uint32_t width, uint32_t height);
	void Destroy();
	bool IsVaild() { return mOutRenderTargetTexture != nullptr; }
	template<typename PostEffectPS>
	void CopyTexture(FRHICommandListImmediate& RHICmdList, FIntPoint SrcSize, std::function<void(TShaderMapRef< PostEffectPS >& PixelShader)> &paramFunction);
	UTextureRenderTarget2D* GetOutPutTarget() {
		return mOutRenderTargetTexture;
	}

private:
	 void InnerDrawRectangle(
		FRHICommandList& RHICmdList,
		float X,
		float Y,
		float SizeX,
		float SizeY,
		float U,
		float V,
		float SizeU,
		float SizeV,
		FIntPoint TargetSize,
		FIntPoint TextureSize,
		const TShaderRef<FShader>& VertexShader,
		EDrawRectangleFlags Flags = EDRF_Default,
		uint32 InstanceCount = 1
	);

	 void DumpRenderTarget();

private:
	UTextureRenderTarget2D* mOutRenderTargetTexture = nullptr;

};

template<typename PostEffectPS>
void FCopyRectEffect::CopyTexture(FRHICommandListImmediate& RHICmdList, FIntPoint SrcSize, std::function<void(TShaderMapRef< PostEffectPS >& PixelShader)> &paramFunction)
{
	RHICmdList.PushEvent(TEXT("CopyRectEffect"), FColor::Black);
	
	FTextureRenderTargetResource* TextureRenderTargetResource = mOutRenderTargetTexture->GetRenderTargetResource();
	FRHITexture2D* RenderTargetTexture = TextureRenderTargetResource->GetRenderTargetTexture();
	RHICmdList.TransitionResource(ERHIAccess::EWritable, RenderTargetTexture);

	FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Load_Store, TextureRenderTargetResource->TextureRHI);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("CopyRectEffect"));


	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(ERHIFeatureLevel::SM5);
	TShaderMapRef< FCopyRectLive2dVS > VertexShader(GlobalShaderMap);
	TShaderMapRef< PostEffectPS > PixelShader(GlobalShaderMap);

	// Set the graphic pipeline state.
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;

	// Update viewport.
	RHICmdList.SetViewport(
		0, 0, 0.f,
		RenderTargetTexture->GetSizeX(), RenderTargetTexture->GetSizeY(), 1.f);

	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

	//SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), *PassParameters);
	//////////////////////////////////////////////////////////////////////////
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
	//PixelShader->SetParameters(RHICmdList, PixelShader->GetPixelShader(), inTexture, mGamma);
	paramFunction(PixelShader);

	// Draw a quad mapping scene color to the view's render target

	FIntPoint DestSize(RenderTargetTexture->GetSizeXY());
	FIntRect SrcRect;
	SrcRect.Min = FIntPoint(0, 0);
	SrcRect.Max = SrcSize;
	FIntRect DestRect;
	DestRect.Min = FIntPoint(0, 0);
	DestRect.Max = RenderTargetTexture->GetSizeXY();

	InnerDrawRectangle(
		RHICmdList,
		DestRect.Min.X, DestRect.Min.Y,
		DestRect.Width(), DestRect.Height(),
		SrcRect.Min.X, SrcRect.Min.Y,
		SrcRect.Width(), SrcRect.Height(),
		DestSize,
		SrcSize,
		VertexShader,
		EDRF_UseTriangleOptimization);

	RHICmdList.EndRenderPass();
	RHICmdList.PopEvent();


	DumpRenderTarget();
}