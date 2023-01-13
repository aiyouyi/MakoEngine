#pragma once
#ifndef _H_CC3D_SKELETON_H_
#define _H_CC3D_SKELETON_H_
#include "Common/CC3DUtils.h"
#include "Model/CC3DModel.h"
#include "DynamicBoneInfo.h"

class CC3DDyanmicBoneManager;

namespace CC3DImageFilter
{

	class CC3DSkeleton : public CCglTFModel
	{
	public:
		CC3DSkeleton();
		virtual ~CC3DSkeleton();

		void InitSkeleton();
		void UpdateBone();
		void UseInitPos();
		void UpdateNeckBone(std::vector<float>& headRotation, const char* neckBoneName);
		void AddDynamicBone(const std::vector<dynamicBoneParameter>& db_paramter_array);
		void ResetDynamicBone();
		void DeleteDynamicBone(const std::string& db_name);
		void UpdateDynamicBoneParameter(const dynamicBoneParameter& param);
		bool HasDynamicBone() const;

		std::vector<CC3DBoneNodeInfo*>m_BoneNode;

		std::vector<std::vector<BoneSkinInfo>>m_BoneNodeArray;

		//骨骼名对应的骨骼ID
		std::map<std::string, int>m_BoneMap;
		//NodeID对应的骨骼ID
		std::map<int, int>m_NodeBoneMap;
		std::vector<CC3DBoneNodeInfo*> m_RootNode;

		std::string m_SkeletonName;

		CC3DNode* m_ModelNode;

		bool _initialized = false;
		bool _enableDynamicBone = true;

	private:

		void CreateModelBoneTree(int NodeID);
		/*
		 @初始化动态骨骼节点信息
		*/
		void InitDynamicBoneNode();
		/*
		 @递归更新骨骼最终状态，，在此处进行动态骨骼更新，同步动态骨骼信息，计算出最后的变化矩阵
		 @param: CC3DBoneNodeInfo  骨骼信息
		*/
		void DFSBoneTree(CC3DBoneNodeInfo* info, glm::mat4& ParentMatrix);

		std::shared_ptr<CC3DDyanmicBoneManager> dynamicBonaMgr;
		int m_nIndex;
	};
}




#endif //
