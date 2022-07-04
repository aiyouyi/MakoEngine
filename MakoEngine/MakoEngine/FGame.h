#pragma once
#include"MWindow.h"
#include"GLBMeshData.h"
#include <winrt/base.h>
#include "FRootSignature.h"
#include <DirectXCollision.h>
#include "UploadBuffer.h"


void TestTemp();

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ObjectConstants
{
	ObjectConstants();
	XMFLOAT4X4 WorldViewProj;
};

struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
};



struct MeshGeometry
{
	// Give it a name so we can look it up by name.
	std::string Name;

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	winrt::com_ptr<ID3DBlob> VertexBufferCPU = nullptr;
	winrt::com_ptr<ID3DBlob> IndexBufferCPU = nullptr;

	winrt::com_ptr<ID3D12Resource> VertexBufferGPU = nullptr;
	winrt::com_ptr<ID3D12Resource> IndexBufferGPU = nullptr;

	winrt::com_ptr<ID3D12Resource> VertexBufferUploader = nullptr;
	winrt::com_ptr<ID3D12Resource> IndexBufferUploader = nullptr;

	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};


class FGame
{
public:
	FGame(MWindow* pMWindow);
	MWindow* pWindow;
	GLBMeshData* GLBScene;
	class MDX12Base* DX12Base = nullptr;
	void Init();
	void Tick();

	void BuildPSO();
	void BuildShadersAndInputLayout();
	winrt::com_ptr<ID3DBlob> mvsByteCode = nullptr;
	winrt::com_ptr<ID3DBlob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	winrt::com_ptr<ID3D12PipelineState> PSO = nullptr;

	winrt::com_ptr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	class FRootSignature* RootSignature;
	UINT64 mCurrentFence = 0;

	void BuildBox();
	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

	static XMFLOAT4X4 Identity4x4();

	void Flush();

};

