#pragma once
#ifndef _H_CC3D_ANIMATE_H_
#define _H_CC3D_ANIMATE_H_
#include "Common/CC3DUtils.h"
#include "Model/CC3DModel.h"
#include "Skeleton/CC3DSkeleton.h"
#include <vector>
class CC3DAnimate : public CCglTFModel
{
public:
	CC3DAnimate();
	virtual ~CC3DAnimate();
	
	void InitAnimate(uint32 AnimateIndex);

	void ReadSkeletonAnimate(CC3DModel *pModel, CC3DSkeleton *pSkeleton, const char*pAnimateFile);

	void play(float fSecond,CC3DModel *pModel,CC3DSkeleton *pSkeleton = NULL);
	
	void playCam(float fSecond, CC3DModel *pModel, int nCamIndex);

	void playModel(float fSecond, CC3DModel *pModel,bool isLoop = false);
	std::string m_AnimateName;

	float m_AnimateAllTime = 0;
	bool hasModelAnimate = false;
private:

	void WriteBin(CC3DSkeleton *pSkeleton = NULL , CC3DModel *pModel = NULL);
	void playBlendShape(float fSecond, CC3DModel *pModel, CC3DAnimationChannelInfo *pInfo);
	void playSkeleton(float fSecond, CC3DSkeleton *pSkeleton, CC3DModel *pModel, CC3DAnimationChannelInfo *pInfo);
	std::vector<CC3DAnimationChannelInfo*> m_ChannelInfo;


};


#endif // _H_CC3D_MODEL_H_
