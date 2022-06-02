#pragma once
#include <unordered_map>
#include "ConstantBuffer.h"
#include "DynamicRHI.h"
enum class UniformType : uint8_t
{
	BOOL,
	BOOL2,
	BOOL3,
	BOOL4,
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	INT,
	INT2,
	INT3,
	INT4,
	UINT,
	UINT2,
	UINT3,
	UINT4,
	MAT3,   //!< a 3x3 float matrix
	MAT4    //!< a 4x4 float matrix
};

inline uint8_t StrideForType(UniformType type) noexcept
{
	switch (type) {
	case UniformType::BOOL:
	case UniformType::INT:
	case UniformType::UINT:
	case UniformType::FLOAT:
		return 1;
	case UniformType::BOOL2:
	case UniformType::INT2:
	case UniformType::UINT2:
	case UniformType::FLOAT2:
		return 2;
	case UniformType::BOOL3:
	case UniformType::INT3:
	case UniformType::UINT3:
	case UniformType::FLOAT3:
		return 3;
	case UniformType::BOOL4:
	case UniformType::INT4:
	case UniformType::UINT4:
	case UniformType::FLOAT4:
		return 4;
	case UniformType::MAT3:
		return 12;
	case UniformType::MAT4:
		return 16;
	}
}

#define BEING_SHADER_STRUCT(Name,CBIndex)\
struct Name##Wrap\
{\
	struct UniformInfo\
	{\
	std::string name;\
	unsigned int offset;\
	uint8_t stride;\
	int index;\
	};\
	std::shared_ptr<CC3DConstantBuffer> CB_;\
	std::unordered_map<std::string, UniformInfo > InfoMap_;\
	std::unordered_map<std::string, int> TextureIndexMap_;\
	int CBIndex_ = CBIndex;\
	struct Name {

#define BEING_STRUCT_CONSTRUCT(Name)\
	};\
	Name ConstBuffer;\
	template<typename T>\
	void SetParameter(std::string name, const T* value, size_t size)\
	{\
		auto pos = InfoMap_.find(name.c_str());\
		if (pos != InfoMap_.end())\
		{\
			size_t offset = pos->second.offset;\
			void* common_ptr = &ConstBuffer;\
			void* p = static_cast<char*>(common_ptr) + offset * 4;\
			memcpy(p, value, size);\
		}\
	}\
	void UpdateConstBufer()\
	{\
		GetDynamicRHI()->UpdateConstantBuffer(CB_, &ConstBuffer);\
	}\
	template<typename T>\
	void SetTexture(const char* str, T TextureRHI)\
	{\
		auto itr = TextureIndexMap_.find(str);\
		if (itr != TextureIndexMap_.end())\
		{\
			int texture_index = itr->second;\
			GetDynamicRHI()->SetPSShaderResource(texture_index, TextureRHI);\
		}\
	}\
	void ApplyToAllBuffer()\
	{\
		UpdateConstBufer();\
		GetDynamicRHI()->SetVSConstantBuffer(CBIndex_, CB_);\
		GetDynamicRHI()->SetPSConstantBuffer(CBIndex_, CB_);\
	}\
	void ApplyToVSBuffer()\
	{\
		UpdateConstBufer();\
		GetDynamicRHI()->SetVSConstantBuffer(CBIndex_, CB_);\
	}\
	void ApplyToPSBuffer()\
	{\
		UpdateConstBufer();\
		GetDynamicRHI()->SetPSConstantBuffer(CBIndex_, CB_);\
	}\
	Name##Wrap(){\
	CB_ = GetDynamicRHI()->CreateConstantBuffer(sizeof(Name));\
	int index = 0;\
	uint32_t offset = 0;

#define IMPLEMENT_PARAM(ValName,Type) \
	InfoMap_.insert({ ValName,{ValName,offset,StrideForType(Type),index}});\
	++index;\
	offset+=StrideForType(Type);

#define INIT_TEXTURE_INDEX(Name,Index) TextureIndexMap_.insert({Name,Index});

#define END_STRUCT_CONSTRUCT }

#define DELCARE_PARAM(Type,ValName) Type ValName;
#define DELCARE_PARAM_VALUE(Type,ValName,Val) Type ValName = Val;
#define DELCARE_ARRAY_PARAM(Type,Count,ValName) Type ValName##[Count];

#define END_SHADER_STRUCT };

#define DELCARE_SHADER_STRUCT_MEMBER(Name) Name##Wrap m_##Name##ConstantBuffer;
#define GET_SHADER_STRUCT_MEMBER(Name) m_##Name##ConstantBuffer
#define GET_CONSTBUFFER(Name) m_##Name##ConstantBuffer.ConstBuffer


BEING_SHADER_STRUCT(ConstantBufferVec4,0)
    DELCARE_ARRAY_PARAM(Vector4,4, param)
	BEING_STRUCT_CONSTRUCT(ConstantBufferVec4)
		IMPLEMENT_PARAM("param", UniformType::FLOAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT
;

BEING_SHADER_STRUCT(ConstantBufferMat4,0)
	DELCARE_PARAM(glm::mat4, matWVP)
	BEING_STRUCT_CONSTRUCT(ConstantBufferMat4)
		IMPLEMENT_PARAM("matWVP", UniformType::MAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT
;