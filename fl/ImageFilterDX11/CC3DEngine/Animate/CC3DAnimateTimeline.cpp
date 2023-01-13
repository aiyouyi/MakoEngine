#include "CC3DAnimateTimeline.h"

#include "CC3DEngine/Model/CC3DModel.h"
#include "CC3DEngine/Skeleton/CC3DSkeleton.h"
#include "CC3DEngine/Animate/CC3DAnimate.h"
#include "Toolbox/inc.h"
#include "CC3DAnimationBlend.h"

namespace CC3DImageFilter
{
	CC3DAnimateTimeline::CC3DAnimateTimeline()
	{
		animationBlend = std::make_shared<CC3DAnimationBlend>();
	}

	CC3DAnimateTimeline::~CC3DAnimateTimeline()
	{

	}

	float CC3DAnimateTimeline::AddSkeletonAnimateData(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateData, const std::string& animName)
	{
		CC3DAnimate* pAnimate = new CC3DAnimate();
		pAnimate->initModel(m_Model);
		pAnimate->ReadSkeletonAnimateData(pModel, pSkeleton, pAnimateData);
		pAnimate->m_AnimateName = animName;
		m_Animate.push_back(pAnimate);
		m_TimelineAllTime += pAnimate->m_AnimateAllTime;

		return pAnimate->m_AnimateAllTime;
	}

	void CC3DAnimateTimeline::AddSkeletonAnimationDataForBlend(CC3DModel* pModel, CC3DSkeleton* pSkeleton, const char* pAnimateData, const std::string& animName, const std::string& boneName, float blendWeight)
	{
		std::shared_ptr<CC3DAnimate> pAnimate = std::make_shared<CC3DAnimate>();
		pAnimate->initModel(m_Model);
		pAnimate->ReadSkeletonAnimateData(pModel, pSkeleton, pAnimateData);
		pAnimate->endTime = pAnimate->m_AnimateAllTime;
		if (animationBlend)
		{
			animationBlend->AddBlendAnimation(pAnimate, animName, boneName, blendWeight);
		}
	}

	void CC3DAnimateTimeline::ReSortAnimateTimeline(float blendDeltaTime)
	{
		if (m_Animate.empty())
			return;

		blendTime = blendDeltaTime;
		m_TimelineAllTime = m_TimelineAllTime - (m_Animate.size() - 1) * blendDeltaTime;

		std::random_device rd;
		std::mt19937 g(rd());

		std::shuffle(m_Animate.begin(), m_Animate.end(), g); //对数组内元素进行随机排序

		//设置timeline上每个动画的start end	
		float delta = 0.0f;
		m_Animate[0]->startTime = 0.0f;
		m_Animate[0]->endTime = m_Animate[0]->m_AnimateAllTime;
		delta = m_Animate[0]->endTime;

		for (int i = 1; i < m_Animate.size(); ++i)
		{
			CC3DAnimate* item = m_Animate[i];
			item->startTime = delta - blendDeltaTime;
			item->endTime = delta + item->m_AnimateAllTime - blendDeltaTime;
			delta = item->endTime;
		}

		//随机生成播放时长
		std::default_random_engine e(rd());
		std::uniform_real_distribution<double> u(0.0, 0.5);
		pauseTime = u(e) * 10.0f;
	}

