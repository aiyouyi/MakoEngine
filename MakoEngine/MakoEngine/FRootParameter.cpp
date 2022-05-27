#include "FRootParameter.h"

FRootParameter::FRootParameter()
{
	RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)-1;
}

 FRootParameter::~FRootParameter()
{
	Clear();
}

 void FRootParameter::Clear()
 {
	 if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
	 {
		 delete[] RootParam.DescriptorTable.pDescriptorRanges;
	 }
	 RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)-1;
 }

 void FRootParameter::InitAsConstants(UINT Register, UINT NumDwords, D3D12_SHADER_VISIBILITY Visibility)
 {
	 RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	 RootParam.ShaderVisibility = Visibility;
	 RootParam.Constants.Num32BitValues = NumDwords;
	 RootParam.Constants.ShaderRegister = Register;
	 RootParam.Constants.RegisterSpace = 0;
 }

 void FRootParameter::InitAsBufferCBV(UINT Register, D3D12_SHADER_VISIBILITY Visibility)
 {
	 RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	 RootParam.ShaderVisibility = Visibility;
	 RootParam.Descriptor.ShaderRegister = Register;
	 RootParam.Descriptor.RegisterSpace = 0;
 }

 void FRootParameter::InitAsBufferSRV(UINT Register, D3D12_SHADER_VISIBILITY Visibility)
 {
	 RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	 RootParam.ShaderVisibility = Visibility;
	 RootParam.Descriptor.ShaderRegister = Register;
	 RootParam.Descriptor.RegisterSpace = 0;
 }

 void FRootParameter::InitAsBufferUAV(UINT Register, D3D12_SHADER_VISIBILITY Visibility)
 {
	 RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	 RootParam.ShaderVisibility = Visibility;
	 RootParam.Descriptor.ShaderRegister = Register;
	 RootParam.Descriptor.RegisterSpace = 0;
 }

 void FRootParameter::InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, D3D12_SHADER_VISIBILITY Visibility)
 {
	 InitAsDescriptorTable(1, Visibility);
	 SetTableRange(0, Type, Register, Count);
 }

 void FRootParameter::InitAsDescriptorTable(UINT RangeCount, D3D12_SHADER_VISIBILITY Visibility)
 {
	 RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	 RootParam.ShaderVisibility = Visibility;
	 RootParam.DescriptorTable.NumDescriptorRanges = RangeCount;
	 RootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[RangeCount];
 }

 void FRootParameter::SetTableRange(UINT RangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, UINT Space)
 {
	 D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(RootParam.DescriptorTable.pDescriptorRanges + RangeIndex);
	 range->RangeType = Type;
	 range->NumDescriptors = Count;
	 range->BaseShaderRegister = Register;
	 range->RegisterSpace = Space;
	 range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
 }
