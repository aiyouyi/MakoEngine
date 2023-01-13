#pragma once
#ifndef _H_CC3D_ANIMATE_H_
#define _H_CC3D_ANIMATE_H_
#include "Common/CC3DUtils.h"
#include "Model/CC3DModel.h"
#include "Skeleton/CC3DSkeleton.h"
#include <vector>


namespace CC3DImageFilter
{
	struct AnimBlendInfo;
	class CC3DBodyDriveConfig;

	class CC3DAnimate : public CCglTFModel
	{
	public:
		CC3DAnimate();
		virtual ~CC3DAnimate();

		void InitAnimate(uint32 AnimateIndex);

		void ReadSkeletonAnimate(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateFile);
		void ReadSkeletonAnimateData(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateData);

		void play(float fSecond, CC3DModel* pModel, CC3DSkeleton* pSkeleton = NULL);

		void play(float fSecond, CC3DModel* pModel, CC3DSkeleton* pSkeleton, AnimBlendInfo& blendInfo);

		void playCam(float fSecond, CC3DModel* pModel, int nCamIndex);

		void playModel(float fSecond, CC3DModel* pModel, bool isLoop = false);
		std::string m_AnimateName;

		float m_AnimateAllTime = 0;
		bool hasModelAnimate = false;
		float startTime = 0.0f;
		float endTime = 0.0f;

		void LoadAnimateJson(const std::string& fileName, CC3DSkeleton* pSkeleton);
		void playByConfig(CC3DModel* pModel, CC3DSkeleton* pSkeleton);

		static float AnimationSLerp;
	private:

		void WriteBin(CC3DSkeleton* pSkeleton = NULL, CC3DModel* pModel = NULL);
		void playBlendShape(float fSecond, CC3DModel* pModel, CC3DAnimationChannelInfo* pInfo);
		void playSkeleton(float fSecond, CC3DSkeleton* pSkeleton, CC3DModel* pModel, CC3DAnimationChannelInfo* pInfo);
		void playBlendShapeWithBlend(float fSecond, CC3DModel* pModel, CC3DAnimationChannelInfo* pInfo, AnimBlendInfo* blendInfo = nullptr);
		void playSkeletonWithBlend(float fSecond, CC3DSkeleton* pSkeleton, CC3DModel* pModel, CC3DAnimationChannelInfo* pInfo, AnimBlendInfo* blendInfo = nullptr);
		std::vector<CC3DAnimationChannelInfo*> m_ChannelInfo;

		std::string maskBoneName;


		std::shared_ptr< CC3DBodyDriveConfig> m_BodyDriveConfig;
		std::chrono::high_resolution_clock::time_point m_TStart, m_TEnd;
		int m_playIndex = 0;
	};
};

#endif // _H_CC3D_MODEL_H_
