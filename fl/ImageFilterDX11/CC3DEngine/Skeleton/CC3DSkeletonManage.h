#pragma once
#ifndef _H_CC3D_SKELETONMANAGE_H_
#define _H_CC3D_SKELETONMANAGE_H_
#include "Common/CC3DUtils.h"
#include "Skeleton/CC3DSkeleton.h"
#include <vector>
#include <string>

struct SkeletonInfo
{
	CC3DImageFilter::CC3DSkeleton *pSkeleton;
	std::string AttachBone;
};

class CC3DSkeletonManage
{
public:
	CC3DSkeletonManage();
	virtual ~CC3DSkeletonManage();

	void UpdateSkeleton();
	void AddDynamicBone(CC3DImageFilter::dynamicBoneParameter& DyBone);
	CC3DImageFilter::dynamicBoneParameter CreateDefaultDynamicBone(const std::string bone_name);
	void UpdateDynamicBoneConfig(const CC3DImageFilter::dynamicBoneParameter& config);
	void ResetDynamicBone();
	void DeleteDynamicBone(const std::string& db_name);

	std::vector<SkeletonInfo> Skeletons;
};


#endif // _H_CC3D_MODEL_H_
