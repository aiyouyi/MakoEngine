#ifndef HAND_PARTICLE_EFFECT_H_
#define HAND_PARTICLE_EFFECT_H_

#include <memory>
#include "EffectKernel/CEffectPart.h"
#include "CC3DParticleSystem.h"
#include "EffectKernel/DXBasicSample.h"
#include "Render/TextureRHI.h"
#include "GPUParticleEffect.h"
#include "AMDGPUParticleEffect.h"

class MaterialTexRHI;

class CHandParticleEffect : public CEffectPart, protected DXBaicSample
{
public:
	CHandParticleEffect();
	virtual~CHandParticleEffect();

	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool ReadConfig(XMLNode& childNode, const std::string& path);

	virtual bool Prepare() override;

	virtual void Render(BaseRenderParam& RenderParam) override;

	virtual void Release() override;

	void Emit(glm::vec3 emitPos, Vector4 faceOri, glm::mat4& projMat, float initSize, Vector2 initDir);

	void Update();

private:
	void DoReady();
private:
	bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL, const std::string& path = "");


	std::shared_ptr<ParticleSystem> mParticleSystem;
	ParticleProps mParticle;

	std::shared_ptr<MaterialTexRHI> MainTex;
	std::shared_ptr<CC3DBlendState> BlendAddOn;

	float mTexcoord[9][8] = {
		//{0.0, 0.0, 0.33333, 0.0, 0.33333,0.33333, 0.0, 0.33333},
		{0.0, 0.0, 0.33333, 0.0, 0.33333,0.33333, 0.0, 0.33333},
		{0.33333, 0.0, 0.66667, 0.0,0.66667,0.33333,0.33333,0.33333},
		{0.66667, 0.0, 1.0, 0.0, 1.0, 0.33333, 0.66667, 0.33333},
		{0.0, 0.33333, 0.33333, 0.33333, 0.33333, 0.66667, 0.0, 0.66667},
		{0.33333, 0.33333, 0.66667, 0.33333, 0.66667, 0.66667, 0.33333, 0.66667},
		{0.66667, 0.33333, 1.0, 0.33333, 1.0, 0.66667, 0.66667, 0.66667},
		{0.0, 0.66667, 0.33333, 0.66667, 0.33333, 1.0, 0.0, 1.0},
		{0.33333, 0.66667, 0.66667, 0.66667, 0.66667, 1.0, 0.33333, 1.0},
		{0.66667, 0.66667, 1.0, 0.66667, 1.0, 1.0, 0.66667, 1.0},
	};


	std::shared_ptr< GPUParticleEffect> GPUParticle;
	std::shared_ptr< AMDGPUParticleEffect > AMDGPUParticle;
	bool NeedPrepare = false;
	bool UseGPU = true;
	bool UseAMDGPU = true;
};

#endif