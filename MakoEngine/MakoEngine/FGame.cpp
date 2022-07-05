#include "FGame.h"
#include <DirectXMath.h>
#include"MDX12Base.h"
#include <D3Dcompiler.h>
#include< iostream >
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include<DirectXColors.h>
#include <DirectXCollision.h>

using namespace DirectX;

//const std::wstring BasePath = L"G:\\makoengine\\MakoEngine\\";
const std::wstring BasePath = L"C:\\Mako\\learncode\\makoengine\\MakoEngine\\";

HRESULT debughr;

static UINT CalcConstantBufferByteSize(UINT byteSize)
{
	// Constant buffers must be a multiple of the minimum hardware
	// allocation size (usually 256 bytes).  So round up to nearest
	// multiple of 256.  We do this by adding 255 and then masking off
	// the lower 2 bytes which store all bits < 256.
	// Example: Suppose byteSize = 300.
	// (300 + 255) & ~255
	// 555 & ~255
	// 0x022B & ~0x00ff
	// 0x022B & 0xff00
	// 0x0200
	// 512
	return (byteSize + 255) & ~255;
}



void FGame::Init()
{
	GLBScene = GLBMeshPool::GetInstance()->GetGLBMesh(BasePath+L"Resource\\DamagedHelmet.glb");

	{
		mCbvHeap = DX12Base->mCbvHeap;
		mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(DX12Base->D3D12Device.get(), 1, true);

		UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
		// Offset to the ith object constant buffer in the buffer.
		int boxCBufIndex = 0;
		cbAddress += boxCBufIndex * objCBByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(ObjectConstants));

		DX12Base->D3D12Device->CreateConstantBufferView(
			&cbvDesc,
			mCbvHeap->GetCPUDescriptorHandleForHeapStart());
	}
	{
		// Shader programs typically require resources as input (constant buffers,
		// textures, samplers).  The root signature defines the resources the shader
		// programs expect.  If we think of the shader programs as a function, and
		// the input resources as function parameters, then the root signature can be
		// thought of as defining the function signature.  

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		// Create a single descriptor table of CBVs.
		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		winrt::com_ptr<ID3DBlob> serializedRootSig = nullptr;
		winrt::com_ptr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.put(), errorBlob.put());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		(DX12Base->D3D12Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)));
	}

	BuildShadersAndInputLayout();
	RootSignature = new FRootSignature(1, 0);
	(*RootSignature)[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,0,1);
	RootSignature->Finalize(DX12Base->D3D12Device,L"FG", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	
	BuildBox();
	BuildPSO();

	auto feg = CD3DX12_RESOURCE_BARRIER::Transition(DX12Base->D3D12ResourceDepthStencilBuffer.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	DX12Base->D3D12GraphicsCommandList->ResourceBarrier(1, &feg);
	(DX12Base->D3D12GraphicsCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { DX12Base->D3D12GraphicsCommandList.get() };
	DX12Base->D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	Flush();
}

void FGame::Tick()
{
	Tick666();
	return;
	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;
	XMFLOAT4X4 mWorld = Identity4x4();
	XMFLOAT4X4 mView = Identity4x4();
	XMFLOAT4X4 mProj = Identity4x4();
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(1280);
	mScreenViewport.Height = static_cast<float>(720);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, 1280, 720 };
	{
		// Convert Spherical to Cartesian coordinates.
		float x = mRadius * sinf(mPhi) * cosf(mTheta);
		float z = mRadius * sinf(mPhi) * sinf(mTheta);
		float y = mRadius * cosf(mPhi);

		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mView, view);

		XMMATRIX world = XMLoadFloat4x4(&mWorld);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX worldViewProj = world * view * proj;

		// Update the constant buffer with the latest worldViewProj matrix.
		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
		mObjectCB->CopyData(0, objConstants);
	}
	//Draw(mTimer);
	static int mCurrBackBuffer = 0;
	{

		auto BBV = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			DX12Base->D3D12DescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(),
			mCurrBackBuffer,
			DX12Base->RtvDescriptorSize);
		auto DSV = DX12Base->D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart();

		auto CBF= DX12Base->mSwapChainBuffer[mCurrBackBuffer].get();
		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished execution on the GPU.
		(DX12Base->D3D12CommandAllocator->Reset());

		// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
		// Reusing the command list reuses memory.
		(DX12Base->D3D12GraphicsCommandList->Reset(DX12Base->D3D12CommandAllocator.get(), PSO.get()));

		DX12Base->D3D12GraphicsCommandList->RSSetViewports(1, &mScreenViewport);
		DX12Base->D3D12GraphicsCommandList->RSSetScissorRects(1, &mScissorRect);

		// Indicate a state transition on the resource usage.
		auto BARR = CD3DX12_RESOURCE_BARRIER::Transition(CBF,
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		DX12Base->D3D12GraphicsCommandList->ResourceBarrier(1, &BARR);

		// Clear the back buffer and depth buffer.
		DX12Base->D3D12GraphicsCommandList->ClearRenderTargetView(BBV, Colors::LightSteelBlue, 0, nullptr);
		DX12Base->D3D12GraphicsCommandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Specify the buffers we are going to render to.
		DX12Base->D3D12GraphicsCommandList->OMSetRenderTargets(1, &BBV, true, &DSV);

		ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.get() };
		DX12Base->D3D12GraphicsCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		DX12Base->D3D12GraphicsCommandList->SetGraphicsRootSignature(mRootSignature.get());

		auto VBV = mBoxGeo->VertexBufferView();
		auto IDV = mBoxGeo->IndexBufferView();
		DX12Base->D3D12GraphicsCommandList->IASetVertexBuffers(0, 1, &VBV);
		DX12Base->D3D12GraphicsCommandList->IASetIndexBuffer(&IDV);
		DX12Base->D3D12GraphicsCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DX12Base->D3D12GraphicsCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

		DX12Base->D3D12GraphicsCommandList->DrawIndexedInstanced(
			mBoxGeo->DrawArgs["box"].IndexCount,
			1, 0, 0, 0);

		// Indicate a state transition on the resource usage.
		auto BAR = CD3DX12_RESOURCE_BARRIER::Transition(CBF,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		DX12Base->D3D12GraphicsCommandList->ResourceBarrier(1, &BAR);

		// Done recording commands.
		(DX12Base->D3D12GraphicsCommandList->Close());

		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { DX12Base->D3D12GraphicsCommandList.get() };
		DX12Base->D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// swap the back and front buffers
		(DX12Base->DXGISwapChain->Present(0, 0));
		mCurrBackBuffer = (mCurrBackBuffer + 1) % 2;

		// Wait until frame commands are complete.  This waiting is inefficient and is
		// done for simplicity.  Later we will show how to organize our rendering code
		// so we do not have to wait per frame.
		Flush();
	}
}

