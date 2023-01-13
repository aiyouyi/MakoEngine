#pragma once
#include "CC3DEngine/Common/CC3DUtils.h"
#include "ToolBox/GL/GLProgramBase.h"
#include "Toolbox/Render/ConstantBuffer.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/ShaderRHI.h"

#ifdef PLATFORM_WINDOWS
enum class UniformType : uint8_t
#else
enum UniformType
#endif
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
		return 1*4;
	case UniformType::BOOL2:
	case UniformType::INT2:
	case UniformType::UINT2:
	case UniformType::FLOAT2:
		return 2*4;
	case UniformType::BOOL3:
	case UniformType::INT3:
	case UniformType::UINT3:
	case UniformType::FLOAT3:
		return 3*4;
	case UniformType::BOOL4:
	case UniformType::INT4:
	case UniformType::UINT4:
	case UniformType::FLOAT4:
		return 4*4;
	case UniformType::MAT3:
		return 12*4;
	case UniformType::MAT4:
		return 16*4;
	}
}

#define BEGIN_SHADER_STRUCT(Name,CBIndex)\
struct Name##Wrap\
{\
	struct UniformInfo\
	{\
	uint32_t offset;\
	uint8_t type;\
	};\
	std::shared_ptr<CC3DConstantBuffer> CB_;\
	std::unordered_map<std::string, UniformInfo > InfoMap_;\
	std::unordered_map<std::string, int32_t> TextureIndexMap_;\
	std::shared_ptr<GLProgramBase> Shader_;\
	int CBIndex_ = CBIndex;\
	struct Name {

#define BEGIN_STRUCT_CONSTRUCT(Name)\
	};\
	Name ConstBuffer;\
	template<typename T>\
	void SetInnerParameter(const std::string& name, const T* value, size_t size)\
	{\
		auto pos = InfoMap_.find(name);\
		if (pos != InfoMap_.end())\
		{\
			size_t offset = pos->second.offset;\
			void* common_ptr = &ConstBuffer;\
			void* p = static_cast<char*>(common_ptr) + offset;\
			memcpy(p, value, size);\
		}\
	}\
	void SetMatrix4Parameter(const std::string& name, const float *value, bool transpose, int count)\
	{\
		auto pos = InfoMap_.find(name);\
		if (pos != InfoMap_.end())\
		{\
			size_t offset = pos->second.offset;\
			void* common_ptr = &ConstBuffer;\
			void* p = static_cast<char*>(common_ptr) + offset;\
			memcpy(p, value, sizeof(float)*16*count);\
		}\
		if(Shader_)\
			Shader_->SetUniformMatrix4fv(name.c_str(), value, transpose, count);\
	}\
	template<typename T>\
	void SetParameter(const std::string& name, const T& value)\
	{\
		SetInnerParameter(name,&value,sizeof(value));\
		if(Shader_)\
			SetGLParameter(name,value);\
	}\
	template<> void SetParameter<Vector3>(const std::string& name, const Vector3& value)\
	{\
		SetInnerParameter(name,&value,sizeof(value));\
		if(Shader_)\
			Shader_->SetUniform3f(name.c_str(),value.x,value.y,value.z);\
	}\
	template<> void SetParameter<Vector2>(const std::string& name, const Vector2& value)\
	{\
		SetInnerParameter(name,&value,sizeof(value));\
		if(Shader_)\
			Shader_->SetUniform2f(name.c_str(),value.x,value.y);\
	}\
	template<> void SetParameter<Vector4>(const std::string& name, const Vector4& value)\
	{\
		SetInnerParameter(name,&value,sizeof(value));\
		if(Shader_)\
			Shader_->SetUniform4f(name.c_str(),value.x,value.y,value.z,value.w);\
	}\
	template<> void SetParameter<glm::mat4>(const std::string& name, const glm::mat4 &mat)\
	{\
		if(!Shader_){\
			glm::mat4 transposeMat = glm::transpose(mat);\
			SetMatrix4Parameter(name,&transposeMat[0][0],false,1);\
		}\
		else{\
			SetMatrix4Parameter(name,&mat[0][0],false,1);\
		}\
	}\
	template<typename T>\
	void SetTexture(const std::string& name, T TextureRHI)\
	{\
		auto itr = TextureIndexMap_.find(name);\
		if (itr != TextureIndexMap_.end())\
		{\
			int texture_index = itr->second;\
			GetDynamicRHI()->SetPSShaderResource(texture_index, TextureRHI);\
			if(Shader_)\
				Shader_->SetUniform1i(name.c_str(),texture_index);\
		}\
	}\
	template<typename T>\
	void SetGSTexture(const std::string& name, T TextureRHI)\
	{\
		auto itr = TextureIndexMap_.find(name);\
		if (itr != TextureIndexMap_.end())\
		{\
			int texture_index = itr->second;\
			GetDynamicRHI()->SetGSShaderResource(texture_index, TextureRHI);\
			if(Shader_)\
				Shader_->SetUniform1i(name.c_str(),texture_index);\
		}\
	}\
	template<typename T> void SetGLParameter(const std::string& name, const T& value)\
	{\
		auto pos = InfoMap_.find(name.c_str());\
		if (pos != InfoMap_.end())\
		{\
			switch(pos->second.type){\
			case UniformType::BOOL:\
			case UniformType::INT:\
			case UniformType::UINT:\
				Shader_->SetUniform1i(name.c_str(),value);\
				break;\
			case UniformType::FLOAT:\
				Shader_->SetUniform1f(name.c_str(),value);\
				break;\
			}\
		}\
	}\
	template<> void SetGLParameter<Vector3>(const std::string& name, const Vector3& value)\
	{\
		Shader_->SetUniform3f(name.c_str(),value.x,value.y,value.z);\
	}\
	template<> void SetGLParameter<Vector2>(const std::string& name, const Vector2& value)\
	{\
		Shader_->SetUniform2f(name.c_str(),value.x,value.y);\
	}\
	template<> void SetGLParameter<Vector4>(const std::string& name, const Vector4& value)\
	{\
		Shader_->SetUniform4f(name.c_str(),value.x,value.y,value.z,value.w);\
	}\
	template<> void SetGLParameter<glm::mat4>(const std::string& name, const glm::mat4 &mat)\
	{\
		Shader_->SetUniformMatrix4fv(name.c_str(), &mat[0][0],false , 1);\
	}\
	template<typename T,int32 Num> void SetParameter(const std::string& name, const T (&value)[Num])\
	{\
		SetInnerParameter(name,value,sizeof(T)*Num);\
		if(Shader_)\
		{\
			for(int32 index = 0; index < Num;++index)\
			{\
				char strUniformName[100] = {'\0'};\
				sprintf_s(strUniformName, "%s[%d]",name.c_str(), index); \
				SetGLParameter(strUniformName,value[index]);\
			}\
		}\
	}\
	template<typename T> void SetArraySingleElementParamter(const std::string& name,const T& value,int index)\
	{\
		auto pos = InfoMap_.find(name);\
		if (pos != InfoMap_.end())\
		{\
			size_t offset = pos->second.offset;\
			void* common_ptr = &ConstBuffer;\
			void* p = static_cast<char*>(common_ptr) + offset+index*sizeof(T);\
			memcpy(p, &value, sizeof(T));\
		}\
		if(Shader_)\
		{\
			char strUniformName[100] = {'\0'};\
			sprintf_s(strUniformName, "%s[%d]",name.c_str(), index); \
			SetGLParameter(strUniformName,value);\
		}\
	}\
	template<> void SetArraySingleElementParamter<glm::mat4>(const std::string& name,const glm::mat4& value,int index)\
	{\
		auto pos = InfoMap_.find(name);\
		if (pos != InfoMap_.end())\
		{\
			size_t offset = pos->second.offset;\
			void* common_ptr = &ConstBuffer;\
			void* p = static_cast<char*>(common_ptr) + offset+index*sizeof(glm::mat4);\
			if(!Shader_)\
			{\
				glm::mat4 transposeMat = glm::transpose(value);\
				memcpy(p, &transposeMat[0][0], sizeof(glm::mat4));\
			}\
			else{\
				memcpy(p, &value[0][0], sizeof(glm::mat4)); \
			}\
		}\
		if(Shader_)\
		{\
			char strUniformName[100] = {'\0'};\
			sprintf_s(strUniformName, "%s[%d]",name.c_str(), index); \
			SetGLParameter(strUniformName,value);\
		}\
	}\
	void UpdateConstBuffer()\
	{\
		GetDynamicRHI()->UpdateConstantBuffer(CB_, &ConstBuffer);\
	}\
	void ApplyToAllBuffer()\
	{\
		UpdateConstBuffer();\
		GetDynamicRHI()->SetVSConstantBuffer(CBIndex_, CB_);\
		GetDynamicRHI()->SetPSConstantBuffer(CBIndex_, CB_);\
		GetDynamicRHI()->SetGSConstantBuffer(CBIndex_, CB_);\
	}\
	void ApplyToVSBuffer()\
	{\
		UpdateConstBuffer();\
		GetDynamicRHI()->SetVSConstantBuffer(CBIndex_, CB_);\
	}\
	void ApplyToPSBuffer()\
	{\
		UpdateConstBuffer();\
		GetDynamicRHI()->SetPSConstantBuffer(CBIndex_, CB_);\
	}\
	void ApplyToGSBuffer()\
	{\
		UpdateConstBuffer();\
		GetDynamicRHI()->SetGSConstantBuffer(CBIndex_, CB_);\
	}\
	Name##Wrap(){\
	CB_ = GetDynamicRHI()->CreateConstantBuffer(sizeof(Name));\
	uint32_t offset = 0;

#define IMPLEMENT_PARAM(ValName,Type) \
	InfoMap_.insert({ ValName,{offset,(uint8_t)Type}});\
	offset+=StrideForType(Type);

#define IMPLEMENT_ARRAY_PARAM(ValName,Type,Number) \
	InfoMap_.insert({ ValName,{offset,(uint8_t)Type}});\
	offset+=StrideForType(Type)*Number;

#define INIT_TEXTURE_INDEX(Name,Index) TextureIndexMap_.insert({Name,Index});

#define END_STRUCT_CONSTRUCT }

#define DECLARE_PARAM(Type,ValName) Type ValName;
#define DECLARE_PARAM_VALUE(Type,ValName,Val) Type ValName = Val;
#define DECLARE_ARRAY_PARAM(Type,Count,ValName) Type ValName##[Count];

#define END_SHADER_STRUCT };

#define DECLARE_SHADER_STRUCT_MEMBER(Name) Name##Wrap m_##Name##ConstantBuffer;
#define GET_SHADER_STRUCT_MEMBER(Name) m_##Name##ConstantBuffer
#define GET_CONSTBUFFER(Name) m_##Name##ConstantBuffer.ConstBuffer


BEGIN_SHADER_STRUCT(ConstantBufferVec4, 0)
	DECLARE_PARAM(Vector4, param)
	BEGIN_STRUCT_CONSTRUCT(ConstantBufferVec4)
		IMPLEMENT_PARAM("param", UniformType::FLOAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

//
BEGIN_SHADER_STRUCT(ConstantBufferMat4, 0)
	DECLARE_PARAM(glm::mat4, matWVP)
	BEGIN_STRUCT_CONSTRUCT(ConstantBufferMat4)
		IMPLEMENT_PARAM("matWVP", UniformType::MAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT


