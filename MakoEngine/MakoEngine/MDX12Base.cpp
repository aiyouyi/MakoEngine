#include "MDX12Base.h"
#include< iostream >
#include <D3Dcompiler.h>

bool MDX12Base::InitEnviroment()
{
	CreateDXGIFactory();
	ChooseAdapter(DXGIFactory);
	CreateDevice(DXGIAdapter);
	CreateFence(D3D12Device);

	CreateCommandObject(D3D12Device);
	GetDescriptorSize(D3D12Device);
	SetMSAA(D3D12Device);
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
	return DXGIAdapter;
}

winrt::com_ptr<ID3D12Device> MDX12Base::CreateDevice(winrt::com_ptr<IDXGIAdapter1> Adapter)
{
	winrt::com_ptr<ID3D12Device>& Device= D3D12Device;
	D3D12CreateDevice(Adapter.get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device));
	Device->SetName(L"D3D12 Device");
	return Device;
}

winrt::com_ptr<ID3D12Fence> MDX12Base::CreateFence(winrt::com_ptr<ID3D12Device> D3dDevice)
{
	winrt::com_ptr<ID3D12Fence>& Fence= D3D12Fence;
	D3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
	return  Fence;
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

void MDX12Base::CreateSwapChain(winrt::com_ptr<ID3D12CommandQueue> CmdQueue, winrt::com_ptr<IDXGIFactory4> DxgiFactory, HWND Hwnd)
{
	winrt::com_ptr<IDXGISwapChain>& SwapChain= DXGISwapChain;
	SwapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;	//交换链描述结构体
	SwapChainDesc.BufferDesc.Width = 1280;	//缓冲区分辨率的宽度
	SwapChainDesc.BufferDesc.Height = 720;	//缓冲区分辨率的高度
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//缓冲区的显示格式
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;	//刷新率的分子
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;	//刷新率的分母
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	//逐行扫描VS隔行扫描(未指定的)
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	//图像相对屏幕的拉伸（未指定的）
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//将数据渲染至后台缓冲区（即作为渲染目标）
	SwapChainDesc.OutputWindow = Hwnd;	//渲染窗口句柄
	SwapChainDesc.SampleDesc.Count = 1;	//多重采样数量
	SwapChainDesc.SampleDesc.Quality = 0;	//多重采样质量
	SwapChainDesc.Windowed = true;	//是否窗口化
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//固定写法
	SwapChainDesc.BufferCount = 2;	//后台缓冲区数量（双缓冲）
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//自适应窗口模式（自动选择最适于当前窗口尺寸的显示模式）
	DxgiFactory->CreateSwapChain(CmdQueue.get(), &SwapChainDesc, SwapChain.put());
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
	D3dDevice->CreateDescriptorHeap(&DsvDescriptorHeapDesc, IID_PPV_ARGS(&DsvHeap));
}

void MDX12Base::CreateRTV(winrt::com_ptr<ID3D12Device> D3dDevice, winrt::com_ptr<ID3D12DescriptorHeap> RtvHeap, winrt::com_ptr<IDXGISwapChain> SwapChain, UINT RtvSize)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	winrt::com_ptr<ID3D12Resource> SwapChainBuffer[2];
	for (int i = 0; i < 2; i++)
	{
		SwapChain->GetBuffer(i, __uuidof(SwapChainBuffer[i]), SwapChainBuffer[i].put_void());
		D3dDevice->CreateRenderTargetView(SwapChainBuffer[i].get(), nullptr, RtvHeapHandle);
		RtvHeapHandle.Offset(1, RtvSize);
	}
}

void MDX12Base::CreateDSV(winrt::com_ptr<ID3D12Device> D3dDevice, D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevel, winrt::com_ptr<ID3D12DescriptorHeap> DsvHeap)
{
	//在CPU中创建好深度模板数据资源
	D3D12_RESOURCE_DESC DsvResourceDesc;
	DsvResourceDesc.Alignment = 0;	//指定对齐
	DsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	//指定资源维度（类型）为TEXTURE2D
	DsvResourceDesc.DepthOrArraySize = 1;	//纹理深度为1
	DsvResourceDesc.Width = 1280;	//资源宽
	DsvResourceDesc.Height = 720;	//资源高
	DsvResourceDesc.MipLevels = 1;	//MIPMAP层级数量
	DsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	//指定纹理布局（这里不指定）
	DsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//深度模板资源的Flag
	DsvResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//24位深度，8位模板,还有个无类型的格式DXGI_FORMAT_R24G8_TYPELESS也可以使用
	DsvResourceDesc.SampleDesc.Count = 4;	//多重采样数量
	DsvResourceDesc.SampleDesc.Quality = MsaaQualityLevel.NumQualityLevels - 1;	//多重采样质量
	CD3DX12_CLEAR_VALUE OptClear;	//清除资源的优化值，提高清除操作的执行速度（CreateCommittedResource函数中传入）
	OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//24位深度，8位模板,还有个无类型的格式DXGI_FORMAT_R24G8_TYPELESS也可以使用
	OptClear.DepthStencil.Depth = 1;	//初始深度值为1
	OptClear.DepthStencil.Stencil = 0;	//初始模板值为0
										//创建一个资源和一个堆，并将资源提交至堆中（将深度模板数据提交至GPU显存中）
	winrt::com_ptr<ID3D12Resource> DepthStencilBuffer;
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
		IID_PPV_ARGS(&DepthStencilBuffer));	//返回深度模板资源
											//创建DSV(必须填充DSV属性结构体，和创建RTV不同，RTV是通过句柄)
											//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
											//dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
											//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
											//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
											//dsvDesc.Texture2D.MipSlice = 0;
	D3dDevice->CreateDepthStencilView(DepthStencilBuffer.get(),
		nullptr,	//D3D12_DEPTH_STENCIL_VIEW_DESC类型指针，可填&dsvDesc（见上注释代码），
					//由于在创建深度模板资源时已经定义深度模板数据属性，所以这里可以指定为空指针
		DsvHeap->GetCPUDescriptorHandleForHeapStart());	//DSV句柄
}

void MDX12Base::FlushCmdQueue(winrt::com_ptr<ID3D12Fence> Fence, winrt::com_ptr<ID3D12CommandQueue> CmdQueue)
{
	static int mCurrentFence = 0;
	mCurrentFence++;	//CPU传完命令并关闭后，将当前围栏值+1
	CmdQueue->Signal(Fence.get(), mCurrentFence);	//当GPU处理完CPU传入的命令后，将fence接口中的围栏值+1，即fence->GetCompletedValue()+1
	if (Fence->GetCompletedValue() < mCurrentFence)	//如果小于，说明GPU没有处理完所有命令
	{
		HANDLE EventHandle = CreateEvent(nullptr, false, false, L"FenceSetDone");	//创建事件
		Fence->SetEventOnCompletion(mCurrentFence, EventHandle);//当围栏达到mCurrentFence值（即执行到Signal（）指令修改了围栏值）时触发的eventHandle事件
		WaitForSingleObject(EventHandle, INFINITE);//等待GPU命中围栏，激发事件（阻塞当前线程直到事件触发，注意此Enent需先设置再等待，
												   //如果没有Set就Wait，就死锁了，Set永远不会调用，所以也就没线程可以唤醒这个线程）
		CloseHandle(EventHandle);
	}
}

winrt::com_ptr<ID3D12Resource> MDX12Base::CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, winrt::com_ptr<ID3D12Resource>& UploadBuffer)
{
	// 本函数执行完并不立即拷贝数据到DefaultBuffer
	// 直到CmdList 真正执行后，才会完成数据拷贝到DefaultBuffer

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
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
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