void FGame::BuildPSO()
{
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
		psoDesc.pRootSignature = mRootSignature.get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
			mvsByteCode->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
			mpsByteCode->GetBufferSize()
		};
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		(DX12Base->D3D12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO)));
		return;
	}
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = RootSignature->GetSignature();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	debughr=DX12Base->D3D12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(PSO.put()));
}

void FGame::BuildShadersAndInputLayout()
{
	mvsByteCode = MDX12Base::CompileShader(BasePath+L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = MDX12Base::CompileShader(BasePath+L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FGame::BuildBox()
{
	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";


	debughr=(D3DCreateBlob(vbByteSize, mBoxGeo->VertexBufferCPU.put()));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	debughr=(D3DCreateBlob(ibByteSize, mBoxGeo->IndexBufferCPU.put()));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mBoxGeo->VertexBufferGPU = MDX12Base::CreateDefaultBuffer(DX12Base->D3D12Device.get(),
		DX12Base->D3D12GraphicsCommandList.get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	mBoxGeo->IndexBufferGPU = MDX12Base::CreateDefaultBuffer(DX12Base->D3D12Device.get(),
		DX12Base->D3D12GraphicsCommandList.get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;
}

XMFLOAT4X4 FGame::Identity4x4()
{
	static DirectX::XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}

void FGame::Flush()
{
	mCurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	(DX12Base->D3D12CommandQueue->Signal(DX12Base->D3D12Fence.get(), mCurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (DX12Base->D3D12Fence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		(DX12Base->D3D12Fence->SetEventOnCompletion(mCurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}


ObjectConstants::ObjectConstants()
{
	WorldViewProj = FGame::Identity4x4();
}


////


using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
}
#endif


static DirectX::XMFLOAT4X4 Identity4x4()
{
	static DirectX::XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}


winrt::com_ptr<ID3D12Resource> CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	winrt::com_ptr<ID3D12Resource>& uploadBuffer)
{
	winrt::com_ptr<ID3D12Resource> defaultBuffer;

	// Create the actual default buffer resource.
	auto Pro = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto Des = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
	ThrowIfFailed(device->CreateCommittedResource(
		&Pro,
		D3D12_HEAP_FLAG_NONE,
		&Des,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.put())));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	auto Pro00 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto des00 = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
	ThrowIfFailed(device->CreateCommittedResource(
		&Pro00,
		D3D12_HEAP_FLAG_NONE,
		&des00,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.put())));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	auto Bar = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &Bar);
	UpdateSubresources<1>(cmdList, defaultBuffer.get(), uploadBuffer.get(), 0, 0, 1, &subResourceData);

	auto Bar2 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &Bar2);

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.


	return defaultBuffer;
}


winrt::com_ptr<ID3DBlob> CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target)
{
	UINT compileFlags = 0;

	HRESULT hr = S_OK;

	winrt::com_ptr<ID3DBlob> byteCode = nullptr;
	winrt::com_ptr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, byteCode.put(), errors.put());

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return byteCode;
}


HINSTANCE mhAppInst = nullptr; // application instance handle

bool      mAppPaused = false;  // is the application paused?
bool      mMinimized = false;  // is the application minimized?
bool      mMaximized = false;  // is the application maximized?
bool      mResizing = false;   // are the resize bars being dragged?
bool      mFullscreenState = false;// fullscreen enabled

// Set true to use 4X MSAA (?.1.8).  The default is false.
bool      m4xMsaaState = false;    // 4X MSAA enabled


static const int SwapChainBufferCount = 2;
int mCurrBackBuffer = 0;
winrt::com_ptr<ID3D12Resource> mDepthStencilBuffer;

D3D12_VIEWPORT mScreenViewport;
D3D12_RECT mScissorRect;


// Derived class should set these in derived constructor to customize starting values.
std::wstring mMainWndCaption = L"d3d App";
D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
int mClientWidth = 800;
int mClientHeight = 600;
winrt::com_ptr<ID3D12RootSignature> mRootSignature = nullptr;
winrt::com_ptr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

winrt::com_ptr<ID3DBlob> mvsByteCode = nullptr;
winrt::com_ptr<ID3DBlob> mpsByteCode = nullptr;

std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

winrt::com_ptr<ID3D12PipelineState> mPSO = nullptr;

XMFLOAT4X4 mWorld = Identity4x4();
XMFLOAT4X4 mView = Identity4x4();
XMFLOAT4X4 mProj = Identity4x4();

float mTheta = 1.5f * XM_PI;
float mPhi = XM_PIDIV4;
float mRadius = 5.0f;

POINT mLastMousePos;

D3D12_CPU_DESCRIPTOR_HANDLE FGame::DepthStencilView()
{
	return DX12Base->D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart();
}
float AspectRatio()
{
	return static_cast<float>(mClientWidth) / mClientHeight;
}

ID3D12Resource* FGame::CurrentBackBuffer()
{
	return DX12Base->mSwapChainBuffer[mCurrBackBuffer].get();
}

D3D12_CPU_DESCRIPTOR_HANDLE FGame::CurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		DX12Base->D3D12DescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBuffer,
		DX12Base->RtvDescriptorSize);
}

