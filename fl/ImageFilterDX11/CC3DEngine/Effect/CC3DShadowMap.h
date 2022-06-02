#ifndef _CC_SHADOWMAP_H_
#define _CC_SHADOWMAP_H_

#include "BaseDefine/Vectors.h"
#include "BaseDefine/Aabb.h"

#include "Scene/CC3DSceneManage.h"

class ShadowMap
{
public:
	explicit ShadowMap() noexcept{}
	~ShadowMap(){}

	using float2 = glm::vec2;
	using float3 = glm::vec3;
	using float4 = glm::vec4;
	using mat4f = glm::mat4;

	struct ShadowMapLayout 
	{
		// the smallest increment in depth precision
		// e.g., for 16 bit depth textures, is this 1 / (2^16)
		float zResolution = 0.0f;

		// the dimension of the encompassing texture atlas
		size_t atlasDimension = 0;

		// the dimension of a single shadow map texture within the atlas
		// e.g., for at atlas size of 1024 split into 4 quadrants, textureDimension would be 512
		size_t textureDimension = 0;

		// the dimension of the actual shadow map, taking into account the 1 texel border
		// e.g., for a texture dimension of 512, shadowDimension would be 510
		size_t shadowDimension = 0;
	};

	struct CascadeParameters {
		// The near and far planes, in clip space, to use for this shadow map
		float2 csNearFar = { -1.0f, 1.0f };

		// The following fields are set by computeSceneCascadeParams.

		// Light-space near/far planes for the scene.
		float2 lsNearFar;

		// View-space near/far planes for the scene.
		float2 vsNearFar;
		CC3DAabb wsShadowCastersVolume;
		CC3DAabb wsShadowReceiversVolume;

		// Position of the directional light in world space.
		float3 wsLightPosition;
	};

	static void computeSceneCascadeParams(CC3DSceneManage& manager, CascadeParameters& cascadeParams);
	
	template<typename Caster>
	static void visitScene(CC3DSceneManage& scene, float3 lightPosition, Caster casters) noexcept;


	static inline float2 computeNearFar(mat4f const& view, CC3DAabb const& wsShadowCastersVolume) noexcept;

	static inline float2 computeNearFar(mat4f const& view, float3 const* wsVertices, size_t count) noexcept;

	void update(const CascadeParameters& cascadesParams) noexcept;

	static float RayPlaneIntersect(float3 rayOrigin, float3 rayDirection, float3 planeOrigin, float3 planeNormal)
	{
		float dist = dot(planeNormal, planeOrigin - rayOrigin) / dot(planeNormal, rayDirection);
		return dist;
	}
	static void calculateNearFar(CC3DSceneManage& scene, CascadeParameters& cascadeParams);
public:
	float lsNear;
	float lsFar;
};



template<typename Caster>
void ShadowMap::visitScene(CC3DSceneManage& scene, float3 lightPosition, Caster casters) noexcept
{
	CC3DModel* render_model = scene.m_Model[0];
	if (render_model)
	{
		glm::mat4 view = scene.m_Camera.m_ViewMatrix;
		//TODO:这里必须是相机作为坐标系原点，模型的位置坐标必须是相对与相机的
		//计算世界空间坐标系下模型的包围盒大小
		mat4f toWsMat = scene.m_ModelControl.GetModelMatrix();
		mat4f worldToOrign = mat4f(1.0f);
		worldToOrign = glm::translate(worldToOrign, -float3(0.0, 0.0, LIGHT_DISTANCE));
		float3 minPoint = toWsMat * float4(render_model->m_ModelBox.minPoint.x, render_model->m_ModelBox.minPoint.y, render_model->m_ModelBox.minPoint.z, 1.0f);
		float3 maxPoint = toWsMat * float4(render_model->m_ModelBox.maxPoint.x, render_model->m_ModelBox.maxPoint.y, render_model->m_ModelBox.maxPoint.z, 1.0f);


		CC3DAabb wsVolume{ float3(render_model->m_ModelBox.minPoint.x, render_model->m_ModelBox.minPoint.y, render_model->m_ModelBox.minPoint.z),
					   float3(render_model->m_ModelBox.maxPoint.x, render_model->m_ModelBox.maxPoint.y, render_model->m_ModelBox.maxPoint.z) };
		const CC3DAabb::Corners wsSceneCorners = wsVolume.getCorners();
		float3 min{ std::numeric_limits<float>::max() }, max{ std::numeric_limits<float>::lowest() };
		for (size_t i = 0; i < wsSceneCorners.size(); i++)
		{
			float3 temp = toWsMat * float4(wsSceneCorners.vertices[i], 1.0f);
			min.x = std::min(min.x, temp.x);
			min.y = std::min(min.y, temp.y);
			min.z = std::min(min.z, temp.z);

			max.x = std::max(max.x, temp.x);
			max.y = std::max(max.y, temp.y);
			max.z = std::max(max.z, temp.z);
		}

		
		//CC3DModel* render_model = scene.m_Model[0];
		float3 lightDir;
		CC3DDirectionalLight *pLight = (CC3DDirectionalLight*)scene.m_Light[0];
		if (pLight != nullptr)
		{
			lightDir = pLight->m_LightDirection;
			lightDir = glm::normalize(lightDir);
		}

		float3 lightPos = float3(0.0f) + (-lightDir * LIGHT_DISTANCE);
		float disMin = FLT_MAX, disMax = -FLT_MAX;
		for (size_t i = 0; i < wsSceneCorners.size(); i++)
		{
			float3 temp = toWsMat * float4(wsSceneCorners.vertices[i], 1.0f);
			float d = RayPlaneIntersect(temp, -lightDir, lightPos, lightDir);

			disMin = std::min(disMin, d);
			disMax = std::max(disMax, d);
		}
		

		const CC3DAabb aabb{ min, max };
		casters(aabb);
	}
}

#endif