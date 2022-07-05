#include "MDX12Base.h"
#include< iostream >
#include <D3Dcompiler.h>

HRESULT debughr01;

bool MDX12Base::InitEnviroment()
{
	CreateDXGIFactory();
	ChooseAdapter(DXGIFactory);
	CreateDevice(DXGIAdapter);
	CreateFence(D3D12Device);
	GetDescriptorSize(D3D12Device);
	SetMSAA(D3D12Device);
	CreateCommandObject(D3D12Device);	
	CreateSwapChain(D3D12CommandQueue, DXGIFactory, WindowHwnd);
	CreateDescriptorHeap(D3D12Device);	
	CreateRTV(D3D12Device, D3D12DescriptorHeapRTV, DXGISwapChain, RtvDescriptorSize);
	CreateDSV(D3D12Device, MsaaQualityLevels, D3D12DescriptorHeapDSV);	
	return false;
}

winrt::com_ptr<IDXGIFactory4> MDX12Base::CreateDXGIFactory()
{
	UINT DxgiFactoryFlags = 0;
	DXGIFactory = nullptr;
	CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&DXGIFactory));
	return DXGIFactory;
}

winrt::com_ptr<IDXGIAdapter1> MDX12Base::ChooseAdapter(winrt::com_ptr<IDXGIFactory4>& Factory)
{
	winrt::com_ptr<IDXGIAdapter1>& Adapter= DXGIAdapter;
	int BestAdapterIndex = -1;
	SIZE_T MaxGPUMemory = 0;
	for (UINT AdapterIndex = 0; DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters1(AdapterIndex, Adapter.put()); ++AdapterIndex)
	{
		DXGI_ADAPTER_DESC1 Desc;
		Adapter->GetDesc1(&Desc);
		if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			Adapter = nullptr;
			continue;
		}
		if (SUCCEEDED(D3D12CreateDevice(Adapter.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
		{
			float SizeGB = 1 << 30;
			std::wcout << "***Adapter: " << Desc.Description << ": " << Desc.DedicatedVideoMemory / SizeGB << "G" << std::endl;
			if (Desc.DedicatedVideoMemory > MaxGPUMemory)
			{
				BestAdapterIndex = AdapterIndex;
				MaxGPUMemory = Desc.DedicatedVideoMemory;
			}
		}
		Adapter = nullptr;
	}
	if (BestAdapterIndex < 0)
	{
		return nullptr;
	}
	Adapter = nullptr;
	Factory->EnumAdapters1(BestAdapterIndex, Adapter.put());
	DXGI_ADAPTER_DESC1 Desc2;
	Adapter->GetDesc1(&Desc2);
	return DXGIAdapter;
}

winrt::com_ptr<ID3D12Device> MDX12Base::CreateDevice(winrt::com_ptr<IDXGIAdapter1>& Adapter)
{
	winrt::com_ptr<ID3D12Device>& Device= D3D12Device;
	debughr01=D3D12CreateDevice(Adapter.get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device));
	debughr01=Device->SetName(L"D3D12 Device");
	return Device;
}

winrt::com_ptr<ID3D12Fence> MDX12Base::CreateFence(winrt::com_ptr<ID3D12Device> D3dDevice)
{
	winrt::com_ptr<ID3D12Fence>& Fence= D3D12Fence;
	D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
	return  Fence;
}

void MDX12Base::GetDescriptorSize(winrt::com_ptr<ID3D12Device> D3dDevice)
{
	RtvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	Cbv_Srv_UavDescriptorSize = D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}


void MDX12Base::SetMSAA(winrt::com_ptr<ID3D12Device> D3dDevice)
{
	MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MsaaQualityLevels.SampleCount = 1;
	MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MsaaQualityLevels.NumQualityLevels = 0;
	D3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MsaaQualityLevels, sizeof(MsaaQualityLevels));
	assert(MsaaQualityLevels.NumQualityLevels > 0);
}

void MDX12Base::CreateCommandObject(winrt::com_ptr<ID3D12Device> D3dDevice)
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	winrt::com_ptr<ID3D12CommandQueue>& CmdQueue = D3D12CommandQueue;
	D3dDevice->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CmdQueue));
	winrt::com_ptr<ID3D12CommandAllocator>& CmdAllocator = D3D12CommandAllocator;
	D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdAllocator));
	winrt::com_ptr<ID3D12GraphicsCommandList>& CmdList = D3D12GraphicsCommandList;
	D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CmdAllocator.get(), nullptr, IID_PPV_ARGS(&CmdList));
	CmdList->Close();
}




