#ifndef _CC3DSHADOW_RENDER_H_
#define _CC3DSHADOW_RENDER_H_

#include <memory>
#include <string>

#include "BaseDefine/Define.h"
#include "glm/detail/type_vec3.hpp"
#include "glm/detail/type_mat4x4.hpp"
#include "Common/CC3DUtils.h"
#include <d3d11.h>

class CC3DMesh;
class CC3DSceneManage;
class ShadowMapManager;
class CC3DBlurPass;
class DX11Texture;
class DX11Shader;
class CC3DRenderTargetRHI;
class CC3DConstantBuffer;

class CC3DShadowRender
{
public:
	struct ShadowMapConstantBuffer
	{
		glm::mat4 world;
		glm::mat4 meshMat;
		glm::mat4 lightSpaceMatrix;
		glm::mat4 meshMatInverse;
		int AnimationEnable = 0;
		Vector3 pad = { 0,0,0 };
	};

	struct ShadowMapSkinMatConstantBuffer
	{
		glm::mat4 BoneMat[MAX_MATRICES];
	};
public:
	CC3DShadowRender();
	~CC3DShadowRender();
public:
	void Bind(CC3DSceneManage& scene_manager, ShadowMapManager& shadowManager);
	void RenderMesh(const CC3DMesh& pMesh, bool withAnimation = false);
	void UnBind();

	void SetShaderResource(const std::string& path);
	void SetDirectionalLight(glm::vec3& dirLight);
	void SetAnimation(CC3DSceneManage& scene, CC3DMesh* pMesh,int nModel);
	void setModelMatrix(glm::mat4& mat)
	{
		mModel_matrix = mat;
	}

	void ProcessBlur();
	std::shared_ptr<CC3DRenderTargetRHI> GetShadowMap();
	glm::mat4& GetLightSpaceMatrix() { return mLightSpaceMatrix; }
public:

	DX11Shader* mShader = nullptr;
	std::shared_ptr<CC3DBlurPass> mBlurPass;
	std::shared_ptr<CC3DRenderTargetRHI> mDepthRenderBuffer;

	ShadowMapConstantBuffer mShadowMapCBData;
	ShadowMapSkinMatConstantBuffer mShadowMapSkinMatCBData;
	std::shared_ptr<CC3DConstantBuffer> mShadowMapCB ;
	std::shared_ptr<CC3DConstantBuffer> mShadowMapSkinMatCB ;

	glm::vec3 mLightDir{ 0.0f, 0.0f, 1.0f };
	glm::vec3 mLightPos{ 1.0f, -1.0f, 1.0f };
	glm::vec3 mLightLookAt{ 0.0, 0.0f, -1.0f };
	glm::vec3 mLightUp{ 0.0, 0.0f, -1.0f };

	glm::mat4 mLightSpaceMatrix;
	glm::mat4 mModel_matrix = glm::mat4();
	glm::vec2 mNearAndFar = glm::vec2(0.0f);

};

#endif