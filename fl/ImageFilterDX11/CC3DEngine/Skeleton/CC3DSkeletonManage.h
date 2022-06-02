#pragma once
#ifndef _H_CC3D_SKELETONMANAGE_H_
#define _H_CC3D_SKELETONMANAGE_H_
#include "Common/CC3DUtils.h"
#include "Skeleton/CC3DSkeleton.h"
#include <vector>
#include <string>

struct SkeletonInfo
{
	CC3DSkeleton *pSkeleton;
	std::string AttachBone;
};

class CC3DSkeletonManage
{
public:
	CC3DSkeletonManage();
	virtual ~CC3DSkeletonManage();

	void UpdateSkeleton();


	vector<SkeletonInfo>m_Skeletons;

private:


};


#endif // _H_CC3D_MODEL_H_