bool FGame::Initialize()
{
	mhMainWnd = pWindow->WindowHwnd;

	{
		//DX12Base->D3D12Device = DX12Base->D3D12Device;
	}

	// Do the initial resize code.
	//OnResize();
	{
		assert(DX12Base->D3D12Device);
		assert(DX12Base->DXGISwapChain);
		assert(DX12Base->D3D12CommandAllocator);

		// Flush before changing any resources.
		Flush();

		ThrowIfFailed(DX12Base->D3D12GraphicsCommandList->Reset(DX12Base->D3D12CommandAllocator.get(), nullptr));


		mCurrBackBuffer = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(DX12Base->D3D12DescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < 2; i++)
		{
			DX12Base->DXGISwapChain->GetBuffer(i, __uuidof(DX12Base->mSwapChainBuffer[i]), DX12Base->mSwapChainBuffer[i].put_void());
			DX12Base->D3D12Device->CreateRenderTargetView(DX12Base->mSwapChainBuffer[i].get(), nullptr, RtvHeapHandle);
			RtvHeapHandle.Offset(1, DX12Base->RtvDescriptorSize);
		}

		{
			auto D3dDevice = DX12Base->D3D12Device;
			auto D3D12Device = DX12Base->D3D12Device;
			D3D12_RESOURCE_DESC DsvResourceDesc;
			DsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			DsvResourceDesc.Alignment = 0;
			DsvResourceDesc.Width = 1280;
			DsvResourceDesc.Height = 720;
			DsvResourceDesc.DepthOrArraySize = 1;
			DsvResourceDesc.MipLevels = 1;
			DsvResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			DsvResourceDesc.SampleDesc.Count = 1;
			DsvResourceDesc.SampleDesc.Quality = 0;
			DsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			DsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			CD3DX12_CLEAR_VALUE OptClear;
			OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			OptClear.DepthStencil.Depth = 1;
			OptClear.DepthStencil.Stencil = 0;
			winrt::com_ptr<ID3D12Resource>& DepthStencilBuffer = mDepthStencilBuffer;
			D3D12_HEAP_PROPERTIES HeapProps;
			HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			HeapProps.CreationNodeMask = 1;
			HeapProps.VisibleNodeMask = 1;
			D3dDevice->CreateCommittedResource(&HeapProps,	//堆类型为默认堆（不能写入）
				D3D12_HEAP_FLAG_NONE,	//Flag
				&DsvResourceDesc,	//上面定义的DSV资源指针
				D3D12_RESOURCE_STATE_COMMON,	//资源的状态为初始状态
				&OptClear,	//上面定义的优化值指针
				IID_PPV_ARGS(DepthStencilBuffer.put()));	//返回深度模板资源

			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
			cbvHeapDesc.NumDescriptors = 1;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.NodeMask = 0;
			(D3D12Device->CreateDescriptorHeap(&cbvHeapDesc,
				IID_PPV_ARGS(&mCbvHeap)));

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.Texture2D.MipSlice = 0;
			D3dDevice->CreateDepthStencilView(DepthStencilBuffer.get(), &dsvDesc, DX12Base->D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart());

		}

		// Transition the resource from its initial state to be used as a depth buffer.
		auto feg = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		DX12Base->D3D12GraphicsCommandList->ResourceBarrier(1, &feg);

		// Execute the resize commands.
		ThrowIfFailed(DX12Base->D3D12GraphicsCommandList->Close());
		ID3D12CommandList* cmdsLists[] = { DX12Base->D3D12GraphicsCommandList.get() };
		DX12Base->D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until resize is complete.
		Flush();

		// Update the viewport transform to cover the client area.
		mScreenViewport.TopLeftX = 0;
		mScreenViewport.TopLeftY = 0;
		mScreenViewport.Width = static_cast<float>(mClientWidth);
		mScreenViewport.Height = static_cast<float>(mClientHeight);
		mScreenViewport.MinDepth = 0.0f;
		mScreenViewport.MaxDepth = 1.0f;

		mScissorRect = { 0, 0, mClientWidth, mClientHeight };
	}
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(DX12Base->D3D12GraphicsCommandList->Reset(DX12Base->D3D12CommandAllocator.get(), nullptr));


	//BuildConstantBuffers();
	{
		mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(DX12Base->D3D12Device.get(), 1, true);

		UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
		// Offset to the ith object constant buffer in the buffer.
		int boxCBufIndex = 0;
		cbAddress += boxCBufIndex * objCBByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(ObjectConstants));

		DX12Base->D3D12Device->CreateConstantBufferView(
			&cbvDesc,
			mCbvHeap->GetCPUDescriptorHandleForHeapStart());
	}
	//BuildRootSignature();
	{
		// Shader programs typically require resources as input (constant buffers,
		// textures, samplers).  The root signature defines the resources the shader
		// programs expect.  If we think of the shader programs as a function, and
		// the input resources as function parameters, then the root signature can be
		// thought of as defining the function signature.  

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		// Create a single descriptor table of CBVs.
		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		winrt::com_ptr<ID3DBlob> serializedRootSig = nullptr;
		winrt::com_ptr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.put(), errorBlob.put());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(DX12Base->D3D12Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)));
	}
	//BuildShadersAndInputLayout();
	{
		HRESULT hr = S_OK;

		mvsByteCode = CompileShader(BasePath + L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
		mpsByteCode = CompileShader(BasePath + L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

		mInputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}
	//BuildBoxGeometry();
	{
		std::array<Vertex, 8> vertices =
		{
			Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
			Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
			Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
			Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
			Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
			Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
			Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
			Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
		};

		std::array<std::uint16_t, 36> indices =
		{
			// front face
			0, 1, 2,
			0, 2, 3,

			// back face
			4, 6, 5,
			4, 7, 6,

			// left face
			4, 5, 1,
			4, 1, 0,

			// right face
			3, 2, 6,
			3, 6, 7,

			// top face
			1, 5, 6,
			1, 6, 2,

			// bottom face
			4, 0, 3,
			4, 3, 7
		};

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		mBoxGeo = std::make_unique<MeshGeometry>();
		mBoxGeo->Name = "boxGeo";

		ThrowIfFailed(D3DCreateBlob(vbByteSize, mBoxGeo->VertexBufferCPU.put()));
		CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, mBoxGeo->IndexBufferCPU.put()));
		CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		mBoxGeo->VertexBufferGPU = CreateDefaultBuffer(DX12Base->D3D12Device.get(),
			DX12Base->D3D12GraphicsCommandList.get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

		mBoxGeo->IndexBufferGPU = CreateDefaultBuffer(DX12Base->D3D12Device.get(),
			DX12Base->D3D12GraphicsCommandList.get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

		mBoxGeo->VertexByteStride = sizeof(Vertex);
		mBoxGeo->VertexBufferByteSize = vbByteSize;
		mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
		mBoxGeo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		mBoxGeo->DrawArgs["box"] = submesh;
	}
	//BuildPSO();
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
		psoDesc.pRootSignature = mRootSignature.get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
			mvsByteCode->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
			mpsByteCode->GetBufferSize()
		};
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = mBackBufferFormat;
		psoDesc.SampleDesc.Count =  1;
		psoDesc.SampleDesc.Quality =  0;
		psoDesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(DX12Base->D3D12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
	}

	// Execute the initialization commands.
	ThrowIfFailed(DX12Base->D3D12GraphicsCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { DX12Base->D3D12GraphicsCommandList.get() };
	DX12Base->D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	Flush();
	
	return true;
}

void FGame::Tick666()
{
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		//Update(mTimer);
		{
			// Convert Spherical to Cartesian coordinates.
			float x = mRadius * sinf(mPhi) * cosf(mTheta);
			float z = mRadius * sinf(mPhi) * sinf(mTheta);
			float y = mRadius * cosf(mPhi);

			// Build the view matrix.
			XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
			XMVECTOR target = XMVectorZero();
			XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
			XMStoreFloat4x4(&mView, view);

			XMMATRIX world = XMLoadFloat4x4(&mWorld);
			XMMATRIX proj = XMLoadFloat4x4(&mProj);
			XMMATRIX worldViewProj = world * view * proj;

			// Update the constant buffer with the latest worldViewProj matrix.
			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
			mObjectCB->CopyData(0, objConstants);
		}
		//Draw(mTimer);
		{
			auto BBV = CurrentBackBufferView();
			auto DSV = DepthStencilView();
			// Reuse the memory associated with command recording.
			// We can only reset when the associated command lists have finished execution on the GPU.
			ThrowIfFailed(DX12Base->D3D12CommandAllocator->Reset());

			// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
			// Reusing the command list reuses memory.
			ThrowIfFailed(DX12Base->D3D12GraphicsCommandList->Reset(DX12Base->D3D12CommandAllocator.get(), mPSO.get()));

			DX12Base->D3D12GraphicsCommandList->RSSetViewports(1, &mScreenViewport);
			DX12Base->D3D12GraphicsCommandList->RSSetScissorRects(1, &mScissorRect);

			// Indicate a state transition on the resource usage.
			auto BARR = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			DX12Base->D3D12GraphicsCommandList->ResourceBarrier(1, &BARR);

			// Clear the back buffer and depth buffer.
			DX12Base->D3D12GraphicsCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
			DX12Base->D3D12GraphicsCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			// Specify the buffers we are going to render to.
			DX12Base->D3D12GraphicsCommandList->OMSetRenderTargets(1, &BBV, true, &DSV);

			ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.get() };
			DX12Base->D3D12GraphicsCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

			DX12Base->D3D12GraphicsCommandList->SetGraphicsRootSignature(mRootSignature.get());

			auto VBV = mBoxGeo->VertexBufferView();
			auto IDV = mBoxGeo->IndexBufferView();
			DX12Base->D3D12GraphicsCommandList->IASetVertexBuffers(0, 1, &VBV);
			DX12Base->D3D12GraphicsCommandList->IASetIndexBuffer(&IDV);
			DX12Base->D3D12GraphicsCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			DX12Base->D3D12GraphicsCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

			DX12Base->D3D12GraphicsCommandList->DrawIndexedInstanced(
				mBoxGeo->DrawArgs["box"].IndexCount,
				1, 0, 0, 0);

			// Indicate a state transition on the resource usage.
			auto BAR = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			DX12Base->D3D12GraphicsCommandList->ResourceBarrier(1, &BAR);

			// Done recording commands.
			ThrowIfFailed(DX12Base->D3D12GraphicsCommandList->Close());

			// Add the command list to the queue for execution.
			ID3D12CommandList* cmdsLists[] = { DX12Base->D3D12GraphicsCommandList.get() };
			DX12Base->D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

			// swap the back and front buffers
			ThrowIfFailed(DX12Base->DXGISwapChain->Present(0, 0));
			mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

			// Wait until frame commands are complete.  This waiting is inefficient and is
			// done for simplicity.  Later we will show how to organize our rendering code
			// so we do not have to wait per frame.
			Flush();
		}
		Sleep(100);
	}
}



FGame::FGame(MWindow* pMWindow)
{
	pWindow = pMWindow;
	
	DX12Base = new MDX12Base();
	DX12Base->WindowHwnd = pMWindow->WindowHwnd;
	DX12Base->InitEnviroment();
/*

	auto& DX12Base->D3D12GraphicsCommandList = DX12Base->D3D12GraphicsCommandList;
	auto feg = CD3DX12_RESOURCE_BARRIER::Transition(DX12Base->D3D12ResourceDepthStencilBuffer.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	DX12Base->D3D12GraphicsCommandList->ResourceBarrier(1, &feg);
	(DX12Base->D3D12GraphicsCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { DX12Base->D3D12GraphicsCommandList.get() };
	DX12Base->D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	Flush();
	Init();*/


	Initialize();
}