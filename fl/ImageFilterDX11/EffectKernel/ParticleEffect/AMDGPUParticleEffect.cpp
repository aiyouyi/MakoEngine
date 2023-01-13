#include "AMDGPUParticleEffect.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/ShaderRHI.h"
#include "EffectKernel/BaseRenderParam.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/MaterialTexRHI.h"
#include "CC3DEngine/Scene/CC3DProject.h"
#include "CC3DEngine/Scene/CC3DCamera.h"
#include "Toolbox/irefptr.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/DXUtils/DX11DepthBuffer.h"
#include <DirectXMath.h>

// GPUParticle structure is split into two sections for better cache efficiency - could even be SOA but would require creating more vertex buffers.
struct GPUParticlePartA
{
	DirectX::XMVECTOR	m_params[3];
};

struct GPUParticlePartB
{
	DirectX::XMVECTOR	m_params[3];
};

// Helper function to align values
int align(int value, int alignment) { return (value + (alignment - 1)) & ~(alignment - 1); }


inline float RandomVariance(float median, float variance)
{
	float fUnitRandomValue = (float)rand() / (float)RAND_MAX;
	float fRange = variance * fUnitRandomValue;
	return median - variance + (2.0f * fRange);
}
inline float RandomFromAndTo(float lowest, float highest)
{
	float fUnitRandomValue = (float)rand() / (float)RAND_MAX;
	float fRange = (highest - lowest) * fUnitRandomValue;
	return lowest + fRange;
}

static const int g_maxParticles = 400 * 1024;
// The per-emitter constant buffer
struct EmitterConstantBuffer
{
	DirectX::XMVECTOR	m_EmitterPosition;
	DirectX::XMVECTOR	m_EmitterVelocity;
	DirectX::XMVECTOR	m_PositionVariance;

	int					m_MaxParticlesThisFrame;
	float				m_ParticleLifeSpan;
	float				m_StartSize;
	float				m_EndSize;


	float				m_VelocityVariance;
	float				m_Mass;
	int					m_Index;
	int					m_Streaks;

	int					m_TextureIndex;
	int					pads[3];
};

static const int NUM_EMITTERS = 4;

struct PER_FRAME_CONSTANT_BUFFER
{
	DirectX::XMVECTOR	m_StartColor[NUM_EMITTERS];
	DirectX::XMVECTOR	m_EndColor[NUM_EMITTERS];
	DirectX::XMVECTOR	m_EmitterLightingCenter[NUM_EMITTERS];

	DirectX::XMMATRIX	m_ViewProjection;
	DirectX::XMMATRIX	m_ViewProjInv;
	DirectX::XMMATRIX	m_View;
	DirectX::XMMATRIX	m_ViewInv;
	DirectX::XMMATRIX	m_Projection;
	DirectX::XMMATRIX	m_ProjectionInv;

	DirectX::XMVECTOR	m_EyePosition;
	DirectX::XMVECTOR	m_SunDirection;
	DirectX::XMVECTOR	m_SunColor;
	DirectX::XMVECTOR	m_AmbientColor;

	DirectX::XMVECTOR	m_SunDirectionVS;
	DirectX::XMVECTOR	pads2[3];

	float				m_FrameTime  ;
	int					m_ScreenWidth;
	int					m_ScreenHeight;
	int					m_FrameIndex;

	float				m_AlphaThreshold;
	float				m_CollisionThickness;
	float				m_ElapsedTime;
	int					m_CollisionsEnabled;

	int					m_ShowSleepingParticles;
	int					m_EnableSleepState;
	int					pads[2];

};


// Per-emitter parameters
struct EmitterParams
{
	DirectX::XMVECTOR	m_Position;				// World position of the emitter
	DirectX::XMVECTOR	m_Velocity;				// Velocity of each particle from the emitter
	DirectX::XMVECTOR	m_PositionVariance;		// Variance in position of each particle
	int					m_NumToEmit;			// Number of particles to emit this frame
	float				m_ParticleLifeSpan;		// How long the particles should live
	float				m_StartSize;			// Size of particles at spawn time
	float				m_EndSize;				// Size of particle when they reach retirement age
	float				m_Mass;					// Mass of particle
	float				m_VelocityVariance;		// Variance in velocity of each particle
	int					m_TextureIndex;			// Index of the texture in the atlas
	bool				m_Streaks;				// Streak the particles in the direction of travel
};

