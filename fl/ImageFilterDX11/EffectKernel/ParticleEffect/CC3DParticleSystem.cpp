#include "CC3DParticleSystem.h"
#include "ToolBox/CC3DRandom.h"
#include "Toolbox/Render/DynamicRHI.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/compatibility.hpp"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/ShaderRHI.h"
#include "../ShaderProgramManager.h"

ParticleSystem::ParticleSystem()
{
	CC3DRandom::Init();
}

ParticleSystem::~ParticleSystem()
{
	SAFE_DELETE_ARRAY(mVertexAlloc);
	SAFE_DELETE_ARRAY(mTexcoordAlloc);
	SAFE_DELETE_ARRAY(mIndexAlloc);
}

void ParticleSystem::ConstructBubbleMesh(glm::mat4& viewMat)
{
	auto calPos = [&](glm::vec4& temp1, Particle* particle) {
		temp1 = temp1 / temp1.w;
		temp1.y = -temp1.y;
		temp1.x = (temp1.x + 1.0) * particle->faceOri.z * 0.5 + particle->faceOri.x;
		temp1.x = temp1.x * 2.0 - 1.0;
		temp1.y = (temp1.y + 1.0) * particle->faceOri.w * 0.5 + particle->faceOri.y;
		temp1.y = temp1.y * 2.0 - 1.0;
	};
	nVertexCount = 0;
	nTriangleCount = 0;
	for (auto& particle : m_ParticlePool)
	{
		if (!particle)
		{
			continue;
		}
		if (!particle->Active)
		{
			continue;
		}
		float life = particle->LifeRemaining / particle->LifeTime;
		glm::vec4 color = glm::lerp(particle->ColorEnd, particle->ColorBegin, life);
		float size = glm::lerp(particle->SizeEnd, particle->SizeBegin, life);
		// Render
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle->Position.x, particle->Position.y, particle->Position.z })
			* glm::rotate(glm::mat4(1.0f), particle->Rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

		vertices[0] = transform * glm::vec4(-0.5f * 100, -0.5f * 100, 0.0f, 1.0);
		vertices[1] = transform * glm::vec4(0.5f * 100, -0.5f * 100, 0.0f, 1.0);
		vertices[2] = transform * glm::vec4(0.5f * 100, 0.5f * 100, 0.0f, 1.0);
		vertices[3] = transform * glm::vec4(-0.5f * 100, 0.5f * 100, 0.0f, 1.0);

		glm::mat4 VP = particle->projMat * viewMat;

		glm::vec4 temp1 = VP * glm::vec4(vertices[0], 1.0);
		calPos(temp1, particle);
		vertices[0] = glm::vec3(temp1.x, temp1.y, temp1.z);

		temp1 = VP * glm::vec4(vertices[1], 1.0);
		calPos(temp1, particle);
		vertices[1] = glm::vec3(temp1.x, temp1.y, temp1.z);

		temp1 = VP * glm::vec4(vertices[2], 1.0);
		calPos(temp1, particle);
		vertices[2] = glm::vec3(temp1.x, temp1.y, temp1.z);

		temp1 = VP * glm::vec4(vertices[3], 1.0);
		calPos(temp1, particle);
		vertices[3] = glm::vec3(temp1.x, temp1.y, temp1.z);

		memcpy(mVertexAlloc + nVertexCount, vertices, sizeof(glm::vec3) * 4);

		nVertexCount += 4;
		nTriangleCount += 2;
	}
}

void ParticleSystem::ConstructFireMesh()
{
	nVertexCount = 0;
	nTriangleCount = 0;
	for (auto& particle : m_ParticlePool)
	{
		if (!particle)
		{
			continue;
		}
		if (!particle->Active)
			continue;

		float life = particle->LifeRemaining / particle->LifeTime;
		glm::vec4 color = glm::lerp(particle->ColorEnd, particle->ColorBegin, life);
		float size = glm::lerp(particle->SizeEnd, particle->SizeBegin, life);
		// Render
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3( particle->Position.x, particle->Position.y, particle->Position.z))
			* glm::rotate(glm::mat4(1.0f), particle->Rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

		vertices[0] = transform * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0);
		vertices[1] = transform * glm::vec4(0.5f, -0.5f, 0.0f, 1.0);
		vertices[2] = transform * glm::vec4(0.5f, 0.5f, 0.0f, 1.0);
		vertices[3] = transform * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0);

		memcpy(mVertexAlloc + nVertexCount, vertices, sizeof(glm::vec3) * 4);
		memcpy(mTexcoordAlloc + nVertexCount, mTexcoord, sizeof(float) * 2 * 4);
		nVertexCount += 4;
		nTriangleCount += 2;

		//endTime = clock();
		//std::cout << "All Time : " << (double)(endTime - startTime) << "ms" << std::endl;
	}
}

