#include "CC3DAnimationBlend.h"
#include "CC3DAnimate.h"
#include "CC3DAnimateTimeline.h"
#include "CC3DEngine/Model/CC3DModel.h"
#include "CC3DEngine/Skeleton/CC3DSkeleton.h"
#include "BaseDefine/Define.h"

namespace CC3DImageFilter
{
	LayeredBlend::LayeredBlend()
	{

	}

	LayeredBlend::~LayeredBlend()
	{

	}

	CC3DAnimationBlend::CC3DAnimationBlend()
	{

	}

	CC3DAnimationBlend::~CC3DAnimationBlend()
	{
		blendAnimMap.clear();
	}

	void CC3DAnimationBlend::AddBlendAnimation(std::shared_ptr<CC3DAnimate> blendAnim, const std::string& relevantAnime, const std::string& boneName, float blendWeight)
	{
		auto initLayerBlend = [&]()
		{
			std::shared_ptr<LayeredBlend> layerBlend = std::make_shared<LayeredBlend>();
			layerBlend->blendPos = blendAnim;
			BlendState state;
			state.blendWeight = blendWeight;
			state.maskBoneName = boneName;
			layerBlend->blendState = state;
			blendAnimMap[relevantAnime] = layerBlend;
		};
		if (blendAnimMap.size() == 0)
		{
			initLayerBlend();
			return;
		}

		auto itr = blendAnimMap.find(relevantAnime);
		if (itr != blendAnimMap.end())
		{
			itr->second.reset();
			initLayerBlend();
		}

	}

	AnimBlendInfo* CC3DAnimationBlend::ApplyBlendNode(double during, CC3DModel* pModel, CC3DSkeleton* pSkeleton, const std::string animateName, AnimBlendInfo* baseAnimResult)
	{
		if (blendAnimMap.empty())
		{
			return nullptr;
		}

		AnimBlendInfo* blendResult = nullptr;

		auto itr = blendAnimMap.find(animateName);
		if (itr != blendAnimMap.end())
		{
			auto blendPos = itr->second->blendPos;
			BlendState state = itr->second->blendState;
			AnimBlendInfo blendAnimeResult;
			blendAnimeResult.maskBoneName = state.maskBoneName;

			int nTemp = during / blendPos->endTime;
			double AnimateDuring = during - nTemp * (double)blendPos->endTime;
			blendPos->play(AnimateDuring, pModel, pSkeleton, blendAnimeResult);

			if (blendAnimeResult.boneNodeMap.size() != 6)
			{
				int kk = 0;
			}

			//Ö´ÐÐÀàËÆUEµÄLayered blend per bone
			ApplyLayeredBlendPerBone(baseAnimResult, &blendAnimeResult, state);
		}

		return baseAnimResult;
	}

	AnimBlendInfo* CC3DAnimationBlend::ApplyLayeredBlendPerBone(AnimBlendInfo* basePos, AnimBlendInfo* blendPos, BlendState blendState)
	{
		if (basePos == nullptr || blendPos == nullptr)
		{
			return basePos;
		}


		for (auto iter = basePos->boneNodeMap.begin(); iter != basePos->boneNodeMap.end(); iter++)
		{
			auto dstItr = blendPos->boneNodeMap.find(iter->first);
			if (dstItr != blendPos->boneNodeMap.end())
			{
				TRSInfo& src = iter->second;
				TRSInfo& dst = dstItr->second;
				if (src.EnableRotate)
				{
					CC3DUtils::QuaternionInterpolate(src.Rotation, src.Rotation, dst.Rotation, blendState.blendWeight);
				}
				if (src.EnableScale)
				{
					src.Scale = src.Scale * (1.0 - blendState.blendWeight) + dst.Scale * blendState.blendWeight;
				}
				if (src.EnableTranslate)
				{
					src.Translate = src.Translate * (1.0 - blendState.blendWeight) + dst.Translate * blendState.blendWeight;
				}
			}
		}

		return basePos;
	}

}