struct EmissionRate
{
	float		m_ParticlesPerSecond;	// Number of particles to emit per second
	float		m_Accumulation;			// Running total of how many particles to emit over elapsed time
};


struct AMDGPUParticleEffectP
{
	std::shared_ptr<ShaderRHI> CSInitDeadList;
	std::shared_ptr<ShaderRHI> CSEmit;
	std::shared_ptr<ShaderRHI> CSSimulate;
	std::shared_ptr<ShaderRHI> CSReset;
	std::shared_ptr<ShaderRHI> VSStructuredBuffer;
	std::shared_ptr<ShaderRHI> PSRasterized;

	IRefPtr<ID3D11Buffer> ParticleBufferA;
	IRefPtr<ID3D11ShaderResourceView> ParticleBufferA_SRV;
	IRefPtr<ID3D11UnorderedAccessView> ParticleBufferA_UAV;

	IRefPtr<ID3D11Buffer> ParticleBufferB;
	IRefPtr<ID3D11ShaderResourceView> ParticleBufferB_SRV;
	IRefPtr<ID3D11UnorderedAccessView> ParticleBufferB_UAV;

	IRefPtr<ID3D11Buffer> ViewSpaceParticlePositions;
	IRefPtr<ID3D11ShaderResourceView> ViewSpaceParticlePositionsSRV;
	IRefPtr<ID3D11UnorderedAccessView> ViewSpaceParticlePositionsUAV;

	IRefPtr<ID3D11Buffer> MaxRadiusBuffer;
	IRefPtr<ID3D11ShaderResourceView> MaxRadiusBufferSRV;
	IRefPtr<ID3D11UnorderedAccessView> MaxRadiusBufferUAV;

	IRefPtr<ID3D11Buffer> DeadListBuffer;
	IRefPtr<ID3D11UnorderedAccessView> DeadListUAV;

	IRefPtr<ID3D11Buffer> DeadListConstantBuffer;
	IRefPtr<ID3D11Buffer> ActiveListConstantBuffer;

	IRefPtr<ID3D11Buffer> IndexBuffer;

	IRefPtr<ID3D11Buffer> EmitterConstantBuf;

	IRefPtr<ID3D11Buffer> AliveIndexBuffer;
	IRefPtr<ID3D11ShaderResourceView> AliveIndexBufferSRV;
	IRefPtr<ID3D11UnorderedAccessView> AliveIndexBufferUAV;

	std::shared_ptr<CC3DTextureRHI> RandomTexture;

	IRefPtr<ID3D11Buffer> IndirectDrawArgsBuffer;
	IRefPtr<ID3D11UnorderedAccessView> IndirectDrawArgsBufferUAV;

	IRefPtr< ID3D11BlendState> CompositeBlendState;

	EmitterParams SparksEmitter{};
	EmissionRate  SparksEmissionRate{};

	// Constant buffers
	IRefPtr<ID3D11Buffer> PerFrameConstantBuffer;
	PER_FRAME_CONSTANT_BUFFER	GlobalConstantBuffer{};

	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;

	float Delta = 1000.f / 60.f * 0.001f;

	bool ResetSystem = true;
	bool StopEmit = false;
	std::shared_ptr<MaterialTexRHI> ParticalTex;
	std::shared_ptr< DX11DepthBuffer> ParticaleDepth;
	const float CameraZ = 10.f;
};

