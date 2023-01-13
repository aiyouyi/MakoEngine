#ifndef _CC3DSHADOW_RENDER_H_
#define _CC3DSHADOW_RENDER_H_

#include "Toolbox/Render/CC3DShaderDef.h"

namespace CC3DImageFilter
{
	class CC3DSceneManage;
	class ShadowMapManager;
}

class CC3DMesh;

class CC3DBlurPass;
class CC3DRenderTargetRHI;
class ShaderRHI;

BEGIN_SHADER_STRUCT(ShadowMapConst1, 0)
	DECLARE_PARAM(glm::mat4, lightSpaceMatrix)
	DECLARE_PARAM(glm::mat4, meshMat)
	DECLARE_PARAM(glm::mat4, meshMatInverse)
	DECLARE_PARAM(glm::mat4, model)
	DECLARE_PARAM_VALUE(int, AnimationEnable, false)
	DECLARE_PARAM_VALUE(float, Alpha, 1.0)
	DECLARE_PARAM(Vector2, pad)
	BEGIN_STRUCT_CONSTRUCT(ShadowMapConst1)
		IMPLEMENT_PARAM("lightSpaceMatrix", UniformType::MAT4)
		IMPLEMENT_PARAM("meshMat", UniformType::MAT4)
		IMPLEMENT_PARAM("meshMatInverse", UniformType::MAT4)
		IMPLEMENT_PARAM("model", UniformType::MAT4)
		IMPLEMENT_PARAM("AnimationEnable", UniformType::INT)
		IMPLEMENT_PARAM("alpha", UniformType::FLOAT)
		IMPLEMENT_PARAM("pad", UniformType::INT2)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(ShadowMapConst2, 1)
	DECLARE_ARRAY_PARAM(glm::mat4, MAX_MATRICES, BoneMat)
	BEGIN_STRUCT_CONSTRUCT(ShadowMapConst2)
		IMPLEMENT_ARRAY_PARAM("gBonesMatrix", UniformType::MAT4, MAX_MATRICES)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class CC3DShadowRender
{
public:
	CC3DShadowRender();
	~CC3DShadowRender();
public:
	void Bind(CC3DImageFilter::CC3DSceneManage& scene_manager, CC3DImageFilter::ShadowMapManager& shadowManager);
	void RenderMesh(const CC3DMesh& pMesh, int withAnimation = false);
	void UnBind();

	void SetShaderResource(const std::string& path);
	void SetDirectionalLight(glm::vec3& dirLight);
	void SetAnimation(CC3DImageFilter::CC3DSceneManage& scene, CC3DMesh* pMesh,int nModel);
	void setModelMatrix(glm::mat4& mat)
	{
		mModel_matrix = mat;
	}

	void ProcessBlur();
	std::shared_ptr<CC3DRenderTargetRHI> GetShadowMap();
	glm::mat4& GetLightSpaceMatrix() { return mLightSpaceMatrix; }
	float ProjectShadow = 1.0;

private:

	std::shared_ptr< ShaderRHI> mShader;
	std::shared_ptr<CC3DBlurPass> mBlurPass;
	std::shared_ptr<CC3DRenderTargetRHI> mDepthRenderBuffer;

	DECLARE_SHADER_STRUCT_MEMBER(ShadowMapConst1);
	DECLARE_SHADER_STRUCT_MEMBER(ShadowMapConst2);

	glm::vec3 mLightDir{ 0.0f, 0.0f, 1.0f };
	glm::vec3 mLightPos{ 1.0f, -1.0f, 1.0f };
	glm::vec3 mLightLookAt{ 0.0, 0.0f, -1.0f };
	glm::vec3 mLightUp{ 0.0, 0.0f, -1.0f };

	glm::mat4 mLightSpaceMatrix;
	glm::mat4 mModel_matrix = glm::mat4();
	glm::vec2 mNearAndFar = glm::vec2(0.0f);
	

};

#endif