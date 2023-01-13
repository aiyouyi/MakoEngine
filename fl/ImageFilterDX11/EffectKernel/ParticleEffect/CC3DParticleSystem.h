#ifndef _CC_PARTICLE_SYSTEM_H_
#define _CC_PARTICLE_SYSTEM_H_

#include "BaseDefine/Define.h"
#include <vector>
#include <memory>
#include <string>
#include "Toolbox/Render/VertexBuffer.h"
#include "Toolbox/Render/IndexBuffer.h"
//#include "GL/CCProgram.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "Toolbox/Render/TextureRHI.h"
#include "CC3DParticle.h"
#include "EffectKernel/BaseRenderParam.h"
#include "EffectKernel/DXBasicSample.h"

BEGIN_SHADER_STRUCT(ParticleConstBuffer, 0)
	BEGIN_STRUCT_CONSTRUCT(ParticleConstBuffer)
	    INIT_TEXTURE_INDEX("MainTex", 0)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class ShaderRHI;

struct ParticleProps
{
	glm::vec3 Position;
	glm::vec2 texcoord;
	glm::vec2 Velocity, VelocityVariation;
	glm::vec2 InitDir;
	glm::vec4 ColorBegin, ColorEnd;
	float SizeBegin, SizeEnd, SizeVariation;
	float SizeControl = 1.0f;
	float LifeTime = 3.0f;
	Vector4 faceOri;
	glm::mat4 projMat = glm::mat4(1.0f);
	std::string particleType;
	int nCount = 0;    //粒子数量
	float Texcoord[8];
};

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	//void OnUpdate(float ms); //单位秒
	void OnRender(BaseRenderParam& scene);
	void ConstructBubbleMesh(glm::mat4& viewMat);
	void ConstructFireMesh();
	void Emit(const ParticleProps& particleProps);
	void SetShaderResource(const std::string& path);
	void SetMainTex(std::shared_ptr<CC3DTextureRHI> tex);
	void SetTexcoord(float* texcoord);
	void SetParticleCount(uint32_t nCount);

	std::vector<Particle*> m_ParticlePool;

private:
	bool mInitPoolFlag = true;
	uint32_t m_PoolIndex = 999;
	uint32_t m_ParticleCountAll = 1000;
	
	std::array<std::shared_ptr<CC3DVertexBuffer>, 10> quadVertexbuffer;
	std::shared_ptr<CC3DIndexBuffer> quadIndexBuffer;
	int AttributeCount = 2;
	std::shared_ptr<CC3DTextureRHI> MainTex = nullptr;
	//std::shared_ptr<CCProgram> pShaderParticle;
	//DX11Shader* pShaderParticle = nullptr;
	std::shared_ptr<ShaderRHI> pShaderParticle;

	Vector3* mVertexAlloc = nullptr;
	Vector2* mTexcoordAlloc = nullptr;
	uint16* mIndexAlloc = nullptr;
	float* mTexcoord = nullptr;

	int nVertexCount = 0;
	int nTriangleCount = 0;

	// Init four vertices
	glm::vec3 vertices[4] = 
	{
		{ -0.5f, -0.5f, 0.0f},
		{  0.5f, -0.5f, 0.0f},
		{ 0.5f,  0.5f, 0.0f },
		{ -0.5f,  0.5f, 0.0f}
	};

	DECLARE_SHADER_STRUCT_MEMBER(ParticleConstBuffer)
};

#endif