AMDGPUParticleEffect::AMDGPUParticleEffect()
	:Data(std::make_unique<AMDGPUParticleEffectP>())
{
	DirectX::XMVECTOR spawnPosition = DirectX::XMVectorSet(2.0f, 100.0f, 26.0f, 1.0f);

	// Sparks
	Data->SparksEmissionRate.m_ParticlesPerSecond = 1500.0f;

	Data->SparksEmitter.m_Position = spawnPosition;
	Data->SparksEmitter.m_Velocity = DirectX::XMVectorSet(0.0f, -4.f, 0.0f, 0.0f);
	Data->SparksEmitter.m_NumToEmit = 0;
	Data->SparksEmitter.m_ParticleLifeSpan = 200.0f;
	Data->SparksEmitter.m_StartSize = 0.2f;
	Data->SparksEmitter.m_EndSize = 0.1f;
	Data->SparksEmitter.m_PositionVariance = DirectX::XMVectorSet(5.0f, 0.0f, 5.0f, 1.0f);
	Data->SparksEmitter.m_VelocityVariance = 0.4f;
	Data->SparksEmitter.m_Mass = 0.5f;
	Data->SparksEmitter.m_TextureIndex = 1;
	Data->SparksEmitter.m_Streaks = false;

	Data->GlobalConstantBuffer.m_StartColor[0] = DirectX::XMVectorSet(10.0f, 10.0f, 0.0f, 1.0f);
	Data->GlobalConstantBuffer.m_EndColor[0] = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);

	Data->GlobalConstantBuffer.m_SunDirection = DirectX::XMVector4Normalize(DirectX::XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f));

	Data->GlobalConstantBuffer.m_SunColor = DirectX::XMVectorSet(0.8f, 0.8f, 0.7f, 0.0f);
	Data->GlobalConstantBuffer.m_AmbientColor = DirectX::XMVectorSet(0.2f, 0.2f, 0.3f, 0.0f);

	Data->GlobalConstantBuffer.m_FrameIndex = 0;
	Data->GlobalConstantBuffer.m_ElapsedTime = 0.0f;

}

AMDGPUParticleEffect::~AMDGPUParticleEffect()
{
	Data.reset();
}

void AMDGPUParticleEffect::SetShaderResource(const std::string& path, std::shared_ptr<MaterialTexRHI> Tex)
{
	Data->ParticalTex = Tex;
	std::string ShaderPath = path + "/Shader/Particle/InitDeadList.hlsl";
	Data->CSInitDeadList = GetDynamicRHI()->CreateShaderRHI();
	Data->CSInitDeadList->initComputeShader(ShaderPath, "CS_InitDeadList");
	
	ShaderPath = path + "/Shader/Particle/ParticleEmit.hlsl";
	Data->CSEmit = GetDynamicRHI()->CreateShaderRHI();
	Data->CSEmit->initComputeShader(ShaderPath, "CS_Emit");

	ShaderPath = path + "/Shader/Particle/ParticleRender.hlsl";
	Data->VSStructuredBuffer = GetDynamicRHI()->CreateShaderRHI();
	Data->VSStructuredBuffer->initVSShader(ShaderPath, "VS_StructuredBuffer",false);

	ShaderPath = path + "/Shader/Particle/ParticleSimulation.hlsl";
	Data->CSSimulate = GetDynamicRHI()->CreateShaderRHI();
	Data->CSSimulate->initComputeShader(ShaderPath, "CS_Simulate");

	Data->CSReset = GetDynamicRHI()->CreateShaderRHI();
	Data->CSReset->initComputeShader(ShaderPath, "CS_Reset");

	ShaderPath = path + "/Shader/Particle/ParticleRender.hlsl";
	Data->PSRasterized = GetDynamicRHI()->CreateShaderRHI();
	Data->PSRasterized->initPSShader(ShaderPath, "PS_Billboard");

	InitRenderResource();

	DirectX::XMVECTOR vEye = DirectX::XMVectorSet(0.f, 0.f, Data->CameraZ, 1.f);
	DirectX::XMVECTOR vLookAt = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
	DirectX::XMVECTOR vWorldUp = DirectX::XMVectorSet(0.f, 1.f, 0.f, 1.f);
	Data->View = DirectX::XMMatrixLookAtLH(vEye, vLookAt, vWorldUp);
}

