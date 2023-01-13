#ifndef _CC_ANIMATION_BLEND_H_
#define _CC_ANIMATION_BLEND_H_

#include "inc.h"

class CC3DModel;

namespace CC3DImageFilter
{
	class CC3DAnimate;

	class CC3DSkeleton;
	struct AnimBlendInfo;

	struct BlendState
	{
		std::string maskBoneName;
		float blendWeight = 1.0;
	};

	class LayeredBlend
	{
	public:
		LayeredBlend();
		~LayeredBlend();
		std::shared_ptr<CC3DAnimate> blendPos;
		BlendState blendState;
	};

	class CC3DAnimationBlend
	{
	public:
		CC3DAnimationBlend();
		~CC3DAnimationBlend();
		void AddBlendAnimation(std::shared_ptr<CC3DAnimate> blendAnim, const std::string& relevantAnime, const std::string& boneName, float blendWeight = 1.0f);
		AnimBlendInfo* ApplyBlendNode(double during, CC3DModel* pModel, CC3DSkeleton* pSkeleton, const std::string animateName, AnimBlendInfo* baseAnimResult);

		AnimBlendInfo* ApplyLayeredBlendPerBone(AnimBlendInfo* basePos, AnimBlendInfo* blendPos, BlendState blendState);
	private:
		std::unordered_map<std::string, std::shared_ptr<LayeredBlend>> blendAnimMap;
	};
}



#endif