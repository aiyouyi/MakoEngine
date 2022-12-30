#include "va/CopyRectEffect.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "win/dxgicommon.h"
#include "core/system.h"
#include "win/dxgicommon.h"

extern RENDERER_API void DrawRectangle(
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

IMPLEMENT_SHADER_TYPE(,FCopyRectLive2dVS, TEXT("/Engine/Private/ScreenPass.usf"), TEXT("ScreenPassVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(,FCopyRectPostEffectPS, TEXT("/Plugin/CCActorLib/Private/CopyRectEffect.usf"), TEXT("CopyRectPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(,FCopyRectToneMappingEffectPS, TEXT("/Plugin/CCActorLib/Private/CopyRectEffect.usf"), TEXT("CopyRectToneMappingPS"), SF_Pixel)



FCopyRectEffect::FCopyRectEffect()
{

}

FCopyRectEffect::~FCopyRectEffect()
{
	Destroy();
}

void FCopyRectEffect::Init(UObject* WorldContextObject,uint32_t width, uint32_t height)
{
	mOutRenderTargetTexture = NewObject<UTextureRenderTarget2D>(WorldContextObject);
	mOutRenderTargetTexture->AddToRoot();
	mOutRenderTargetTexture->bGPUSharedFlag = true;
	mOutRenderTargetTexture->ClearColor = FLinearColor::Transparent;
	mOutRenderTargetTexture->TargetGamma = 1.0f;
	const bool bForceLinearGamma = true;
	mOutRenderTargetTexture->InitCustomFormat(width, height, PF_B8G8R8A8, bForceLinearGamma);
}

void FCopyRectEffect::Destroy()
{
	if (mOutRenderTargetTexture)
	{
		mOutRenderTargetTexture->RemoveFromRoot();
		mOutRenderTargetTexture->MarkPendingKill();
		mOutRenderTargetTexture = nullptr;
 	}
}

void FCopyRectEffect::InnerDrawRectangle(FRHICommandList& RHICmdList, float X, float Y, float SizeX, float SizeY, float U, float V, float SizeU, float SizeV, 
	FIntPoint TargetSize, FIntPoint TextureSize, const TShaderRef<FShader>& VertexShader, EDrawRectangleFlags Flags /*= EDRF_Default*/, uint32 InstanceCount /*= 1 */)
{
	DrawRectangle(RHICmdList, X, Y, SizeX, SizeY, U, V, SizeU, SizeV, TargetSize, TextureSize, VertexShader, Flags, InstanceCount);
}

void FCopyRectEffect::DumpRenderTarget()
{
	DXGI::DumpDX11Res(L"d:/vadump/", static_cast<ID3D11Resource*>(mOutRenderTargetTexture->GetRenderTargetResource()->GetTexture2DRHI()->GetNativeResource()), 5 * 1000);
}

