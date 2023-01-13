#include "CParticleEffect.h"
#include "Common/CC3DEnvironmentConfig.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include <mutex>
#include "Toolbox/HeaderModelForCull.h"

CParticleEffect::CParticleEffect()
{
	mParticle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	mParticle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	mParticle.SizeBegin = 0.2f, mParticle.SizeVariation = 0.3f, mParticle.SizeEnd = 0.5f;
	mParticle.SizeControl = 1.0f;
	mParticle.LifeTime = 5.0f;
	mParticle.Velocity = { 0.0f, 0.0f };
	mParticle.VelocityVariation = { 300.0f, 100.0f };
	mParticle.InitDir = { 0.0, 0.0 };
	mParticle.Position = { 0.0f, 0.0f, 0.0f };
	mParticle.faceOri = { 0.0, 0.0, 1.0, 1.0 };

}


CParticleEffect::~CParticleEffect()
{
	Release();
}

bool CParticleEffect::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath, const std::string& path)
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
					MainTex = GetDynamicRHI()->CreateTextureFromFile(szName, false);
				}
				else
				{
					MainTex = GetDynamicRHI()->CreateTextureFromZip(hZip, szName, false);
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
			return true;
		}
	}
	return false;
}

bool CParticleEffect::ReadConfig(XMLNode& childNode, HZIP hZip /*= 0*/, char* pFilePath /*= NULL*/)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	return ReadConfig(childNode, hZip, pFilePath, "");
}

bool CParticleEffect::ReadConfig(XMLNode& childNode, const std::string& path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CParticleEffect::Prepare()
{
	float texcoord[8] = {
		0.0, 0.0,
		0.14286, 0.0,
		0.14286,0.14286,
		0.0, 0.14286
	};

	if (mParticleSystem == nullptr)
	{
		mParticleSystem = std::make_shared<ParticleSystem>();
		
		mParticleSystem->SetParticleCount(mParticle.nCount);
		mParticleSystem->SetTexcoord(texcoord);

		mParticleSystem->SetShaderResource(m_resourcePath);

	}
	if (MainTex)
	{
		mParticleSystem->SetMainTex(MainTex);
	}

	
	return true;
}

void CParticleEffect::Render(BaseRenderParam &RenderParam)
{
	int nFaceCount = RenderParam.GetFaceCount();
	if (nFaceCount == 0)
	{
		return;
	}

	if (mParticleSystem == nullptr)
	{
		return;
	}
	
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();

	Vector4 faceOriRect = Vector4(0.0, 0.0, 1.0, 1.0);
	{
		FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(0);
		{
			faceOriRect.x = pFaceInfo->pFaceRect.x*1.f / width;
			faceOriRect.y = pFaceInfo->pFaceRect.y*1.f / height;
			faceOriRect.z = pFaceInfo->pFaceRect.width*1.f / width;
			faceOriRect.w = pFaceInfo->pFaceRect.height*1.f / height;
		}
	}
	glm::vec3 mousePos;
	glm::mat4 viewMat;
	glm::mat4 projMat;
	std::tuple<bool, float> mouseOpenParam;
	float xLen = 1.0f;
	if (nFaceCount > 0)
	{
		//for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
		{
			//只使用第一个人脸
			auto pFaceInfo = RenderParam.GetFaceInfo(0);

			Vector2 *pFacePoint = RenderParam.GetFacePoint(0, FACE_POINT_130);
			if (pFacePoint)
			{
				mouseOpenParam = isMouthOpened(pFacePoint);
			}
			
			float fAngle = atan(0.5) * 2;
			if (0.5 * height > width)
			{
				fAngle = atan(0.5*height /width) * 2;
			}
			glm::mat4 matView = glm::lookAtLH(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f));
			fAngle = glm::radians(45.0);
			glm::mat4 matProjDX = glm::perspectiveLH(fAngle, pFaceInfo->pFaceRect.width*1.f / pFaceInfo->pFaceRect.height, 10.0f, 2000.f);

			viewMat = matView;
			projMat = matProjDX;

			GetDynamicRHI()->SetViewPort(0, 0, width, height);

			//矩阵变换信息
			glm::mat4 matRotateXYZ = glm::mat4(1.0f);
			memcpy((float *)(&matRotateXYZ[0][0]), pFaceInfo->arrWMatrix, sizeof(float) * 16);
			matRotateXYZ = glm::rotate(matRotateXYZ, glm::radians(m_rotateX), glm::vec3(1.0f, 0.f, 0.f));
			glm::mat4 matScale2 = glm::scale(glm::mat4(1.0f), glm::vec3(m_MatScaleHead));

			glm::vec3 emitDir = matRotateXYZ * glm::vec4(0.0, 0.0, 1.0, 0.0);
			xLen = glm::dot(emitDir, glm::vec3(1.0, 0.0, 0.0));
			float yLen = glm::dot(emitDir, glm::vec3(0.0, 1.0, 0.0));

			glm::mat4 matWVP = matProjDX * matView* matRotateXYZ*matScale2;
			glm::vec4 temPos = matRotateXYZ * glm::vec4(g_headerModelForCullVertices[1986], g_headerModelForCullVertices[1987], g_headerModelForCullVertices[1988], 1.0);
			mousePos = temPos / temPos.w;
		}
	}

	if (std::get<0>(mouseOpenParam))
	{
		float initSize = (std::min)(1.0f, std::get<1>(mouseOpenParam));
		Emit(mousePos, faceOriRect, projMat, initSize, Vector2(xLen, 0.0));
	}
	Update();
	
	mParticleSystem->ConstructBubbleMesh(viewMat);

	////设置混合方式
	//float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOn, blendFactor, 0xffffffff);
	
	mParticleSystem->OnRender(RenderParam);
	
	//GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
}

void CParticleEffect::Release()
{
	
}

void CParticleEffect::Emit(glm::vec3 emitPos, Vector4 faceOri, glm::mat4& projMat, float initSize, Vector2 initDir)
{
	mParticle.Position = emitPos;//根据脸部坐标给粒子初始位置值
	mParticle.faceOri = faceOri;
	mParticle.projMat = projMat;
	mParticle.SizeControl = initSize;
	mParticle.InitDir = glm::vec2(initDir.x, initDir.y);

	for (int i = 0; i < 1; i++)
	{
		mParticleSystem->Emit(mParticle);
	}
}

void CParticleEffect::Update()
{
	for (int i = 0; i < mParticleSystem->m_ParticlePool.size(); i++)
	{
		BubbleParticle* particle = (BubbleParticle*)mParticleSystem->m_ParticlePool[i];
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

std::tuple<bool, float> CParticleEffect::isMouthOpened(Vector2 *pFacePoints, float param)
{
	//计算上下唇的距离相加取中值
	bool Ist = 0;
	float upans = pFacePoints[89].distance(pFacePoints[100]);
	float downans = pFacePoints[95].distance(pFacePoints[104]);
	float ans = (upans + downans) / 2.0f;
	//嘴唇中间距离
	float midans = pFacePoints[100].distance(pFacePoints[104]);
	//计算嘴唇长度
	float lengthans = pFacePoints[98].distance(pFacePoints[102]) / 4.5f;
	if (lengthans < ans) {
		ans = lengthans;
	}
	//比例
	float ratio = midans / ans;
	if (ratio > param) {
		Ist = 1;
	}
	return { Ist, ratio };
}