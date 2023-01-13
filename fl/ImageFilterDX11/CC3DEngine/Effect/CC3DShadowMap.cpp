#include "CC3DShadowMap.h"
#include <Scene/CC3DLight.h>

namespace CC3DImageFilter
{

	void ShadowMap::computeSceneCascadeParams(CC3DSceneManage& manager, CascadeParameters& cascadeParams)
	{
		glm::vec3 lightDir{ 0.0f, 0.0f, 1.0f };
		CC3DDirectionalLight* pLight = (CC3DDirectionalLight*)manager.m_Light[0];
		if (pLight != nullptr)
		{
			lightDir = pLight->m_LightDirection;
			lightDir = glm::normalize(lightDir);
		}
		cascadeParams.wsLightPosition = manager.m_Camera.GetCameraPos();

		//const float3 lightlookAt = float3(0.0f);
		//const float3 lightPosition = lightlookAt + lightDir * 4.0f;
		//const glm::mat4 M = glm::lookAt(lightPosition, lightlookAt, float3(0.0f, 1.0f, 0.0f));
		//const glm::mat4 MInv = glm::transpose(M);

		const float3 lightPosition = glm::vec3(0.0f);
		const float3 dir = lightDir;

		float3 upDir{ 0.0f, 1.0f, 0.0f };
		float3 z_axis = glm::normalize((lightPosition + dir) - lightPosition);
		if (abs(glm::dot(z_axis, upDir)) > 0.999)
		{
			upDir = { upDir.z, upDir.x,upDir.y };
		}

		const mat4f M = glm::lookAt(lightPosition, lightPosition + dir, upDir);
		const mat4f Mv = glm::transpose(M);

		cascadeParams.lsNearFar = { std::numeric_limits<float>::lowest(), (std::numeric_limits<float>::max)() };
		cascadeParams.vsNearFar = { std::numeric_limits<float>::lowest(), (std::numeric_limits<float>::max)() };
		cascadeParams.wsShadowCastersVolume = {};
		cascadeParams.wsShadowReceiversVolume = {};

		//visitScene(manager,lightPosition,
		//	[&](Aabb caster)
		//	{
		//	cascadeParams.wsShadowCastersVolume.min =
		//		min(cascadeParams.wsShadowCastersVolume.min, caster.min);
		//	cascadeParams.wsShadowCastersVolume.max =
		//		max(cascadeParams.wsShadowCastersVolume.max, caster.max);
		//	float2 nf = computeNearFar(Mv, caster);
		//	cascadeParams.lsNearFar.x = std::max(cascadeParams.lsNearFar.x, nf.x);  // near
		//	cascadeParams.lsNearFar.y = std::min(cascadeParams.lsNearFar.y, nf.y);  // far
		//	}
		//);

		calculateNearFar(manager, cascadeParams);


	}

	ShadowMap::float2 ShadowMap::computeNearFar(mat4f const& view, CC3DAabb const& wsShadowCastersVolume) noexcept
	{
		const CC3DAabb::Corners wsSceneCastersCorners = wsShadowCastersVolume.getCorners();
		return computeNearFar(view, wsSceneCastersCorners.data(), wsSceneCastersCorners.size());
	}

	ShadowMap::float2 ShadowMap::computeNearFar(mat4f const& view, float3 const* wsVertices, size_t count) noexcept
	{
		float2 nearFar = { std::numeric_limits<float>::lowest(), (std::numeric_limits<float>::max)() };
		for (size_t i = 0; i < count; i++)
		{
			float4 r = view * float4(wsVertices[i], 1.0f);
			float c = (r / r.w).z;
			nearFar.x = (std::max)(nearFar.x, c);  // near
			nearFar.y = (std::min)(nearFar.y, c);  // far
		}

		return nearFar;
	}

	void ShadowMap::update(const CascadeParameters& cascadesParams) noexcept
	{
		lsFar = -cascadesParams.lsNearFar.y;
		lsNear = -cascadesParams.lsNearFar.x;

		lsFar = cascadesParams.lsNearFar.x;
		lsNear = cascadesParams.lsNearFar.y;


	}

	void ShadowMap::calculateNearFar(CC3DSceneManage& scene, CascadeParameters& cascadeParams)
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

			float3 lightDir;
			CC3DDirectionalLight* pLight = (CC3DDirectionalLight*)scene.m_Light[0];
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

				disMin = (std::min)(disMin, d);
				disMax = (std::max)(disMax, d);
			}

			cascadeParams.lsNearFar.y = disMin;
			cascadeParams.lsNearFar.x = disMax;
		}
	}
}