void ParticleSystem::OnRender(BaseRenderParam& scene)
{
	if (nVertexCount == 0)
	{
		return;
	}

	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	//GetDynamicRHI()->SetDepthWritableState(CC3DPiplelineState::DepthStateWriteEnable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
	//float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOff, blendFactor, 0xffffffff);

	pShaderParticle->UseShader();
	//scene.GetDoubleBuffer()->GetFBOA()->clearDepth(1.0f, 0.0f);
	RHIResourceCast(scene.GetDoubleBuffer().get())->GetFBOA()->clearDepth(1.0f, 0.0f);
	scene.GetDoubleBuffer()->BindFBOA();

	GET_SHADER_STRUCT_MEMBER(ParticleConstBuffer).SetTexture("MainTex", MainTex);

	quadVertexbuffer[0]->UpdateVertexBUffer((float*)mVertexAlloc, nVertexCount, sizeof(Vector3), sizeof(Vector3));
	quadVertexbuffer[1]->UpdateVertexBUffer((float*)mTexcoordAlloc, nVertexCount, sizeof(Vector2), sizeof(Vector2));

	quadIndexBuffer->UpdateIndexBuffer((uint16*)mIndexAlloc, nTriangleCount,  3 * sizeof(uint16), 0);

	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);
	GET_SHADER_STRUCT_MEMBER(ParticleConstBuffer).ApplyToAllBuffer();

	GetDynamicRHI()->DrawPrimitive(quadVertexbuffer, AttributeCount, quadIndexBuffer);

	//GetDynamicRHI()->SetDepthWritableState(CC3DPiplelineState::DepthStateWriteEnable);
	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	//GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
}

void ParticleSystem::Emit(const ParticleProps& particleProps)
{
	if (mInitPoolFlag && particleProps.particleType == "BubbleParticle")
	{
		m_ParticlePool.resize(m_ParticleCountAll);
		m_PoolIndex = particleProps.nCount - 1;
		for (int i = 0; i < m_ParticlePool.size(); i++)
		{
			m_ParticlePool[i] = new BubbleParticle();
		}
		mInitPoolFlag = false;
	}
	else if (mInitPoolFlag && particleProps.particleType == "FireParticle")
	{
		m_ParticlePool.resize(m_ParticleCountAll);
		m_PoolIndex = particleProps.nCount - 1;
		for (int i = 0; i < m_ParticlePool.size(); i++)
		{
			m_ParticlePool[i] = new FireParticle();
		}
		mInitPoolFlag = false;
	}

	Particle *particle = m_ParticlePool[m_PoolIndex];
	particle->Active = true;
	particle->Position = particleProps.Position;
	particle->StartPosition = particleProps.Position;
	particle->faceOri = particleProps.faceOri;
	particle->projMat = particleProps.projMat;
	particle->Rotation = CC3DRandom::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle->Velocity = particleProps.Velocity;
	float _dir = abs(particleProps.InitDir.x);
	if (_dir < 0.03)
	{
		particle->Velocity.x += particleProps.VelocityVariation.x * (CC3DRandom::Float() - 0.5f);  //初始方向完全随机
	}
	else
	{
		if (particleProps.InitDir.x > 0.0)
		{
			particle->Velocity.x += (particleProps.VelocityVariation.x * (1.0 + particleProps.InitDir.x)) * abs(CC3DRandom::Float() - 0.5f); //往屏幕右边随机
		}
		else
		{
			particle->Velocity.x += (-particleProps.VelocityVariation.x * (1.0 + abs(particleProps.InitDir.x))) * abs(CC3DRandom::Float() - 0.5f);//往屏幕左边随机
		}
	}

	if (particleProps.particleType == "BubbleParticle")
	{
		particle->Velocity.y += particleProps.VelocityVariation.y * - abs(CC3DRandom::Float() - 0.5f); //往屏幕上方随机
	}
	else if (particleProps.particleType == "FireParticle")
	{
		particle->Velocity.y += particleProps.VelocityVariation.y * + abs(CC3DRandom::Float() - 0.5f); //往屏幕下方随机

		//particle->EndPosition.x = particle->StartPosition.x + particle->Velocity.x * particle->LifeTime;//粒子轨迹的结束位置x
		//particle->EndPosition.y = particle->StartPosition.y + particle->Velocity.y * 2.0;//粒子轨迹的结束位置y

		//particle->ControlPosition.x = particle->StartPosition.x + (particle->EndPosition.x - particle->StartPosition.x) * 0.2;//贝赛尔二次曲线的控制点
		//particle->ControlPosition.y = particle->StartPosition.y + particle->Velocity.y * 4.0;
	}

	// Color
	particle->ColorBegin = particleProps.ColorBegin;
	particle->ColorEnd = particleProps.ColorEnd;

	particle->LifeTime = particleProps.LifeTime;
	particle->LifeRemaining = particleProps.LifeTime;
	particle->SizeBegin = particleProps.SizeBegin * particleProps.SizeControl + particleProps.SizeVariation * (CC3DRandom::Float() - 0.5f);
	particle->SizeEnd = particleProps.SizeEnd;

	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}

