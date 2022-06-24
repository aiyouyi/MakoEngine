#pragma once
#include <winrt/base.h>
#include <d3d12.h>
#include "common\d3dx12.h"
#include <dxgi1_4.h>

class MDX12Base
{
public:
	virtual bool InitEnviroment();
	virtual void Tick();
	virtual void Exit();

	winrt::com_ptr<IDXGIFactory4> CreateDXGIFactory();
	winrt::com_ptr<IDXGIAdapter1> ChooseAdapter(winrt::com_ptr<IDXGIFactory4>& Factory);
	winrt::com_ptr<ID3D12Device> CreateDevice(winrt::com_ptr<IDXGIAdapter1> Adapter);
	winrt::com_ptr<ID3D12Fence> CreateFence(winrt::com_ptr<ID3D12Device> D3dDevice);

	void CreateCommandObject(winrt::com_ptr<ID3D12Device> D3dDevice);
	void GetDescriptorSize(winrt::com_ptr<ID3D12Device> D3dDevice);
	void SetMSAA(winrt::com_ptr<ID3D12Device> D3dDevice);
	void CreateSwapChain(winrt::com_ptr<ID3D12CommandQueue> CmdQueue, winrt::com_ptr<IDXGIFactory4> DxgiFactory, HWND Hwnd);
	void CreateDescriptorHeap(winrt::com_ptr<ID3D12Device> D3dDevice);
	void CreateRTV(winrt::com_ptr<ID3D12Device> D3dDevice, winrt::com_ptr<ID3D12DescriptorHeap> RtvHeap, winrt::com_ptr<IDXGISwapChain> SwapChain, UINT RtvSize);
	void CreateDSV(winrt::com_ptr<ID3D12Device> D3dDevice, D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevel, winrt::com_ptr<ID3D12DescriptorHeap> DsvHeap);
	void FlushCmdQueue(winrt::com_ptr<ID3D12Fence> Fence, winrt::com_ptr<ID3D12CommandQueue> CmdQueue);

	static winrt::com_ptr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList, const void* InitData, UINT64 ByteSize, winrt::com_ptr<ID3D12Resource>& UploadBuffer);

	winrt::com_ptr<IDXGIFactory4>				DXGIFactory = nullptr;
	winrt::com_ptr<IDXGIAdapter1>				DXGIAdapter = nullptr;
	winrt::com_ptr<ID3D12Device>				D3D12Device = nullptr;
	winrt::com_ptr<ID3D12Fence>					D3D12Fence = nullptr;
	winrt::com_ptr<ID3D12CommandQueue>			D3D12CommandQueue = nullptr;
	winrt::com_ptr<ID3D12CommandAllocator>		D3D12CommandAllocator = nullptr;
	winrt::com_ptr<ID3D12GraphicsCommandList>	D3D12GraphicsCommandList = nullptr;
	winrt::com_ptr<IDXGISwapChain>				DXGISwapChain = nullptr;
	winrt::com_ptr<ID3D12DescriptorHeap>		D3D12DescriptorHeapRTV = nullptr;
	winrt::com_ptr<ID3D12DescriptorHeap>		D3D12DescriptorHeapDSV = nullptr;
	winrt::com_ptr<ID3D12Resource>				D3D12ResourceDepthStencilBuffer = nullptr;

	UINT RtvDescriptorSize =			0;
	UINT DsvDescriptorSize =			0;
	UINT Cbv_Srv_UavDescriptorSize =	0;
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;
	HWND WindowHwnd = 0;
};

