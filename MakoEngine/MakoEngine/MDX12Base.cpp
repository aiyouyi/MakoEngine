#include "MDX12Base.h"
#include< iostream >
#include <D3Dcompiler.h>

HRESULT debughr01;



bool MDX12Base::InitEnviroment()
{
	CreateDXGIFactory();
	ChooseAdapter();
	CreateDevice();
	CreateFence();
	GetDescriptorSize();
	SetMSAA();
	CreateCommandObject();
	CreateSwapChain();
	CreateDescriptorHeap();
	CreateRTV();
	CreateDSV();
	FlushCmdQueue();
	return false;
}

winrt::com_ptr<IDXGIFactory4> MDX12Base::CreateDXGIFactory()
{
	UINT DxgiFactoryFlags = 0;
	DXGIFactory = nullptr;
	CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&DXGIFactory));
	return DXGIFactory;
}

winrt::com_ptr<IDXGIAdapter1> MDX12Base::ChooseAdapter()
{
	int BestAdapterIndex = -1;
	SIZE_T MaxGPUMemory = 0;
	for (UINT AdapterIndex = 0; DXGI_ERROR_NOT_FOUND != DXGIFactory->EnumAdapters1(AdapterIndex, DXGIAdapter.put()); ++AdapterIndex)
	{
		DXGI_ADAPTER_DESC1 Desc;
		DXGIAdapter->GetDesc1(&Desc);
		if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			DXGIAdapter = nullptr;
			continue;
		}
		if (SUCCEEDED(D3D12CreateDevice(DXGIAdapter.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
		{
			float SizeGB = 1 << 30;
			std::wcout << "***Adapter: " << Desc.Description << ": " << Desc.DedicatedVideoMemory / SizeGB << "G" << std::endl;
			if (Desc.DedicatedVideoMemory > MaxGPUMemory)
			{
				BestAdapterIndex = AdapterIndex;
				MaxGPUMemory = Desc.DedicatedVideoMemory;
			}
		}
		DXGIAdapter = nullptr;
	}
	if (BestAdapterIndex < 0)
	{
		return nullptr;
	}
	DXGIAdapter = nullptr;
	DXGIFactory->EnumAdapters1(BestAdapterIndex, DXGIAdapter.put());
	DXGI_ADAPTER_DESC1 Desc2;
	DXGIAdapter->GetDesc1(&Desc2);
	return DXGIAdapter;
}

winrt::com_ptr<ID3D12Device> MDX12Base::CreateDevice()
{
	debughr01 = D3D12CreateDevice(DXGIAdapter.get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&D3D12Device));
	debughr01 = D3D12Device->SetName(L"D3D12 Device");
	return D3D12Device;
}

winrt::com_ptr<ID3D12Fence> MDX12Base::CreateFence()
{
	D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&D3D12Fence));
	return  D3D12Fence;
}

void MDX12Base::GetDescriptorSize()
{
	RtvDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	Cbv_Srv_UavDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}


void MDX12Base::SetMSAA()
{
	MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MsaaQualityLevels.SampleCount = 1;
	MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MsaaQualityLevels.NumQualityLevels = 0;
	D3D12Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MsaaQualityLevels, sizeof(MsaaQualityLevels));
	assert(MsaaQualityLevels.NumQualityLevels > 0);
}

void MDX12Base::CreateCommandObject()
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	D3D12Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&D3D12CommandQueue));
	D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&D3D12CommandAllocator));
	D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12CommandAllocator.get(), nullptr, IID_PPV_ARGS(&D3D12GraphicsCommandList));
	D3D12GraphicsCommandList->Close();
}




