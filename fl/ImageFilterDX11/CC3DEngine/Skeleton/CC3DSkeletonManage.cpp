
#include "CC3DSkeletonManage.h"

CC3DSkeletonManage::CC3DSkeletonManage()
{
}

CC3DSkeletonManage::~CC3DSkeletonManage()
{

}

void CC3DSkeletonManage::UpdateSkeleton()
{

	int nSkeleton = m_Skeletons.size();
	if (nSkeleton>0)
	{
		auto &BoneNode = m_Skeletons[0].pSkeleton->m_BoneNode;
		auto &BoneMap = m_Skeletons[0].pSkeleton->m_BoneMap;
		m_Skeletons[0].pSkeleton->UpdateBone();
		for (int i=1;i<nSkeleton;i++)
		{
			auto &BoneNodeAttach = m_Skeletons[i].pSkeleton->m_BoneNode;
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
				auto AttachName = m_Skeletons[i].AttachBone;
				auto itr = BoneMap.find(AttachName);
				if (itr != BoneMap.end())
				{
					int nIndex = itr->second;
					CC3DBoneNodeInfo *pInfo = BoneNode[nIndex];
					auto &RootNodeAttach = m_Skeletons[i].pSkeleton->m_RootNode;
					for (int b = 0; b < RootNodeAttach.size(); b++)
					{
						CC3DBoneNodeInfo *pInfoAttach = RootNodeAttach[b];

						pInfoAttach->ParentMat = pInfo->FinalTransformation *glm::inverse(pInfo->TPosMat);
					}
				}
			}

			m_Skeletons[i].pSkeleton->UpdateBone();
		
		}




	}
}
