#include "CHandParticleEffect.h"
#include "Common/CC3DEnvironmentConfig.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/MaterialTexRHI.h"
#include <mutex>
#include "ToolBox/CC3DRandom.h"
#include <iostream>

CHandParticleEffect::CHandParticleEffect()
{
	mParticle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	mParticle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	mParticle.SizeBegin = 0.22f, mParticle.SizeVariation = 0.03f, mParticle.SizeEnd = 0.15f;
	mParticle.SizeControl = 1.0f;
	mParticle.LifeTime = 3.0f;
	mParticle.Velocity = { 0.0f, 0.0f };
	mParticle.VelocityVariation = { 1.0f,2.0f };
	mParticle.InitDir = { 0.0, 0.0 };
	mParticle.Position = { 0.0f, 0.0f, 0.0f };
	mParticle.faceOri = { 0.0, 0.0, 1.0, 1.0 };
	GPUParticle = std::make_shared<GPUParticleEffect>();
	AMDGPUParticle = std::make_shared<AMDGPUParticleEffect>();
}


CHandParticleEffect::~CHandParticleEffect()
{
	Release();
}

void CHandParticleEffect::Release()
{

}

void CHandParticleEffect::DoReady()
{
	if (mParticleSystem == nullptr)
	{
		mParticleSystem = std::make_shared<ParticleSystem>();

		mParticleSystem->SetTexcoord(mTexcoord[0]);
		mParticleSystem->SetParticleCount(mParticle.nCount);
		mParticleSystem->SetShaderResource(m_resourcePath);
	}

	MainTex->UpdateTexture();

	if (mParticleSystem && MainTex)
	{
		mParticleSystem->SetMainTex(MainTex->GetTextureRHI());
	}

	BlendAddOn = GetDynamicRHI()->CreateBlendState(CC3DBlendState::BP_SRCALPHA, CC3DBlendState::BP_ONE);


	GPUParticle->SetShaderResource(m_resourcePath, MainTex);
	AMDGPUParticle->SetShaderResource(m_resourcePath, MainTex);
}

bool CHandParticleEffect::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath, const std::string& path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeMaterial = childNode.getChildNode("drawable", 0);
		if (!nodeMaterial.isEmpty())
		{
			const char* szName = nodeMaterial.getAttribute("Material");
			if (szName != nullptr)
			{
				if (hZip == 0)
				{
					std::string materialPath = path + "/" + std::string(szName);
					MainTex = GetDynamicRHI()->CreateAsynTextureFromFile(materialPath, false);
				}
				else
				{
					MainTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szName, false);
				}
			}
			const char* szType = nodeMaterial.getAttribute("ParticleType");
			if (szType != nullptr)
			{
				mParticle.particleType = szType;
			}
			const char* szLiftTime = nodeMaterial.getAttribute("LifeTime");
			if (szLiftTime != nullptr)
			{
				sscanf(szLiftTime, "%f", &mParticle.LifeTime);
			}
			const char* szVelocity = nodeMaterial.getAttribute("Velocity");
			if (szVelocity != nullptr)
			{
				sscanf(szVelocity, "%f,%f", &mParticle.VelocityVariation.x, &mParticle.VelocityVariation.y);
			}
			const char* szCount = nodeMaterial.getAttribute("ParticleCount");
			if (szCount != nullptr)
			{
				sscanf(szCount, "%d", &mParticle.nCount);
			}
		}
		return true;
	}
	return false;
}

bool CHandParticleEffect::ReadConfig(XMLNode& childNode, HZIP hZip /*= 0*/, char* pFilePath /*= NULL*/)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);

	return ReadConfig(childNode, hZip, pFilePath, "");
}

bool CHandParticleEffect::ReadConfig(XMLNode& childNode, const std::string& path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CHandParticleEffect::Prepare()
{
	NeedPrepare = true;
	return true;
}

void CHandParticleEffect::Render(BaseRenderParam& RenderParam)
{
	int nHandCount = RenderParam.GetHandCount();

	if (NeedPrepare)
	{
		DoReady();
		NeedPrepare = false;
	}

	if (mParticleSystem == nullptr)
	{
		return;
	}

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	Vector2 ScaleSize(1.0 / width, 1.0 / height);

	bool HasHand = nHandCount > 0;
	for(int index = 0; index < nHandCount; index++)
	{
		Vector2 pHeartPoint;
		auto pHandInfo = RenderParam.GetGestureInfo(index);
		pHeartPoint.x = (pHandInfo->handPoint[0].x + pHandInfo->handPoint[4].x) * 0.5;
		pHeartPoint.y = (pHandInfo->handPoint[0].y + pHandInfo->handPoint[4].y) * 0.5;

		if (UseGPU)
		{
			if (UseAMDGPU)
			{
				AMDGPUParticle->Update(pHeartPoint, RenderParam);
			}
			else
			{
				GPUParticle->Update(pHeartPoint, RenderParam);
			}
			
		}
		else
		{
			pHeartPoint = pHeartPoint * ScaleSize * 2.0 - Vector2(1.0, 1.0);
			glm::vec3 heartEmitPos = glm::vec3(pHeartPoint.x, pHeartPoint.y, 0.5);
			Vector2 initDir = Vector2(0.02, 0.0);//粒子发射后的随机偏移的左右方向

			Vector4 faceOriRect = Vector4(0.0, 0.0, 1.0, 1.0);
			glm::mat4 projMat = glm::mat4(1.0);

			Emit(heartEmitPos, faceOriRect, projMat, 1.0, initDir);
		}
	}

	if (UseGPU)
	{
		if (UseAMDGPU)
		{
			if (!HasHand)
			{
				AMDGPUParticle->Update(Vector2(-1.f, -1.f), RenderParam);
			}

			AMDGPUParticle->Render(RenderParam);
		}
		else
		{
			if (!HasHand)
			{
				GPUParticle->Update(Vector2(-1.f, -1.f), RenderParam);
			}

			GPUParticle->Render(RenderParam);
		}

	}
	else
	{
		Update();

		int randIndex = (int)(CC3DRandom::Float() * 1000) % 9;
		mParticleSystem->SetTexcoord(mTexcoord[randIndex]);
		mParticleSystem->ConstructFireMesh();

		//设置混合方式
		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		GetDynamicRHI()->SetBlendState(BlendAddOn, blendFactor, 0xffffffff);

		mParticleSystem->OnRender(RenderParam);
		GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
	}

}

void CHandParticleEffect::Emit(glm::vec3 emitPos, Vector4 faceOri, glm::mat4& projMat, float initSize, Vector2 initDir)
{
	mParticle.Position = emitPos;
	mParticle.faceOri = faceOri;
	mParticle.projMat = projMat;
	mParticle.SizeControl = initSize;
	mParticle.InitDir = glm::vec2(initDir.x, initDir.y);

	for (int i = 0; i < 1; i++)
	{
		mParticleSystem->Emit(mParticle);;
	}
}

void CHandParticleEffect::Update()
{
	for (int i = 0; i < mParticleSystem->m_ParticlePool.size(); i++)
	{
		FireParticle* particle = (FireParticle*)mParticleSystem->m_ParticlePool[i];
		if (particle)
		{
			if (!particle->Active)
				continue;

			if (particle->LifeRemaining <= 0.0f)
			{
				particle->Active = false;
				continue;
			}
			particle->Update(CC3DEnvironmentConfig::getInstance()->deltaTime);
		}

	}
	//mParticleSystem->OnUpdate(CC3DEnvironmentConfig::getInstance()->deltaTime);
}
