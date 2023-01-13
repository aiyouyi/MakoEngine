#pragma once
#ifndef _H_CC3D_ANIMATEMANAGE_H_
#define _H_CC3D_ANIMATEMANAGE_H_
#include "Common/CC3DUtils.h"
#include "Model/CC3DModel.h"
#include "CC3DAnimate.h"
#include "Skeleton/CC3DSkeleton.h"
#include <vector>

namespace CC3DImageFilter
{
	class CC3DAnimateTimeline;

	class CC3DAnimateManage : public CCglTFModel
	{
	public:
		CC3DAnimateManage();
		virtual ~CC3DAnimateManage();

		void InitAnimate();

		void ReadSkeletonAnimate(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateFile);
		void ReadSkeletonAnimateData(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateData);
		void AddSkeletonAnimateData(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateData, const std::string& animName);
		void AddSkeletonAnimateDataForBlend(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateData, const std::string& animName, const std::string& boneName, float blendWeight = 1.0f);
		void ReSortAnimateTimeline(float blendTime);
		void play(float fSecond, CC3DModel* pModel, CC3DSkeleton* pSkeleton = NULL);
		void playOnce(float fSecond, CC3DModel* pModel, CC3DSkeleton* pSkeleton = NULL);
		void LoadAnimateJson(const std::string& fileName, CC3DSkeleton* pSkeleton);

		float m_AnimateAllTime = 0;

		long frameCount = 0;
		bool m_hasModelAnimate = false;
		std::vector<CC3DAnimate*>m_Animate;

	private:
		std::shared_ptr<CC3DAnimateTimeline> m_AnimateTimeline;
		std::shared_ptr<CC3DAnimate> m_JsonAnimate;

	};
}



#endif // _H_CC3D_MODEL_H_
