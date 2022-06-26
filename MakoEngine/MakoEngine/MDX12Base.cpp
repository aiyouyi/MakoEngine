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
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;	//�����������ṹ��
	SwapChainDesc.BufferDesc.Width = 1280;	//�������ֱ��ʵĿ��
	SwapChainDesc.BufferDesc.Height = 720;	//�������ֱ��ʵĸ߶�
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//����������ʾ��ʽ
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;	//ˢ���ʵķ���
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;	//ˢ���ʵķ�ĸ
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	//����ɨ��VS����ɨ��(δָ����)
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	//ͼ�������Ļ�����죨δָ���ģ�
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//��������Ⱦ����̨������������Ϊ��ȾĿ�꣩
	SwapChainDesc.OutputWindow = Hwnd;	//��Ⱦ���ھ��
	SwapChainDesc.SampleDesc.Count = 1;	//���ز�������
	SwapChainDesc.SampleDesc.Quality = 0;	//���ز�������
	SwapChainDesc.Windowed = true;	//�Ƿ񴰿ڻ�
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//�̶�д��
	SwapChainDesc.BufferCount = 2;	//��̨������������˫���壩
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//����Ӧ����ģʽ���Զ�ѡ�������ڵ�ǰ���ڳߴ����ʾģʽ��
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
	//��CPU�д��������ģ��������Դ
	D3D12_RESOURCE_DESC DsvResourceDesc;
	DsvResourceDesc.Alignment = 0;	//ָ������
	DsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	//ָ����Դά�ȣ����ͣ�ΪTEXTURE2D
	DsvResourceDesc.DepthOrArraySize = 1;	//�������Ϊ1
	DsvResourceDesc.Width = 1280;	//��Դ��
	DsvResourceDesc.Height = 720;	//��Դ��
	DsvResourceDesc.MipLevels = 1;	//MIPMAP�㼶����
	DsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	//ָ�������֣����ﲻָ����
	DsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//���ģ����Դ��Flag
	DsvResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//24λ��ȣ�8λģ��,���и������͵ĸ�ʽDXGI_FORMAT_R24G8_TYPELESSҲ����ʹ��
	DsvResourceDesc.SampleDesc.Count = 4;	//���ز�������
	DsvResourceDesc.SampleDesc.Quality = MsaaQualityLevel.NumQualityLevels - 1;	//���ز�������
	CD3DX12_CLEAR_VALUE OptClear;	//�����Դ���Ż�ֵ��������������ִ���ٶȣ�CreateCommittedResource�����д��룩
	OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//24λ��ȣ�8λģ��,���и������͵ĸ�ʽDXGI_FORMAT_R24G8_TYPELESSҲ����ʹ��
	OptClear.DepthStencil.Depth = 1;	//��ʼ���ֵΪ1
	OptClear.DepthStencil.Stencil = 0;	//��ʼģ��ֵΪ0
										//����һ����Դ��һ���ѣ�������Դ�ύ�����У������ģ�������ύ��GPU�Դ��У�
	winrt::com_ptr<ID3D12Resource> DepthStencilBuffer;
	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;
	D3dDevice->CreateCommittedResource(&HeapProps,	//������ΪĬ�϶ѣ�����д�룩
		D3D12_HEAP_FLAG_NONE,	//Flag
		&DsvResourceDesc,	//���涨���DSV��Դָ��
		D3D12_RESOURCE_STATE_COMMON,	//��Դ��״̬Ϊ��ʼ״̬
		&OptClear,	//���涨����Ż�ֵָ��
		IID_PPV_ARGS(&DepthStencilBuffer));	//�������ģ����Դ
											//����DSV(�������DSV���Խṹ�壬�ʹ���RTV��ͬ��RTV��ͨ�����)
											//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
											//dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
											//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
											//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
											//dsvDesc.Texture2D.MipSlice = 0;
	D3dDevice->CreateDepthStencilView(DepthStencilBuffer.get(),
		nullptr,	//D3D12_DEPTH_STENCIL_VIEW_DESC����ָ�룬����&dsvDesc������ע�ʹ��룩��
					//�����ڴ������ģ����Դʱ�Ѿ��������ģ���������ԣ������������ָ��Ϊ��ָ��
		DsvHeap->GetCPUDescriptorHandleForHeapStart());	//DSV���
}

void MDX12Base::FlushCmdQueue(winrt::com_ptr<ID3D12Fence> Fence, winrt::com_ptr<ID3D12CommandQueue> CmdQueue)
{
	static int mCurrentFence = 0;
	mCurrentFence++;	//CPU��������رպ󣬽���ǰΧ��ֵ+1
	CmdQueue->Signal(Fence.get(), mCurrentFence);	//��GPU������CPU���������󣬽�fence�ӿ��е�Χ��ֵ+1����fence->GetCompletedValue()+1
	if (Fence->GetCompletedValue() < mCurrentFence)	//���С�ڣ�˵��GPUû�д�������������
	{
		HANDLE EventHandle = CreateEvent(nullptr, false, false, L"FenceSetDone");	//�����¼�
		Fence->SetEventOnCompletion(mCurrentFence, EventHandle);//��Χ���ﵽmCurrentFenceֵ����ִ�е�Signal����ָ���޸���Χ��ֵ��ʱ������eventHandle�¼�
		WaitForSingleObject(EventHandle, INFINITE);//�ȴ�GPU����Χ���������¼���������ǰ�߳�ֱ���¼�������ע���Enent���������ٵȴ���
												   //���û��Set��Wait���������ˣ�Set��Զ������ã�����Ҳ��û�߳̿��Ի�������̣߳�
		CloseHandle(EventHandle);
	}
}

winrt::com_ptr<ID3D12Resource> MDX12Base::CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, winrt::com_ptr<ID3D12Resource>& UploadBuffer)
{
	// ������ִ���겢�������������ݵ�DefaultBuffer
	// ֱ��CmdList ����ִ�к󣬲Ż�������ݿ�����DefaultBuffer

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
