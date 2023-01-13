#ifndef _CC3D_ANIMATE_TIMELINE_H_
#define _CC3D_ANIMATE_TIMELINE_H_


#include "Common/CC3DUtils.h"
#include <vector>
#include <unordered_map>

class CC3DModel;

namespace CC3DImageFilter
{
	class CC3DSkeleton;
	class CC3DAnimate;

	struct TRSInfo
	{
		Vector4 Rotation = Vector4(0, 0, 0, 1);
		bool EnableRotate = false;
		Vector3 Scale = Vector3(1, 1, 1);
		bool EnableScale = false;
		Vector3 Translate = Vector3(0, 0, 0);
		bool EnableTranslate = false;
	};

	struct AnimBlendInfo
	{
		std::unordered_map<int, TRSInfo> boneNodeMap;
		std::unordered_map<int, TRSInfo> modelNodeMap;
		std::unordered_map<int, std::vector<float>> weightMap;
		int animIndex = -1;
		std::string maskBoneName;
	};

	class CC3DAnimateTimeline : public CCglTFModel
	{
	public:
		CC3DAnimateTimeline();
		~CC3DAnimateTimeline();
		float AddSkeletonAnimateData(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateData, const std::string& animName);
		void AddSkeletonAnimationDataForBlend(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateData, const std::string& animName, const std::string& boneName, float blendWeight = 1.0f);
		void ReSortAnimateTimeline(float blendDeltaTime);
		void Play(float fSecond, CC3DModel* pModel, CC3DSkeleton* pSkeleton);
		bool HasModelAnimate = false;
	private:
		std::vector<CC3DAnimate*> m_Animate;
		float m_TimelineAllTime = 0.0f;
		long frameCount = 0;
		float blendTime = 0.05f;
		float pauseTime = 5.0f;
		std::shared_ptr<class CC3DAnimationBlend> animationBlend;
	};
}


#endif