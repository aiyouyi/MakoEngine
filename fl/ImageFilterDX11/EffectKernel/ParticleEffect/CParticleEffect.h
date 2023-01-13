#ifndef _C_PARTICLE_EFFECT_H_
#define _C_PARTICLE_EFFECT_H_

#include <memory>
#include "EffectKernel/CEffectPart.h"
#include "CC3DParticleSystem.h"
#include "EffectKernel/DXBasicSample.h"
#include "Toolbox/Render/TextureRHI.h"

class CParticleEffect : public CEffectPart, protected DXBaicSample
{
public:
	CParticleEffect();
	virtual~CParticleEffect();

	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool ReadConfig(XMLNode& childNode, const std::string& path) override;

	virtual bool Prepare() override;

	virtual void Render(BaseRenderParam &RenderParam) override;

	virtual void Release() override;

	void Emit(glm::vec3 emitPos, Vector4 faceOri, glm::mat4& projMat, float initSize, Vector2 initDir);

	void Update();

	std::tuple<bool, float> isMouthOpened(Vector2 *pFacePoints, float param = 0.25);
private:
	bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL, const std::string& path = "");

	std::shared_ptr<ParticleSystem> mParticleSystem;
	ParticleProps mParticle;

	float m_rotateX = 0.0f;
	float m_MatScaleHead = 1.0f;
	float m_MatScale = 100.f;

	
	//std::shared_ptr<MaterialTex> MainMtl;
	std::shared_ptr<CC3DTextureRHI> MainTex;
	//std::shared_ptr<class MaterialTexRHI> MainTex;

};

#endif