void MDX12Base::CreateSwapChain()
{
	winrt::com_ptr<IDXGISwapChain>& SwapChain = DXGISwapChain;
	SwapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = 1280;
	SwapChainDesc.BufferDesc.Height = 720;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = WindowHwnd;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	DXGIFactory->CreateSwapChain(D3D12CommandQueue.get(), &SwapChainDesc, SwapChain.put());

	for (int i = 0; i < 2; ++i)
		SwapChainBuffer[i] = nullptr;
	D3D12ResourceDepthStencilBuffer = nullptr;

	// Resize the swap chain.
	(DXGISwapChain->ResizeBuffers(
		2,
		1280, 720,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
}

void MDX12Base::CreateDescriptorHeap()
{
	//RTV
	D3D12_DESCRIPTOR_HEAP_DESC RtvDescriptorHeapDesc;
	RtvDescriptorHeapDesc.NumDescriptors = 2;
	RtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvDescriptorHeapDesc.NodeMask = 0;
	D3D12Device->CreateDescriptorHeap(&RtvDescriptorHeapDesc, IID_PPV_ARGS(&D3D12DescriptorHeapRTV));
	//DSV
	D3D12_DESCRIPTOR_HEAP_DESC DsvDescriptorHeapDesc;
	DsvDescriptorHeapDesc.NumDescriptors = 1;
	DsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvDescriptorHeapDesc.NodeMask = 0;
	D3D12Device->CreateDescriptorHeap(&DsvDescriptorHeapDesc, IID_PPV_ARGS(D3D12DescriptorHeapDSV.put()));
}

void MDX12Base::CreateRTV()
{
	FlushCmdQueue();

	(D3D12GraphicsCommandList->Reset(D3D12CommandAllocator.get(), nullptr));
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(D3D12DescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < 2; i++)
	{
		DXGISwapChain->GetBuffer(i, __uuidof(SwapChainBuffer[i]), SwapChainBuffer[i].put_void());
		D3D12Device->CreateRenderTargetView(SwapChainBuffer[i].get(), nullptr, RtvHeapHandle);
		RtvHeapHandle.Offset(1, RtvDescriptorSize);
	}
}

void MDX12Base::CreateDSV()
{
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
	winrt::com_ptr<ID3D12Resource>& DepthStencilBuffer = D3D12ResourceDepthStencilBuffer;
	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;
	debughr01 = D3D12Device->CreateCommittedResource(&HeapProps,	//堆类型为默认堆（不能写入）
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
	debughr01 = (D3D12Device->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&D3D12CbvHeap)));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	D3D12Device->CreateDepthStencilView(DepthStencilBuffer.get(), &dsvDesc, D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart());

}

void MDX12Base::FlushCmdQueue()
{
	static int mCurrentFence = 0;
	mCurrentFence++;
	D3D12CommandQueue->Signal(D3D12Fence.get(), mCurrentFence);
	if (D3D12Fence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE EventHandle = CreateEvent(nullptr, false, false, L"FenceSetDone");
		D3D12Fence->SetEventOnCompletion(mCurrentFence, EventHandle);
		WaitForSingleObject(EventHandle, INFINITE);

		CloseHandle(EventHandle);
	}
}

winrt::com_ptr<ID3D12Resource> MDX12Base::CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, winrt::com_ptr<ID3D12Resource>& UploadBuffer)
{
	winrt::com_ptr<ID3D12Resource> defaultBuffer;

	// Create the actual default buffer resource.
	auto Pro = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto Des = CD3DX12_RESOURCE_DESC::Buffer(ByteSize);
	(Device->CreateCommittedResource(
		&Pro,
		D3D12_HEAP_FLAG_NONE,
		&Des,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.put())));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	auto Pro00 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto des00 = CD3DX12_RESOURCE_DESC::Buffer(ByteSize);
	(Device->CreateCommittedResource(
		&Pro00,
		D3D12_HEAP_FLAG_NONE,
		&des00,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(UploadBuffer.put())));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = InitData;
	subResourceData.RowPitch = ByteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	auto Bar = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	CmdList->ResourceBarrier(1, &Bar);
	UpdateSubresources<1>(CmdList, defaultBuffer.get(), UploadBuffer.get(), 0, 0, 1, &subResourceData);

	auto Bar2 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	CmdList->ResourceBarrier(1, &Bar2);

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.


	return defaultBuffer;
}

void MDX12Base::SwapBuffer()
{
	SwapChainBufferIndex = (SwapChainBufferIndex + 1) % 2;
}

D3D12_CPU_DESCRIPTOR_HANDLE MDX12Base::CurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12DescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferIndex, RtvDescriptorSize);
}

ID3D12Resource* MDX12Base::CurrentBackBuffer()
{
	return SwapChainBuffer[SwapChainBufferIndex].get();
}

D3D12_CPU_DESCRIPTOR_HANDLE MDX12Base::DepthStencilView()
{
	return D3D12DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart();
}

winrt::com_ptr<ID3DBlob> MDX12Base::CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	//compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	winrt::com_ptr<ID3DBlob> byteCode = nullptr;
	winrt::com_ptr<ID3DBlob> errors = nullptr;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, byteCode.put(), errors.put());

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());
	return byteCode;
}