void AMDGPUParticleEffect::Render(BaseRenderParam& Scene)
{
	//Scene.GetDoubleBuffer()->BindFBOA();

	auto D3d11DoubleBuffer = RHIResourceCast(Scene.GetDoubleBuffer().get());

	if (!Data->ParticaleDepth)
	{
		Data->ParticaleDepth = std::make_shared<DX11DepthBuffer>();
		Data->ParticaleDepth->Create32Float(Scene.GetWidth(), Scene.GetHeight(), D3d11DoubleBuffer->IsMsaa());
	}

	// Set the viewport to be the full screen area
	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)Scene.GetWidth();
	vp.Height = (float)Scene.GetHeight();
	DeviceContextPtr->RSSetViewports(1, &vp);

	auto RTV = D3d11DoubleBuffer->GetFBOA()->getRenderTargetView();
	// Make the render target our intermediate buffer (ie not the back buffer)
	DeviceContextPtr->OMSetRenderTargets(1, &RTV, Data->ParticaleDepth->GetDSV());

	DeviceContextPtr->ClearDepthStencilView(Data->ParticaleDepth->GetDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);

	//// Compute some matrices for our per-frame constant buffer
	Data->GlobalConstantBuffer.m_ScreenHeight = Scene.GetHeight();
	Data->GlobalConstantBuffer.m_ScreenWidth = Scene.GetWidth();

	DirectX::XMMATRIX ViewProjection = Data->View * Data->Proj;

	Data->GlobalConstantBuffer.m_ViewProjection = DirectX::XMMatrixTranspose(ViewProjection);
	Data->GlobalConstantBuffer.m_View = DirectX::XMMatrixTranspose(Data->View);
	Data->GlobalConstantBuffer.m_Projection = DirectX::XMMatrixTranspose(Data->Proj);

	DirectX::XMMATRIX viewProjInv = DirectX::XMMatrixInverse(nullptr, ViewProjection);
	Data->GlobalConstantBuffer.m_ViewProjInv = DirectX::XMMatrixTranspose(viewProjInv);

	DirectX::XMMATRIX viewInv = DirectX::XMMatrixInverse(nullptr, Data->View);
	Data->GlobalConstantBuffer.m_ViewInv = DirectX::XMMatrixTranspose(viewInv);

	DirectX::XMMATRIX projInv = DirectX::XMMatrixInverse(nullptr, Data->Proj);
	Data->GlobalConstantBuffer.m_ProjectionInv = DirectX::XMMatrixTranspose(projInv);
	Data->GlobalConstantBuffer.m_SunDirectionVS = DirectX::XMVector4Transform(Data->GlobalConstantBuffer.m_SunDirection, Data->View);
	Data->GlobalConstantBuffer.m_EyePosition = DirectX::XMVectorSet(0, 0, 4, 1.f);

	Data->GlobalConstantBuffer.m_FrameTime = Data->Delta;
	Data->GlobalConstantBuffer.m_ElapsedTime += Data->Delta;

	// Wrap the timer so the numbers don't go too high
	const float wrapPeriod = 10.0f;
	if (Data->GlobalConstantBuffer.m_ElapsedTime > wrapPeriod)
	{
		Data->GlobalConstantBuffer.m_ElapsedTime -= wrapPeriod;
	}

	const float AlphaThreshold = 97.f;
	const float CollisionThickness = 40.f;
	Data->GlobalConstantBuffer.m_AlphaThreshold = AlphaThreshold / 100.0f;
	Data->GlobalConstantBuffer.m_CollisionThickness = CollisionThickness * 0.1f;

	Data->GlobalConstantBuffer.m_CollisionsEnabled = 0;
	Data->GlobalConstantBuffer.m_EnableSleepState = 0;
	Data->GlobalConstantBuffer.m_ShowSleepingParticles = 0;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	HRESULT hr = DeviceContextPtr->Map(Data->PerFrameConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	if (SUCCEEDED(hr))
	{
		memcpy(MappedResource.pData, &Data->GlobalConstantBuffer, sizeof(Data->GlobalConstantBuffer));
		DeviceContextPtr->Unmap(Data->PerFrameConstantBuffer, 0);
	}
	else
	{
		return;
	}

	ID3D11ShaderResourceView* srv = 0;
	DeviceContextPtr->CSSetShaderResources(0, 1, &srv);

	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullFront);

	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::WarpLinerSampler, 0);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler, 1);

	GetDynamicRHI()->SetCSSamplerState(CC3DPiplelineState::WarpLinerSampler, 0);
	GetDynamicRHI()->SetCSSamplerState(CC3DPiplelineState::ClampLinerSampler, 1);


	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnableWriteDisable, 0);
	//GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable, 0);

	// Set the per-frame constant buffer on all shader stages
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &Data->PerFrameConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &Data->PerFrameConstantBuffer);
	DeviceContextPtr->CSSetConstantBuffers(0, 1, &Data->PerFrameConstantBuffer);


	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOn, blendFactor, 0xffffffff);

	PopulateEmitter(Data->Delta);

	DeviceContextPtr->OMSetRenderTargets(0, nullptr, nullptr);

	Data->ParticalTex->UpdateTexture();
	GetDynamicRHI()->SetPSShaderResource(0, Data->ParticalTex->GetTextureRHI());

	// If we are resetting the particle system, then initialize the dead list
	if (Data->ResetSystem)
	{
		InitDeadList();

		ID3D11UnorderedAccessView* uavs[] = { Data->ParticleBufferA_UAV, Data->ParticleBufferB_UAV };
		UINT initialCounts[] = { (UINT)-1, (UINT)-1 };

		DeviceContextPtr->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, initialCounts);

		Data->CSReset->UseShader();
		DeviceContextPtr->Dispatch(align(g_maxParticles, 256) / 256, 1, 1);

		Data->ResetSystem = false;
	}

	if (!Data->StopEmit)
	{
		Emit();
	}
	
	Simulate(Scene);

	// Copy the atomic counter in the alive list UAV into a constant buffer for access by subsequent passes
	DeviceContextPtr->CopyStructureCount(Data->ActiveListConstantBuffer, 0, Data->AliveIndexBufferUAV);

	Data->VSStructuredBuffer->UseShader();
	Data->PSRasterized->UseShader();

	
	// Set a null vertex buffer
	ID3D11Buffer* vb = nullptr;
	UINT stride = 0;
	UINT offset = 0;
	DeviceContextPtr->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

	DeviceContextPtr->VSSetConstantBuffers(3, 1, &Data->ActiveListConstantBuffer);

	// Non-GS path is faster but requires an index buffer
	DeviceContextPtr->IASetIndexBuffer(Data->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11ShaderResourceView* vs_srv[] = { Data->ParticleBufferA_SRV, Data->ViewSpaceParticlePositionsSRV, Data->AliveIndexBufferSRV };
	DeviceContextPtr->VSSetShaderResources(0, ARRAYSIZE(vs_srv), vs_srv);

	auto depthSRV = Data->ParticaleDepth->GetDepthSRV();
	DeviceContextPtr->PSSetShaderResources(1, 1, &depthSRV);

	//Scene.GetDoubleBuffer()->BindFBOA();
	DeviceContextPtr->OMSetRenderTargets(1, &RTV, nullptr);
	
	// Render the primitives using the DrawInstancedIndirect API. 
	// The indirect args are filled in prior to this call in a compute shader.
	DeviceContextPtr->DrawIndexedInstancedIndirect(Data->IndirectDrawArgsBuffer, 0);

	ZeroMemory(vs_srv, sizeof(vs_srv));
	DeviceContextPtr->VSSetShaderResources(0, ARRAYSIZE(vs_srv), vs_srv);
	ID3D11ShaderResourceView* ps_srv[1]{nullptr};
	DeviceContextPtr->PSSetShaderResources(2, 1, ps_srv);
}

