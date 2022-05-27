#pragma once
#include <vector>
#include <d3d12.h>
#include<string>
#include <winrt/base.h>
#include"FRootParameter.h"

class FRootSignature
{
public:
	FRootSignature(UINT NumRootParams = 0, UINT NumStaticSamplers = 0);

	~FRootSignature();

	void Reset(UINT NumRootParam, UINT NumStaticSampler);

	FRootParameter& operator[] (size_t EntryIndex)
	{
		return ParamArray[EntryIndex];
	}

	const FRootParameter& operator[] (size_t EntryIndex) const
	{
		return ParamArray[EntryIndex];
	}

	void InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& SamplerDesc, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);

	void Finalize(winrt::com_ptr<ID3D12Device> D3DDevice, const std::wstring& Name, D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

	ID3D12RootSignature* GetSignature() const { return D3DRootSignature; }

	UINT GetNumParameters() const { return NumParameters; }

	uint32_t GetSamplerTableBitMap() const { return SamplerTableBitMap; }
	uint32_t GetDescriptorTableBitMap() const { return DescriptorTableBitMap; }
	uint32_t GetDescriptorTableSize(uint32_t RootIndex) const { return DescriptorTableSize[RootIndex]; }


protected:
	bool Finalized = false;
	UINT NumParameters = 0;
	UINT NumStaticSamplers = 0;

	uint32_t DescriptorTableBitMap = 0;
	uint32_t SamplerTableBitMap = 0;
	uint32_t DescriptorTableSize[16] = {};

	std::vector<FRootParameter> ParamArray;
	std::vector< D3D12_STATIC_SAMPLER_DESC> StaticSamplerArray;
	ID3D12RootSignature* D3DRootSignature = nullptr;
};