void MDX12Base::CreateSwapChain(winrt::com_ptr<ID3D12CommandQueue> CmdQueue, winrt::com_ptr<IDXGIFactory4> DxgiFactory, HWND Hwnd)
{
	winrt::com_ptr<IDXGISwapChain>& SwapChain= DXGISwapChain;
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
	SwapChainDesc.OutputWindow = Hwnd;	
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;	
	SwapChainDesc.Windowed = true;	
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	DxgiFactory->CreateSwapChain(CmdQueue.get(), &SwapChainDesc, SwapChain.put());

	for (int i = 0; i < 2; ++i)
		mSwapChainBuffer[i] = nullptr;
	D3D12ResourceDepthStencilBuffer = nullptr;

	// Resize the swap chain.
	(DXGISwapChain->ResizeBuffers(
		2,
		1280, 720,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
}

void MDX12Base::CreateDescriptorHeap(winrt::com_ptr<ID3D12Device> D3dDevice)
{
	//RTV
	D3D12_DESCRIPTOR_HEAP_DESC RtvDescriptorHeapDesc;
	RtvDescriptorHeapDesc.NumDescriptors = 2;
	RtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvDescriptorHeapDesc.NodeMask = 0;
	winrt::com_ptr<ID3D12DescriptorHeap>& RtvHeap= D3D12DescriptorHeapRTV;
	D3dDevice->CreateDescriptorHeap(&RtvDescriptorHeapDesc, IID_PPV_ARGS(&RtvHeap));
	//DSV
	D3D12_DESCRIPTOR_HEAP_DESC DsvDescriptorHeapDesc;
	DsvDescriptorHeapDesc.NumDescriptors = 1;
	DsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvDescriptorHeapDesc.NodeMask = 0;
	winrt::com_ptr<ID3D12DescriptorHeap> &DsvHeap= D3D12DescriptorHeapDSV;
	D3dDevice->CreateDescriptorHeap(&DsvDescriptorHeapDesc, IID_PPV_ARGS(D3D12DescriptorHeapDSV.put()));
}

void MDX12Base::CreateRTV(winrt::com_ptr<ID3D12Device> D3dDevice, winrt::com_ptr<ID3D12DescriptorHeap>& RtvHeap, winrt::com_ptr<IDXGISwapChain> SwapChain, UINT RtvSize)
{
	FlushCmdQueue(D3D12Fence, D3D12CommandQueue);

	(D3D12GraphicsCommandList->Reset(D3D12CommandAllocator.get(), nullptr));
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < 2; i++)
	{
		SwapChain->GetBuffer(i, __uuidof(mSwapChainBuffer[i]), mSwapChainBuffer[i].put_void());
		D3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].get(), nullptr, RtvHeapHandle);
		RtvHeapHandle.Offset(1, RtvSize);
	}
}

void MDX12Base::CreateDSV(winrt::com_ptr<ID3D12Device>& D3dDevice, D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevel, winrt::com_ptr<ID3D12DescriptorHeap>& DsvHeap)
{
	D3D12_RESOURCE_DESC DsvResourceDesc;
	DsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DsvResourceDesc.Alignment = 0;
	DsvResourceDesc.Width = 1280;
	DsvResourceDesc.Height = 720;
	DsvResourceDesc.DepthOrArraySize = 1;
	DsvResourceDesc.MipLevels = 1;
	DsvResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	DsvResourceDesc.SampleDesc.Count =  1;
	DsvResourceDesc.SampleDesc.Quality = 0;
	DsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	CD3DX12_CLEAR_VALUE OptClear;
	OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	OptClear.DepthStencil.Depth = 1;	
	OptClear.DepthStencil.Stencil = 0;	
	winrt::com_ptr<ID3D12Resource>& DepthStencilBuffer= D3D12ResourceDepthStencilBuffer;
	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;
	debughr01 =D3dDevice->CreateCommittedResource(&HeapProps,	//堆类型为默认堆（不能写入）
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
	D3dDevice->CreateDepthStencilView(DepthStencilBuffer.get(), &dsvDesc, DsvHeap->GetCPUDescriptorHandleForHeapStart());

}

void MDX12Base::FlushCmdQueue(winrt::com_ptr<ID3D12Fence> Fence, winrt::com_ptr<ID3D12CommandQueue> CmdQueue)
{
	static int mCurrentFence = 0;
	mCurrentFence++;
	CmdQueue->Signal(Fence.get(), mCurrentFence);	
	if (Fence->GetCompletedValue() < mCurrentFence)	
	{
		HANDLE EventHandle = CreateEvent(nullptr, false, false, L"FenceSetDone");	
		Fence->SetEventOnCompletion(mCurrentFence, EventHandle);
		WaitForSingleObject(EventHandle, INFINITE);
												  
		CloseHandle(EventHandle);
	}
}

winrt::com_ptr<ID3D12Resource> MDX12Base::CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, winrt::com_ptr<ID3D12Resource>& UploadBuffer)
{
	winrt::com_ptr<ID3D12Resource> DefaultBuffer = nullptr;
	CD3DX12_HEAP_PROPERTIES HeapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto BufferSize= CD3DX12_RESOURCE_DESC::Buffer(ByteSize);
	Device->CreateCommittedResource(&HeapDefault, D3D12_HEAP_FLAG_NONE, &BufferSize, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(DefaultBuffer.put()));
	CD3DX12_HEAP_PROPERTIES HeapUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	Device->CreateCommittedResource(&HeapUpload, D3D12_HEAP_FLAG_NONE, &BufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(UploadBuffer.put()));

	D3D12_SUBRESOURCE_DATA SubResourceData = {};
	SubResourceData.pData = InitData;
	SubResourceData.RowPitch = ByteSize;
	SubResourceData.SlicePitch = SubResourceData.RowPitch;

	auto DefaultBarrier = CD3DX12_RESOURCE_BARRIER::Transition(DefaultBuffer.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	CmdList->ResourceBarrier(1, &DefaultBarrier);
	UpdateSubresources<1>(CmdList, DefaultBuffer.get(), UploadBuffer.get(), 0, 0, 1, &SubResourceData);
	auto UploadBarrier = CD3DX12_RESOURCE_BARRIER::Transition(DefaultBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	CmdList->ResourceBarrier(1, &UploadBarrier);

	return DefaultBuffer;
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

HRESULT MDX12Base::Test()
{
	winrt::com_ptr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	debughr01 = (D3D12Device->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mCbvHeap)));
	return debughr01;
}
