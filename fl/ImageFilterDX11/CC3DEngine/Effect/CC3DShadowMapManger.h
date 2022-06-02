#ifndef _CC_SHADOWMAP_MANAGER_H_
#define _CC_SHADOWMAP_MANAGER_H_

#include <vector>
#include "BaseDefine/Define.h"
using namespace std;

constexpr size_t CONFIG_MAX_SHADOW_CASCADES = 4;

class CC3DSceneManage;
class ShadowMap;

class ShadowMapManager
{
public:
	explicit ShadowMapManager(){}

	~ShadowMapManager()
	{
		for (int ni = 0; ni < cascadeShadowMaps.size(); ni++ )
		{
			if (cascadeShadowMaps[ni])
			{
				SAFE_DELETE(cascadeShadowMaps[ni]);
			}
		}
	}

	//只允许调用一次
	void setShadowCascades(size_t cascades) noexcept;

	void update(CC3DSceneManage& manager);

	ShadowMap* getShadowMap(int index);

private:
	void updateCascadeShadowMaps(CC3DSceneManage& manager);
	vector<ShadowMap*> cascadeShadowMaps;
};


#endif