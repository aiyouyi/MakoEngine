#pragma once
#ifndef _H_CC3D_ANIMATEMANAGE_H_
#define _H_CC3D_ANIMATEMANAGE_H_
#include "Common/CC3DUtils.h"
#include "Model/CC3DModel.h"
#include "CC3DAnimate.h"
#include "Skeleton/CC3DSkeleton.h"
#include <vector>
class CC3DAnimateManage : public CCglTFModel
{
public:
	CC3DAnimateManage();
	virtual ~CC3DAnimateManage();
	
	void InitAnimate();

	void ReadSkeletonAnimate(CC3DModel *pModel, CC3DSkeleton *pSkeleton, const char*pAnimateFile);
	void play(float fSecond,CC3DModel *pModel, CC3DSkeleton *pSkeleton = NULL);
	void playOnce(float fSecond, CC3DModel *pModel, CC3DSkeleton *pSkeleton = NULL);

	float m_AnimateAllTime = 0;

	long frameCount = 0;
	bool m_hasModelAnimate = false;
	vector<CC3DAnimate*>m_Animate;

private:


};


#endif // _H_CC3D_MODEL_H_