void AMDGPUParticleEffect::Update(Vector2 EmitPos, BaseRenderParam& Scene)
{
	//Data->Project.SetAspect((float)Scene.GetWidth() / (float)Scene.GetHeight());


	Data->Proj = DirectX::XMMatrixPerspectiveFovLH(45.0f, (float)Scene.GetWidth() / (float)Scene.GetHeight(), 0.1f, 1000.f);

	if ((EmitPos.x - -1.f) < 0.01f && (EmitPos.y - -1.f) < 0.01f)
	{
		Data->StopEmit = true;
	}
	else
	{
		Data->StopEmit = false;
		DirectX::XMMATRIX ViewProj = DirectX::XMMatrixMultiply(Data->View,Data->Proj);
		DirectX::XMMATRIX invProjectionView = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(ViewProj), (ViewProj));

		EmitPos.x = (((2.0f * EmitPos.x) / Scene.GetWidth()) - 1);
		EmitPos.y = -(((2.0f * EmitPos.y) / Scene.GetHeight()) - 1);

		DirectX::XMVECTOR EmitPosW = XMVector4Transform(DirectX::XMVectorSet(EmitPos.x, EmitPos.y, 1.0f, 0.0f), invProjectionView);

		DirectX::XMFLOAT4 LoadData;
		DirectX::XMStoreFloat4(&LoadData, EmitPosW);
		Data->SparksEmitter.m_Position = DirectX::XMVectorSet(LoadData.x *Data->CameraZ, LoadData.y * Data->CameraZ, 0 ,1.0f);
	}
}

