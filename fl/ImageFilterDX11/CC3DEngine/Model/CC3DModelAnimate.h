#pragma once
#ifndef _H_CC3D_MODEL_ANIMATE_H_
#define _H_CC3D_MODEL_ANIMATE_H_
#include "Common/CC3DUtils.h"
#include "Model/CC3DNode.h"
#include "Scene/CC3DProject.h"
#include <vector>
class CC3DModelAnimate : public CCglTFModel
{
public:
	CC3DModelAnimate();
	virtual ~CC3DModelAnimate();
	void LoadModelInfo(CC3DNode* pNode);
	void UpdateNode();

	CC3DNode* m_ModelNode;

	std::vector<CC3DNodeInfo*> m_CameraNode;
	std::vector<CC3DPerspectiveProject> m_Project;
private:



};


#endif // _H_CC3D_MODEL_H_
