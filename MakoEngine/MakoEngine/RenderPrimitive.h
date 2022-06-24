#pragma once
#include <winrt/base.h>
#include <d3d12.h>
#include "common\d3dx12.h"
#include <dxgi1_4.h>
#include <vector>;
#include <string>

class RenderPrimitive
{
public:
	winrt::com_ptr<ID3D12Resource> VertexBufferGPU = nullptr;
	winrt::com_ptr<ID3D12Resource> IndexBufferGPU = nullptr;

	winrt::com_ptr<ID3D12Resource> VertexBufferUploader = nullptr;
	winrt::com_ptr<ID3D12Resource> IndexBufferUploader = nullptr;

	bool InitData(class GLBMeshData* pGLBMeshData);

};
class RenderMesh
{
public:
	~RenderMesh();
	void ClearPrimitive();
	std::vector<RenderPrimitive*> Primitives;

};

class RenderModel
{
public:
	~RenderModel();
	void LoadMeshFromFile(std::wstring FilePath);
	void ClearMesh();
	std::vector<RenderMesh*> Meshes;

};
