#pragma once
#include "Toolbox/Render/CC3DShaderDef.h"

BEGIN_SHADER_STRUCT(GPUParticle_ChangesEveryFrame, 0)
	DECLARE_PARAM(glm::mat4, ViewProj)
	DECLARE_PARAM(Vector3, EyePosW)
	DECLARE_PARAM_VALUE(float, GameTime,0)
	DECLARE_PARAM_VALUE(Vector3, EmitDirW, Vector3(0.0f,1.0f,0.0f))
	DECLARE_PARAM(float, TimeStep)
	DECLARE_PARAM_VALUE(Vector3, EmitPosW, Vector3(0.0f, 0.0f, 0.0f))
	DECLARE_PARAM_VALUE(float, EmitInterval, 0.001f)
	DECLARE_PARAM(Vector3, AccelW)
	DECLARE_PARAM_VALUE(float, AliveTime, 1.0f)
	DECLARE_PARAM_VALUE(int32_t, IsCanEmit, 0)
	DECLARE_PARAM(Vector3, Padding)
	BEGIN_STRUCT_CONSTRUCT(GPUParticle_ChangesEveryFrame)
		IMPLEMENT_PARAM("g_ViewProj", UniformType::MAT4)
		IMPLEMENT_PARAM("g_EyePosW", UniformType::FLOAT3)
		IMPLEMENT_PARAM("g_GameTime", UniformType::FLOAT)
		IMPLEMENT_PARAM("g_EmitDirW", UniformType::FLOAT3)
		IMPLEMENT_PARAM("g_TimeStep", UniformType::FLOAT)
		IMPLEMENT_PARAM("g_EmitPosW", UniformType::FLOAT3)
		IMPLEMENT_PARAM("g_EmitInterval", UniformType::FLOAT)
		IMPLEMENT_PARAM("g_AccelW", UniformType::FLOAT3)
		IMPLEMENT_PARAM("g_AliveTime", UniformType::FLOAT)
		IMPLEMENT_PARAM("g_IsCanEmit", UniformType::INT)
		IMPLEMENT_PARAM("Padding", UniformType::FLOAT3)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(GPUParticle_Texture, -1)
	BEGIN_STRUCT_CONSTRUCT(GPUParticle_Texture)
		INIT_TEXTURE_INDEX("g_TextureInput", 0)
		INIT_TEXTURE_INDEX("g_TextureRandom", 1)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class BaseRenderParam;


struct GPUParticleEffectP;
class GPUParticleEffect
{
public:
	GPUParticleEffect();
	~GPUParticleEffect();

	void SetShaderResource(const std::string& path, std::shared_ptr<MaterialTexRHI> Tex);
	void Render(BaseRenderParam& scene);
	void Update(Vector2 EmitPos, BaseRenderParam& scene);
private:
	void RenderToVertexBuffer(
		std::shared_ptr< CC3DVertexBuffer> input,
		std::shared_ptr< CC3DVertexBuffer> output,
		uint32_t vertexCount = 0);

private:
	std::unique_ptr<GPUParticleEffectP> Data;
	DECLARE_SHADER_STRUCT_MEMBER(GPUParticle_ChangesEveryFrame);
	DECLARE_SHADER_STRUCT_MEMBER(GPUParticle_Texture);
};