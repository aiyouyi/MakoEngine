#pragma once
#include "inc.h"
#include "BaseDefine/Vectors.h"
#include "glm/matrix.hpp"

class CC3DBoneNodeInfo;

namespace CC3DImageFilter
{
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

	struct BoneSkinInfo
	{
		CC3DBoneNodeInfo* pNode;
		glm::mat4 InverseBindMat;
		glm::mat4 FinalMat;
	};
}