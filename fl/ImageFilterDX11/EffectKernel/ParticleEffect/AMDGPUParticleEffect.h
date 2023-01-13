#pragma once //from https://github.com/GPUOpen-LibrariesAndSDKs/GPUParticles11
#include "Toolbox/Render/CC3DShaderDef.h"

class BaseRenderParam;

struct AMDGPUParticleEffectP;

class AMDGPUParticleEffect
{
public:
	AMDGPUParticleEffect();
	~AMDGPUParticleEffect();

	void SetShaderResource(const std::string& path, std::shared_ptr<MaterialTexRHI> Tex);
	void Render(BaseRenderParam& Scene);
	void Update(Vector2 EmitPos, BaseRenderParam& Scene);
private:
	void InitDeadList();
	void InitRenderResource();
	void FillRandomTexture();
	void PopulateEmitter(float frameTime);
	void Emit();
	void Simulate(BaseRenderParam& Scene);
private:
	std::unique_ptr<AMDGPUParticleEffectP> Data;
};
