
#include "CC3DSkeletonManage.h"

CC3DSkeletonManage::CC3DSkeletonManage()
{
}

CC3DSkeletonManage::~CC3DSkeletonManage()
{

}

void CC3DSkeletonManage::UpdateSkeleton()
{

	int nSkeleton = Skeletons.size();
	if (nSkeleton>0)
	{
		auto &BoneNode = Skeletons[0].pSkeleton->m_BoneNode;
		auto &BoneMap = Skeletons[0].pSkeleton->m_BoneMap;
		Skeletons[0].pSkeleton->UpdateBone();
		for (int i=1;i<nSkeleton;i++)
		{
			auto &BoneNodeAttach = Skeletons[i].pSkeleton->m_BoneNode;
			bool hasCommon = false;
			for (int b = 0;b<BoneNodeAttach.size();b++)
			{
				auto BoneName = BoneNodeAttach[b]->bone_name;
				auto itr = BoneMap.find(BoneName);
				if (itr != BoneMap.end())
				{
					int nIndex = itr->second;
					CC3DBoneNodeInfo *pInfo = BoneNode[nIndex];
					CC3DBoneNodeInfo *pInfoAttach = BoneNodeAttach[b];
					pInfoAttach->TargetRotation = pInfo->TargetRotation;
					pInfoAttach->TargetScale = pInfo->TargetScale;
					pInfoAttach->TargetTranslate = pInfo->TargetTranslate;
					if (pInfo->pParent!=NULL)
					{
						pInfoAttach->ParentMat = pInfo->pParent->FinalTransformation;
						hasCommon = true;
					}
				}
			}

			if (!hasCommon)
			{
				auto AttachName = Skeletons[i].AttachBone;
				auto itr = BoneMap.find(AttachName);
				if (itr != BoneMap.end())
				{
					int nIndex = itr->second;
					CC3DBoneNodeInfo *pInfo = BoneNode[nIndex];
					auto &RootNodeAttach = Skeletons[i].pSkeleton->m_RootNode;
					for (int b = 0; b < RootNodeAttach.size(); b++)
					{
						CC3DBoneNodeInfo *pInfoAttach = RootNodeAttach[b];

						pInfoAttach->ParentMat = pInfo->FinalTransformation *glm::inverse(pInfo->TPosMat);
					}
				}
			}

			Skeletons[i].pSkeleton->UpdateBone();
		
		}

	}
}


void CC3DSkeletonManage::AddDynamicBone(CC3DImageFilter::dynamicBoneParameter& DyBone)
{
	CC3DImageFilter::dynamicBoneParameter db_info;
	int nSkeleton = Skeletons.size();
	if (nSkeleton > 0)
	{
		for (int i = 0; i < nSkeleton; i++)
		{
			auto& boneMap = Skeletons[i].pSkeleton->m_BoneMap;
			auto itr = boneMap.find(DyBone.bone_name);
			if (itr != boneMap.end())
			{
				std::vector<CC3DImageFilter::dynamicBoneParameter> param_array;

				param_array.push_back(DyBone);
				Skeletons[i].pSkeleton->AddDynamicBone(param_array);
			}
		}
	}
}

CC3DImageFilter::dynamicBoneParameter CC3DSkeletonManage::CreateDefaultDynamicBone(const std::string bone_name)
{
	CC3DImageFilter::dynamicBoneParameter db_info;
	db_info.bone_name = bone_name;
	db_info._fDamping = 0.2f;
	db_info._fElasticity = 0.07f;
	db_info._fStiffness = 0.5f;
	db_info._fInert = 0.5f;
	db_info._fRadius = 0.0f;
	db_info._fEndLength = 0.0f;
	db_info._endOffset = 0.0f;
	return db_info;
}

void CC3DSkeletonManage::UpdateDynamicBoneConfig(const CC3DImageFilter::dynamicBoneParameter& config)
{
	int nSkeleton = Skeletons.size();
	if (nSkeleton > 0)
	{
		for (int i = 0; i < nSkeleton; i++)
		{
			Skeletons[i].pSkeleton->UpdateDynamicBoneParameter(config);
		}
	}
}

void CC3DSkeletonManage::ResetDynamicBone()
{
	int nSkeleton = Skeletons.size();
	if (nSkeleton > 0)
	{
		for (int i = 0; i < nSkeleton; i++)
		{
			Skeletons[i].pSkeleton->ResetDynamicBone();
		}
	}
}

void CC3DSkeletonManage::DeleteDynamicBone(const std::string& db_name)
{
	int nSkeleton = Skeletons.size();
	if (nSkeleton > 0)
	{
		for (int i = 0; i < nSkeleton; i++)
		{
			Skeletons[i].pSkeleton->DeleteDynamicBone(db_name);
		}
	}
}