void ParticleSystem::SetTexcoord(float* texcoord)
{
	mTexcoord = texcoord;
}

void ParticleSystem::SetParticleCount(uint32_t nCount)
{
	m_ParticleCountAll = nCount;
}

void ParticleSystem::SetShaderResource(const std::string& path)
{
	pShaderParticle = GetDynamicRHI()->CreateShaderRHI();

	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		CCVetexAttribute pAttribute[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C3},
			{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
		};

		std::string path2 = path + "/Shader/3D/simpleparticle.fx";
		//pShaderParticle = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path2, pAttribute, 2, true);
		pShaderParticle->InitShader(path2, pAttribute, 2, true);

		uint16 indices[] = {
			0, 3, 1, 1, 3, 2
		};

		mVertexAlloc = new Vector3[m_ParticleCountAll * 4];
		memset(mVertexAlloc, 0, sizeof(Vector3) * m_ParticleCountAll * 4);

		mTexcoordAlloc = new Vector2[m_ParticleCountAll * 4];
		memset(mTexcoordAlloc, 0, sizeof(Vector2) * m_ParticleCountAll * 4);

		int vertexCount = 0;
		if (mTexcoord != nullptr)
		{
			for (int ni = 0; ni < m_ParticleCountAll; ni++)
			{
				memcpy(mTexcoordAlloc + vertexCount, mTexcoord, sizeof(float) * 2 * 4);
				vertexCount += 4;
			}
		}

		mIndexAlloc = new uint16[m_ParticleCountAll * 6];
		memset(mIndexAlloc, 0, sizeof(uint16) * m_ParticleCountAll * 6);
		vertexCount = 0;
		for (int ni = 0; ni < m_ParticleCountAll; ni++)
		{
			indices[0] = 0 + ni * 4;
			indices[1] = 3 + ni * 4;
			indices[2] = 1 + ni * 4;
			indices[3] = 1 + ni * 4;
			indices[4] = 3 + ni * 4;
			indices[5] = 2 + ni * 4;

			memcpy(mIndexAlloc + vertexCount, indices, sizeof(uint16) * 6);
			vertexCount += 6;
		}

		if (quadVertexbuffer[0] == nullptr)
		{
			quadVertexbuffer[0] = GetDynamicRHI()->CreateVertexBuffer((float*)mVertexAlloc, m_ParticleCountAll * 4, 3);
			quadVertexbuffer[1] = GetDynamicRHI()->CreateVertexBuffer((float*)mTexcoordAlloc, m_ParticleCountAll * 4, 2);
		}
		if (quadIndexBuffer == nullptr)
		{
			quadIndexBuffer = GetDynamicRHI()->CreateIndexBuffer((uint16*)mIndexAlloc, m_ParticleCountAll * 2);
		}
	}
	else
	{

	}


}

void ParticleSystem::SetMainTex(std::shared_ptr<CC3DTextureRHI> tex)
{
	MainTex = tex;
}

