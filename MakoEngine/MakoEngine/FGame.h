#pragma once
#include"MWindow.h"
#include"GLBMeshData.h"
#include <winrt/base.h>
#include "FRootSignature.h"

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

	class FRootSignature* RootSignature;
};

