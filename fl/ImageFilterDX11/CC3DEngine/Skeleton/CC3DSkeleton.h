#pragma once
#ifndef _H_CC3D_SKELETON_H_
#define _H_CC3D_SKELETON_H_
#include "Common/CC3DUtils.h"
#include "Model/CC3DModel.h"
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

class CC3DDyanmicBoneManager;

struct dynamicBoneParameter
{
	std::string bone_name;
	float _fDamping = 0.2f;			//阻尼
	float _fElasticity = 0.05f;		//弹性
	float _fStiffness = 0.5f;		//刚性
	float _fInert = 0.5f;			//惯性
	float _fRadius = 0.0f;			//半径
	float _fEndLength = 0.0f;		//
	Vector3 _endOffset = Vector3(0.0f);
	//重力
	Vector3 _gravity = Vector3(0.0f);
	Vector3 _force = Vector3(0.0f);
};

struct SkinInfo
{
	CC3DBoneNodeInfo *pNode;
	glm::mat4 InverseBindMat;
	glm::mat4 FinalMat;
};
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
	void UpdateDynamicBoneParameter(dynamicBoneParameter& param);
	
	std::vector<CC3DBoneNodeInfo*>m_BoneNode;

	std::vector<std::vector<SkinInfo>>m_BoneNodeArray;

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

	shared_ptr<CC3DDyanmicBoneManager> dynamicBonaMgr;
	int m_nIndex;
};


#endif //
