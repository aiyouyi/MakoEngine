#include "FGame.h"
#include <DirectXMath.h>
#include"MDX12Base.h"
#include<DirectXColors.h>
#include <DirectXCollision.h>

using namespace DirectX;

const std::wstring BasePath = L"G:\\makoengine\\MakoEngine\\";
//const std::wstring BasePath = L"C:\\Mako\\learncode\\makoengine\\MakoEngine\\";

HRESULT debughr;

FGame::FGame(MWindow* pMWindow)
{
	pWindow = pMWindow;
	DX12Base = new MDX12Base();
	DX12Base->WindowHwnd = pMWindow->WindowHwnd;
	DX12Base->InitEnviroment();
	Init();
	
}

void FGame::Init()
{
	GLBScene = GLBMeshPool::GetInstance()->GetGLBMesh(BasePath+L"Resource\\DamagedHelmet.glb");
	BuildShadersAndInputLayout();
	RootSignature = new FRootSignature(1, 0);
	(*RootSignature)[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,0,1);
	RootSignature->Finalize(DX12Base->D3D12Device,L"FG", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	BuildPSO();
	BuildBox();
	//DX12Base->Test();

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	(DX12Base->D3D12Device->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mCbvHeap)));

	auto& mCommandList = DX12Base->D3D12GraphicsCommandList;
	(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.get() };
	DX12Base->D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	Flush();
}

void FGame::Tick()
{
	auto& mCommandList = DX12Base->D3D12GraphicsCommandList;
	//debughr=DX12Base->D3D12CommandAllocator->Reset();
	//debughr=mCommandList->Reset(DX12Base->D3D12CommandAllocator.get(), PSO.get());
	D3D12_VIEWPORT mScreenViewport;
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = 1280;
	mScreenViewport.Height = 720;
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;
	mCommandList->RSSetViewports(1, &mScreenViewport);

	D3D12_RECT mScissorRect;
	mScissorRect = { 0, 0, 1280, 720 };
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	//DX12Base->D3D12GraphicsCommandList->ClearDepthStencilView(DX12Base->D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	// Indicate a state transition on the resource usage.
	static int index = 0;
	
	auto SCB = CD3DX12_RESOURCE_BARRIER::Transition(DX12Base->mSwapChainBuffer[index].get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &SCB);

	DX12Base->D3D12GraphicsCommandList->ClearDepthStencilView(DX12Base->D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	// Clear the back buffer and depth buffer.
	CD3DX12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView(
		DX12Base->D3D12DescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(),
		index,
		DX12Base->RtvDescriptorSize);
	//DX12Base->Test();
	static XMVECTORF32 color = DirectX::Colors::LightSteelBlue;
	
	//mCommandList->ClearRenderTargetView(CurrentBackBufferView, color, 0, nullptr);
	//DX12Base->Test();
	//mCommandList->ClearDepthStencilView(DX12Base->D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	//DX12Base->Test();
	// Specify the buffers we are going to render to.
	auto DSYStart = DX12Base->D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart();
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView, true, &DSYStart);
	mCommandList->ClearRenderTargetView(CurrentBackBufferView, DirectX::Colors::LightSteelBlue, 0, nullptr);
	/*ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(RootSignature->GetSignature());

	auto VBV = mBoxGeo->VertexBufferView();
	mCommandList->IASetVertexBuffers(0, 1, &VBV);

	auto IBV = mBoxGeo->IndexBufferView();
	mCommandList->IASetIndexBuffer(&IBV);
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	mCommandList->DrawIndexedInstanced(mBoxGeo->DrawArgs["box"].IndexCount, 1, 0, 0, 0);*/

	// Indicate a state transition on the resource usage.
	auto SCB2 = CD3DX12_RESOURCE_BARRIER::Transition(DX12Base->mSwapChainBuffer[index].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &SCB2);

	// Done recording commands.
	(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.get() };
	DX12Base->D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	(DX12Base->DXGISwapChain->Present(0, 0));
	//index = (index + 1) % 2;
	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	Flush();
	//debughr=DX12Base->D3D12CommandAllocator->Reset();
	//debughr=mCommandList->Reset(DX12Base->D3D12CommandAllocator.get(), PSO.get());
}

void FGame::BuildPSO()
{
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

	auto mCommandList = DX12Base->D3D12GraphicsCommandList;

	debughr=(D3DCreateBlob(vbByteSize, mBoxGeo->VertexBufferCPU.put()));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	debughr=(D3DCreateBlob(ibByteSize, mBoxGeo->IndexBufferCPU.put()));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mBoxGeo->VertexBufferGPU = MDX12Base::CreateDefaultBuffer(DX12Base->D3D12Device.get(),
		mCommandList.get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	mBoxGeo->IndexBufferGPU = MDX12Base::CreateDefaultBuffer(DX12Base->D3D12Device.get(),
		mCommandList.get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

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