void AMDGPUParticleEffect::InitDeadList()
{
	Data->CSInitDeadList->UseShader();

	UINT initialCount[] = { 0 };
	DeviceContextPtr->CSSetUnorderedAccessViews(0, 1, &Data->DeadListUAV, initialCount);

	// Disaptch a set of 1d thread groups to fill out the dead list, one thread per particle
	DeviceContextPtr->Dispatch(align(g_maxParticles, 256) / 256, 1, 1);
}

void AMDGPUParticleEffect::InitRenderResource()
{
	// Create the global particle pool. Each particle is split into two parts for better cache coherency. The first half contains the data more 
// relevant to rendering while the second half is more related to simulation
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(GPUParticlePartA) * g_maxParticles;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(GPUParticlePartA);

	DevicePtr->CreateBuffer(&desc, nullptr, &Data->ParticleBufferA);

	desc.ByteWidth = sizeof(GPUParticlePartB) * g_maxParticles;
	desc.StructureByteStride = sizeof(GPUParticlePartB);

	DevicePtr->CreateBuffer(&desc, nullptr, &Data->ParticleBufferB);

	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	srv.Format = DXGI_FORMAT_UNKNOWN;
	srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv.Buffer.ElementOffset = 0;
	srv.Buffer.ElementWidth = g_maxParticles;

	DevicePtr->CreateShaderResourceView(Data->ParticleBufferA, &srv, &Data->ParticleBufferA_SRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav;
	uav.Format = DXGI_FORMAT_UNKNOWN;
	uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav.Buffer.FirstElement = 0;
	uav.Buffer.NumElements = g_maxParticles;
	uav.Buffer.Flags = 0;
	DevicePtr->CreateUnorderedAccessView(Data->ParticleBufferA, &uav, &Data->ParticleBufferA_UAV);
	DevicePtr->CreateUnorderedAccessView(Data->ParticleBufferB, &uav, &Data->ParticleBufferB_UAV);

	// The view space positions of particles are cached during simulation so allocate a buffer for them
	desc.ByteWidth = 16 * g_maxParticles;
	desc.StructureByteStride = 16;
	DevicePtr->CreateBuffer(&desc, 0, &Data->ViewSpaceParticlePositions);
	DevicePtr->CreateShaderResourceView(Data->ViewSpaceParticlePositions, &srv, &Data->ViewSpaceParticlePositionsSRV);
	DevicePtr->CreateUnorderedAccessView(Data->ViewSpaceParticlePositions, &uav, &Data->ViewSpaceParticlePositionsUAV);

	// The maximum radii of each particle is cached during simulation to avoid recomputing multiple times later. This is only required
// for streaked particles as they are not round so we cache the max radius of X and Y
	desc.ByteWidth = 4 * g_maxParticles;
	desc.StructureByteStride = 4;
	DevicePtr->CreateBuffer(&desc, 0, &Data->MaxRadiusBuffer);
	DevicePtr->CreateShaderResourceView(Data->MaxRadiusBuffer, &srv, &Data->MaxRadiusBufferSRV);
	DevicePtr->CreateUnorderedAccessView(Data->MaxRadiusBuffer, &uav, &Data->MaxRadiusBufferUAV);

	// The dead particle index list. Created as an append buffer
	desc.ByteWidth = sizeof(UINT) * g_maxParticles;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(UINT);

	DevicePtr->CreateBuffer(&desc, nullptr, &Data->DeadListBuffer);

	uav.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
	DevicePtr->CreateUnorderedAccessView(Data->DeadListBuffer, &uav, &Data->DeadListUAV);

	// Create constant buffers to copy the dead and alive list counters into
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.ByteWidth = 4 * sizeof(UINT);
	DevicePtr->CreateBuffer(&desc, nullptr, &Data->DeadListConstantBuffer);
	DevicePtr->CreateBuffer(&desc, nullptr, &Data->ActiveListConstantBuffer);

	// Create the emitter constant buffer
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = sizeof(EmitterConstantBuffer);
	DevicePtr->CreateBuffer(&desc, nullptr, &Data->EmitterConstantBuf);


	struct IndexBufferElement
	{
		float		distance;	// distance squared from the particle to the camera
		float		index;		// global index of the particle
	};

	// Create the index buffer of alive particles that is to be sorted (at least in the rasterization path).
	// For the tiled rendering path this could be just a UINT index buffer as particles are not globally sorted
	desc.ByteWidth = sizeof(IndexBufferElement) * g_maxParticles;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(IndexBufferElement);

	DevicePtr->CreateBuffer(&desc, nullptr, &Data->AliveIndexBuffer);

	srv.Format = DXGI_FORMAT_UNKNOWN;
	srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv.Buffer.ElementOffset = 0;
	srv.Buffer.ElementWidth = g_maxParticles;

	DevicePtr->CreateShaderResourceView(Data->AliveIndexBuffer, &srv, &Data->AliveIndexBufferSRV);

	uav.Buffer.NumElements = g_maxParticles;
	uav.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
	uav.Format = DXGI_FORMAT_UNKNOWN;
	DevicePtr->CreateUnorderedAccessView(Data->AliveIndexBuffer, &uav, &Data->AliveIndexBufferUAV);

	// Create the buffer to store the indirect args for the DrawInstancedIndirect call
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	desc.ByteWidth = 5 * sizeof(UINT);
	desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	DevicePtr->CreateBuffer(&desc, nullptr, &Data->IndirectDrawArgsBuffer);

	ZeroMemory(&uav, sizeof(uav));
	uav.Format = DXGI_FORMAT_R32_UINT;
	uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav.Buffer.FirstElement = 0;
	uav.Buffer.NumElements = 5;
	uav.Buffer.Flags = 0;
	DevicePtr->CreateUnorderedAccessView(Data->IndirectDrawArgsBuffer, &uav, &Data->IndirectDrawArgsBufferUAV);

	// Create the particle billboard index buffer required for the rasterization VS-only path
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = g_maxParticles * 6 * sizeof(UINT);
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA data;

	UINT* indices = new UINT[g_maxParticles * 6];
	data.pSysMem = indices;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	UINT base = 0;
	for (int i = 0; i < g_maxParticles; i++)
	{
		indices[0] = base + 0;
		indices[1] = base + 1;
		indices[2] = base + 2;

		indices[3] = base + 2;
		indices[4] = base + 1;
		indices[5] = base + 3;

		base += 4;
		indices += 6;
	}

	DevicePtr->CreateBuffer(&desc, &data, &Data->IndexBuffer);

	delete[] data.pSysMem;

	// Create a blend state for compositing the particles onto the render target
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	DevicePtr->CreateBlendState(&blendDesc, &Data->CompositeBlendState);

	FillRandomTexture();

	// Create constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	cbDesc.ByteWidth = sizeof(Data->GlobalConstantBuffer);
	DevicePtr->CreateBuffer(&cbDesc, nullptr, &Data->PerFrameConstantBuffer);
}

void AMDGPUParticleEffect::FillRandomTexture()
{

	float* values = new float[1024 * 1024 * 4];
	float* ptr = values;
	for (UINT i = 0; i < 1024 * 1024; i++)
	{
		ptr[0] = RandomVariance(0.0f, 1.0f);
		ptr[1] = RandomVariance(0.0f, 1.0f);
		ptr[2] = RandomVariance(0.0f, 1.0f);
		ptr[3] = RandomVariance(0.0f, 1.0f);
		ptr += 4;
	}

	Data->RandomTexture = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_A32R32G32B32F, CC3DTextureRHI::OT_NONE, 1024, 1024, values, 1024 * 16, false, false);
	delete[] values;

}