	void CC3DAnimateTimeline::Play(float fSecond, CC3DModel* pModel, CC3DSkeleton* pSkeleton)
	{
		if (m_TimelineAllTime < 0.001)
		{
			return;
		}

		//if (fSecond > pauseTime)
		//{
		//	return;
		//}

		int nTemp = fSecond / m_TimelineAllTime;
		double during = fSecond - nTemp * m_TimelineAllTime;

		frameCount++;

		AnimBlendInfo blendInfoSrc, blendInfoDst;
		bool bFindSrc = false;
		for (int i = 0; i < m_Animate.size(); ++i)
		{
			if (m_Animate[i]->startTime < during && during < m_Animate[i]->endTime)
			{
				if (!bFindSrc)
				{
					double AnimDuring = during - m_Animate[i]->startTime;
					m_Animate[i]->play(AnimDuring, pModel, pSkeleton, blendInfoSrc); //during 有问题 TODO
					if (animationBlend)
					{
						animationBlend->ApplyBlendNode(AnimDuring, pModel, pSkeleton, m_Animate[i]->m_AnimateName, &blendInfoSrc);
					}
					if (m_Animate[i]->hasModelAnimate)
					{
						HasModelAnimate = true;
					}
					blendInfoSrc.animIndex = i;
					bFindSrc = true;
					continue;
				}
				else
				{
					float AnimDuring = during - m_Animate[i]->startTime;
					m_Animate[i]->play(AnimDuring, pModel, pSkeleton, blendInfoDst); //during 有问题 TODO
					if (animationBlend)
					{
						animationBlend->ApplyBlendNode(AnimDuring, pModel, pSkeleton, m_Animate[i]->m_AnimateName, &blendInfoDst);
					}
					if (m_Animate[i]->hasModelAnimate)
					{
						HasModelAnimate = true;
					}
					blendInfoDst.animIndex = i;
					break;
				}
			}
		}

		//更新骨骼动画
		if (!blendInfoSrc.boneNodeMap.empty() && !blendInfoDst.boneNodeMap.empty())
		{
			float alpha = (during - m_Animate[blendInfoDst.animIndex]->startTime) / blendTime;
			for (auto iter = blendInfoSrc.boneNodeMap.begin(); iter != blendInfoSrc.boneNodeMap.end(); iter++)
			{
				auto dstItr = blendInfoDst.boneNodeMap.find(iter->first);
				if (dstItr != blendInfoDst.boneNodeMap.end())
				{
					TRSInfo src = iter->second;
					TRSInfo dst = dstItr->second;
					CC3DBoneNodeInfo* pBoneInfo = pSkeleton->m_BoneNode[pSkeleton->m_NodeBoneMap[iter->first]];
					if (src.EnableRotate)
					{
						CC3DUtils::QuaternionInterpolate(pBoneInfo->TargetRotation, src.Rotation, dst.Rotation, alpha);
					}
					if (src.EnableScale)
					{
						pBoneInfo->TargetScale = src.Scale * (1.0 - alpha) + dst.Scale * alpha;
					}
					if (src.EnableTranslate)
					{
						pBoneInfo->TargetTranslate = src.Translate * (1.0 - alpha) + dst.Translate * alpha;
					}
					//CC3DUtils::QuaternionInterpolate(pBoneInfo->TargetRotation, src.TargetRotation, dst.TargetRotation, alpha);
					//pBoneInfo->TargetScale = src.TargetScale * (1.0 - alpha) + dst.TargetScale * alpha;
					//pBoneInfo->TargetTranslate = src.TargetTranslate * (1.0 - alpha) + dst.TargetTranslate * alpha;
				}
			}
		}
		else if (!blendInfoSrc.boneNodeMap.empty() && blendInfoDst.boneNodeMap.empty())
		{
			for (auto iter = blendInfoSrc.boneNodeMap.begin(); iter != blendInfoSrc.boneNodeMap.end(); iter++)
			{
				CC3DBoneNodeInfo* pBoneInfo = pSkeleton->m_BoneNode[pSkeleton->m_NodeBoneMap[iter->first]];
				if (iter->second.EnableRotate)
				{
					pBoneInfo->TargetRotation = iter->second.Rotation;
				}
				if (iter->second.EnableScale)
				{
					pBoneInfo->TargetScale = iter->second.Scale;
				}
				if (iter->second.EnableTranslate)
				{
					pBoneInfo->TargetTranslate = iter->second.Translate;
				}


			}
		}

		//更新模型动画
		//if (!blendInfoSrc.modelNodeMap.empty() && !blendInfoDst.modelNodeMap.empty())
		//{
		//	float alpha = (during - m_Animate[blendInfoDst.animIndex]->startTime) / blendTime;
		//	for (auto iter = blendInfoSrc.modelNodeMap.begin(); iter != blendInfoSrc.modelNodeMap.end(); iter++)
		//	{
		//		auto dstItr = blendInfoDst.modelNodeMap.find(iter->first);
		//		if (dstItr != blendInfoDst.modelNodeMap.end())
		//		{
		//			CC3DNodeInfo src = iter->second;
		//			CC3DNodeInfo dst = dstItr->second;
		//			CC3DNodeInfo *pNodeInfo = pModel->m_ModelNode->m_Node[iter->first];
		//			CC3DUtils::QuaternionInterpolate(pNodeInfo->Rotation, src.Rotation, dst.Rotation, alpha);
		//			pNodeInfo->Scale = src.Scale * (1.0 - alpha) + dst.Scale * alpha;
		//			pNodeInfo->Translate = src.Translate * (1.0 - alpha) + dst.Translate * alpha;
		//		}
		//	}
		//}
		//else if (!blendInfoSrc.modelNodeMap.empty() && blendInfoDst.modelNodeMap.empty())
		//{
		//	for (auto iter = blendInfoSrc.modelNodeMap.begin(); iter != blendInfoSrc.modelNodeMap.end(); iter++)
		//	{
		//		CC3DNodeInfo *pNodeInfo = pModel->m_ModelNode->m_Node[iter->first];
		//		pNodeInfo->Rotation = iter->second.Rotation;
		//		pNodeInfo->Scale = iter->second.Scale;
		//		pNodeInfo->Translate = iter->second.Translate;
		//	}
		//}

		//更新blend shape
		if (!blendInfoSrc.weightMap.empty() && !blendInfoDst.weightMap.empty())
		{
			float alpha = (during - m_Animate[blendInfoDst.animIndex]->startTime) / blendTime;
			for (int i = 0; i < pModel->m_ModelMesh.size(); ++i)
			{
				auto itr = blendInfoSrc.weightMap.find(pModel->m_ModelMesh[i]->m_nNodeID);
				if (itr != blendInfoSrc.weightMap.end())
				{
					std::vector<float> vWeight(itr->second.size());
					std::vector<float>& src = itr->second;
					std::vector<float>& dst = blendInfoDst.weightMap[itr->first];
					for (int j = 0; j < vWeight.size(); ++j)
					{
						vWeight[j] = src[j] * (1.0 - alpha) + dst[j] * alpha;
					}

					pModel->m_ModelMesh[i]->GenVertWithWeights(vWeight);
				}
			}
		}
		else if (!blendInfoSrc.weightMap.empty() && blendInfoDst.weightMap.empty())
		{
			for (int i = 0; i < pModel->m_ModelMesh.size(); i++)
			{
				auto itr = blendInfoSrc.weightMap.find(pModel->m_ModelMesh[i]->m_nNodeID);
				if (itr != blendInfoSrc.weightMap.end())
				{
					pModel->m_ModelMesh[i]->GenVertWithWeights(itr->second);
				}
			}
		}
	}
}
