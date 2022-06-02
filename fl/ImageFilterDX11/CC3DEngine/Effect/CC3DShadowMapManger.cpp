#include "CC3DShadowMapManger.h"
#include "Scene/CC3DSceneManage.h"
#include "Effect/CC3DShadowMap.h"

void ShadowMapManager::setShadowCascades(size_t cascades) noexcept
{
	if (cascades > CONFIG_MAX_SHADOW_CASCADES)
		return;

	for ( int ni = 0; ni < cascades; ni++ )
	{
		cascadeShadowMaps.emplace_back(new ShadowMap());
	}
}

void ShadowMapManager::update(CC3DSceneManage& manager)
{
	updateCascadeShadowMaps(manager);
}

ShadowMap* ShadowMapManager::getShadowMap(int index)
{
	if (index > cascadeShadowMaps.size())
	{
		return nullptr;
	}

	return cascadeShadowMaps[index];
}

void ShadowMapManager::updateCascadeShadowMaps(CC3DSceneManage& manager)
{
	ShadowMap::CascadeParameters cascadeParams;

	ShadowMap::computeSceneCascadeParams(manager, cascadeParams);

	ShadowMap& map = *cascadeShadowMaps[0];
	map.update(cascadeParams);
}