void AMDGPUParticleEffect::PopulateEmitter( float frameTime)
{
	if (Data->SparksEmissionRate.m_ParticlesPerSecond > 0.0f)
	{
		Data->SparksEmissionRate.m_Accumulation += Data->SparksEmissionRate.m_ParticlesPerSecond * frameTime;

		if (Data->SparksEmissionRate.m_Accumulation > 1.0f)
		{
			float integerPart = 0.0f;
			float fraction = modf(Data->SparksEmissionRate.m_Accumulation, &integerPart);

			Data->SparksEmitter.m_NumToEmit = (int)integerPart;
			Data->SparksEmissionRate.m_Accumulation = fraction;
		}
	}
}

void AMDGPUParticleEffect::Emit()
{
	// Set resources but don't reset any atomic counters
	ID3D11UnorderedAccessView* uavs[] = { Data->ParticleBufferA_UAV, Data->ParticleBufferB_UAV, Data->DeadListUAV };
	UINT initialCounts[] = { (UINT)-1, (UINT)-1, (UINT)-1 };
	DeviceContextPtr->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, initialCounts);

	ID3D11Buffer* buffers[] = { Data->EmitterConstantBuf, Data->DeadListConstantBuffer };
	DeviceContextPtr->CSSetConstantBuffers(1, ARRAYSIZE(buffers), buffers);

	ID3D11ShaderResourceView* srvs[] = { RHIResourceCast(Data->RandomTexture.get())->GetSRV() };
	DeviceContextPtr->CSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

	Data->CSEmit->UseShader();

	// Run CS for each emitter
	const EmitterParams& emitter = Data->SparksEmitter;

	if (emitter.m_NumToEmit > 0)
	{
		// Update the emitter constant buffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		DeviceContextPtr->Map(Data->EmitterConstantBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		EmitterConstantBuffer* constants = (EmitterConstantBuffer*)MappedResource.pData;
		constants->m_EmitterPosition = emitter.m_Position;
		constants->m_EmitterVelocity = emitter.m_Velocity;
		constants->m_MaxParticlesThisFrame = emitter.m_NumToEmit;
		constants->m_ParticleLifeSpan = emitter.m_ParticleLifeSpan;
		constants->m_StartSize = emitter.m_StartSize;
		constants->m_EndSize = emitter.m_EndSize;
		constants->m_PositionVariance = emitter.m_PositionVariance;
		constants->m_VelocityVariance = emitter.m_VelocityVariance;
		constants->m_Mass = emitter.m_Mass;
		constants->m_Index = 0;
		constants->m_Streaks = emitter.m_Streaks ? 1 : 0;
		constants->m_TextureIndex = emitter.m_TextureIndex;
		DeviceContextPtr->Unmap(Data->EmitterConstantBuf, 0);

		// Copy the current number of dead particles into a CB so we know how many new particles are available to be spawned
		DeviceContextPtr->CopyStructureCount(Data->DeadListConstantBuffer, 0, Data->DeadListUAV);

		// Dispatch enough thread groups to spawn the requested particles
		int numThreadGroups = align(emitter.m_NumToEmit, 1024) / 1024;
		DeviceContextPtr->Dispatch(numThreadGroups, 1, 1);
	}
}

