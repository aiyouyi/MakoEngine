#include "FRootSignature.h"

HRESULT debughr02 ;

FRootSignature::FRootSignature(UINT NumRootParams, UINT NumStaticSamplers)
{
	Reset(NumRootParams, NumStaticSamplers);
}

FRootSignature::~FRootSignature()
{
	if (D3DRootSignature)
	{
		D3DRootSignature->Release();
		D3DRootSignature = nullptr;
	}
}

void FRootSignature::Reset(UINT NumRootParam, UINT NumStaticSampler)
{
	ParamArray.resize(NumRootParam);
	NumParameters = NumRootParam;

	StaticSamplerArray.clear();
	NumStaticSamplers = NumStaticSampler;
}

void FRootSignature::InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& SamplerDesc, D3D12_SHADER_VISIBILITY Visibility /*= D3D12_SHADER_VISIBILITY_ALL*/)
{
	D3D12_STATIC_SAMPLER_DESC StaticSamplerDesc;
	StaticSamplerDesc.Filter = SamplerDesc.Filter;
	StaticSamplerDesc.AddressU = SamplerDesc.AddressU;
	StaticSamplerDesc.AddressV = SamplerDesc.AddressV;
	StaticSamplerDesc.AddressW = SamplerDesc.AddressW;
	StaticSamplerDesc.MipLODBias = SamplerDesc.MipLODBias;
	StaticSamplerDesc.MaxAnisotropy = SamplerDesc.MaxAnisotropy;
	StaticSamplerDesc.ComparisonFunc = SamplerDesc.ComparisonFunc;
	StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	StaticSamplerDesc.MinLOD = SamplerDesc.MinLOD;
	StaticSamplerDesc.MaxLOD = SamplerDesc.MaxLOD;
	StaticSamplerDesc.ShaderRegister = Register;
	StaticSamplerDesc.RegisterSpace = 0;
	StaticSamplerDesc.ShaderVisibility = Visibility;

	if (SamplerDesc.BorderColor[3] == 1.0f)
	{
		if (SamplerDesc.BorderColor[0] == 1.0f)
			StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		else
			StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	}
	else
	{
		StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	}
	StaticSamplerArray.emplace_back(StaticSamplerDesc);
}

void FRootSignature::Finalize(winrt::com_ptr<ID3D12Device> D3DDevice, const std::wstring& Name, D3D12_ROOT_SIGNATURE_FLAGS Flags)
{
	if (Finalized)
		return;

	D3D12_ROOT_SIGNATURE_DESC RootDesc;
	RootDesc.Flags = Flags;
	RootDesc.NumParameters = NumParameters;
	if (NumParameters == 0)
		RootDesc.pParameters = nullptr;
	else
		RootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)&ParamArray[0];
	RootDesc.NumStaticSamplers = NumStaticSamplers;
	if (NumStaticSamplers == 0)
		RootDesc.pStaticSamplers = nullptr;
	else
		RootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)&StaticSamplerArray[0];

	DescriptorTableBitMap = 0;
	SamplerTableBitMap = 0;
	for (UINT i = 0; i < NumParameters; ++i)
	{
		const D3D12_ROOT_PARAMETER& RootParam = RootDesc.pParameters[i];
		DescriptorTableSize[i] = 0;

		if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			if (RootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
				SamplerTableBitMap |= (1 << i);
			else
				DescriptorTableBitMap |= (1 << i);

			for (UINT TableRange = 0; TableRange < RootParam.DescriptorTable.NumDescriptorRanges; ++TableRange)
			{
				DescriptorTableSize[i] += RootParam.DescriptorTable.pDescriptorRanges[TableRange].NumDescriptors;
			}
		}
	}

	winrt::com_ptr<ID3DBlob> pOutBlob = nullptr, pErrorBlob = nullptr;

	D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1, pOutBlob.put(), pErrorBlob.put());

	debughr02=D3DDevice->CreateRootSignature(1, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(), IID_PPV_ARGS(&D3DRootSignature));

	D3DRootSignature->SetName(Name.c_str());
}