void AMDGPUParticleEffect::Simulate(BaseRenderParam& Scene)
{
	auto depthSRV = Data->ParticaleDepth->GetDepthSRV();

	// Set the UAVs and reset the alive index buffer's counter
	ID3D11UnorderedAccessView* uavs[] = { Data->ParticleBufferA_UAV,  Data->ParticleBufferB_UAV,  Data->DeadListUAV,  Data->AliveIndexBufferUAV,  Data->ViewSpaceParticlePositionsUAV,  Data->MaxRadiusBufferUAV,  Data->IndirectDrawArgsBufferUAV };
	UINT initialCounts[] = { (UINT)-1, (UINT)-1, (UINT)-1, 0, (UINT)-1, (UINT)-1, (UINT)-1 };

	DeviceContextPtr->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, initialCounts);

	// Bind the depth buffer as a texture for doing collision detection and response
	ID3D11ShaderResourceView* srvs[] = { depthSRV };
	DeviceContextPtr->CSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

	Data->CSSimulate->UseShader();
	DeviceContextPtr->Dispatch(align(g_maxParticles, 256) / 256, 1, 1);

	ZeroMemory(srvs, sizeof(srvs));
	DeviceContextPtr->CSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

	ZeroMemory(uavs, sizeof(uavs));
	DeviceContextPtr->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, nullptr);
}
