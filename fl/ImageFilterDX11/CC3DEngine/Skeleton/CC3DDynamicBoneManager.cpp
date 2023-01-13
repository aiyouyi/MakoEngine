#include "CC3DDynamicBoneManager.h"
#include "BaseDefine/Define.h"
#include "Common/CC3DUtils.h"
#include "Skeleton/CC3DSkeleton.h"


CC3DDyanmicBoneManager::CC3DDyanmicBoneManager()
{

}

CC3DDyanmicBoneManager::~CC3DDyanmicBoneManager()
{
	auto itr = transformMap.begin();
	for (; itr != transformMap.end(); )
	{
		if (itr->second)
			SAFE_DELETE(itr->second);
		transformMap.erase(itr++);
	}

	dynamicBoneNamesArray.clear();
}

void CC3DDyanmicBoneManager::InitParticle(CC3DTransformNode* transNode, int db_index )
{
	if (transNode == nullptr || db_index == -1 || db_index >= dynamicBoneArray.size() )
	{
		return;
	}

	dynamicBoneArray[db_index]->InitParticle(transNode);
}


void CC3DDyanmicBoneManager::InitTransfrom(int db_index)
{
	if (db_index == -1 || db_index >= dynamicBoneArray.size())
	{
		return;
	}

	dynamicBoneArray[db_index]->InitTransform();
}

void CC3DDyanmicBoneManager::DFSInitDynamicBoneNode(CC3DBoneNodeInfo* info, glm::mat4& ParentMatrix)
{
	glm::mat4 NodeTransformation;

	glm::mat4 mat4Scaling = glm::scale(NodeTransformation, glm::vec3(info->TargetScale.x, info->Scale.y, info->Scale.z));
	glm::mat4 mat4Rotation = CC3DUtils::QuaternionToMatrix(info->TargetRotation);
	glm::mat4 mat4Translation = glm::translate(NodeTransformation, glm::vec3(info->TargetTranslate.x, info->TargetTranslate.y, info->TargetTranslate.z));

	NodeTransformation = ParentMatrix * mat4Translation *mat4Rotation* mat4Scaling;
	info->FinalTransformation = NodeTransformation;

	//根据配置骨骼名添加动态骨骼部件
	for (int i = 0; i < dynamicBoneNamesArray.size(); i++)
	{
		CC3DMath::CC3DMatrix rot_mat(info->FinalTransformation[0][0], info->FinalTransformation[1][0], info->FinalTransformation[2][0], info->FinalTransformation[3][0],
			info->FinalTransformation[0][1], info->FinalTransformation[1][1], info->FinalTransformation[2][1], info->FinalTransformation[3][1],
			info->FinalTransformation[0][2], info->FinalTransformation[1][2], info->FinalTransformation[2][2], info->FinalTransformation[3][2],
			info->FinalTransformation[0][3], info->FinalTransformation[1][3], info->FinalTransformation[2][3], info->FinalTransformation[3][3]);

		CC3DMath::CC3DQuaternion qtn;
		CC3DMath::CC3DQuaternion::createFromRotationMatrix(rot_mat, &qtn);

		auto cf_boneNameParam = dynamicBoneNamesArray[i];
		if (cf_boneNameParam.bone_name == info->bone_name)
		{
			auto worldRotationMat = info->FinalTransformation;
			CC3DTransformNode* transformNode = new CC3DTransformNode(info->bone_name.c_str());
			transformNode->localPosition = info->TargetTranslate;
			transformNode->localRotation = CC3DMath::CC3DQuaternion(info->TargetRotation.x, info->TargetRotation.y, info->TargetRotation.z, info->TargetRotation.w);
			transformNode->worldPosition = float3(worldRotationMat[3][0], worldRotationMat[3][1], worldRotationMat[3][2]);
			transformNode->worldRotation = qtn;

			transformNode->localToWorld = rot_mat;
			rot_mat.invert(&transformNode->worldToLocal);
			transformNode->glm_localToWorld = worldRotationMat;
			transformNode->glm_worldToLocal = glm::inverse(worldRotationMat);

			auto dmc_bone = std::make_shared<DynamicBone>();
			dynamicBoneArray.push_back(dmc_bone);
			info->db_index = dynamicBoneArray.size() - 1;

			DynamicBoneInfo db_info;
			db_info._fDamping = cf_boneNameParam._fDamping;
			db_info._fElasticity = cf_boneNameParam._fElasticity;
			db_info._fStiffness = cf_boneNameParam._fStiffness;
			db_info._fInert = cf_boneNameParam._fInert;
			db_info._fRadius = cf_boneNameParam._fRadius;
			db_info._fEndLength = cf_boneNameParam._fEndLength;
			db_info._endOffset = cf_boneNameParam._endOffset;
			db_info._gravity = cf_boneNameParam._gravity;
			dmc_bone->Init(db_info);
			info->bAttachToDynamic = true;

			auto itr = transformMap.find(info->bone_name);
			if (itr == transformMap.end())
			{
				transformMap[info->bone_name] = transformNode;
			}

		}
		else if (info->pParent != nullptr && info->pParent->bAttachToDynamic == true)
		{
			//寻找父节点所对应的transform接口
			auto itr = transformMap.find(info->pParent->bone_name);
			if (itr != transformMap.end())
			{
				auto worldRotationMat = info->FinalTransformation;
				CC3DTransformNode* transformNode = new CC3DTransformNode(info->bone_name.c_str());
				transformNode->localPosition = info->TargetTranslate;
				transformNode->localRotation = CC3DMath::CC3DQuaternion(info->TargetRotation.x, info->TargetRotation.y, info->TargetRotation.z, info->TargetRotation.w);
				transformNode->worldPosition = float3(worldRotationMat[3][0], worldRotationMat[3][1], worldRotationMat[3][2]);
				transformNode->worldRotation = qtn;

				transformNode->localToWorld = rot_mat;
				rot_mat.invert(&transformNode->worldToLocal);
				transformNode->glm_localToWorld = worldRotationMat;
				transformNode->glm_worldToLocal = glm::inverse(worldRotationMat);

				itr->second->addChild(transformNode);
				transformMap[info->bone_name] = transformNode;
				info->bAttachToDynamic = true;
				break;
			}
		}
	}

	for (int i = 0; i < info->pChildren.size(); i++)
	{
		DFSInitDynamicBoneNode(info->pChildren[i], NodeTransformation);
	}
}

void CC3DDyanmicBoneManager::DynamicBonePreUpdate(CC3DBoneNodeInfo* info, glm::mat4& ParentMatrix)
{
	glm::mat4 NodeTransformation;

	glm::mat4 mat4Scaling = glm::scale(NodeTransformation, glm::vec3(info->TargetScale.x, info->TargetScale.y, info->TargetScale.z));

	glm::mat4 mat4Rotation = CC3DUtils::QuaternionToMatrix(info->TargetRotation);

	glm::mat4 mat4Translation = glm::translate(NodeTransformation, glm::vec3(info->TargetTranslate.x, info->TargetTranslate.y, info->TargetTranslate.z));

	NodeTransformation = ParentMatrix * mat4Translation *mat4Rotation* mat4Scaling;
	info->FinalTransformation = NodeTransformation;

	auto itr = transformMap.find(info->bone_name);
	if (itr != transformMap.end())
	{
		CC3DMath::CC3DMatrix rot_mat(info->FinalTransformation[0][0], info->FinalTransformation[1][0], info->FinalTransformation[2][0], info->FinalTransformation[3][0],
			info->FinalTransformation[0][1], info->FinalTransformation[1][1], info->FinalTransformation[2][1], info->FinalTransformation[3][1],
			info->FinalTransformation[0][2], info->FinalTransformation[1][2], info->FinalTransformation[2][2], info->FinalTransformation[3][2],
			info->FinalTransformation[0][3], info->FinalTransformation[1][3], info->FinalTransformation[2][3], info->FinalTransformation[3][3]);

		CC3DMath::CC3DQuaternion qtn;
		CC3DMath::CC3DQuaternion::createFromRotationMatrix(rot_mat, &qtn);

		itr->second->localPosition = info->TargetTranslate;
		itr->second->localRotation = quaternion4f(info->TargetRotation.x, info->TargetRotation.y, info->TargetRotation.z, info->TargetRotation.w);
		itr->second->localToWorld = rot_mat;
		rot_mat.invert(&itr->second->worldToLocal);

		itr->second->worldPosition = float3(info->FinalTransformation[3][0], info->FinalTransformation[3][1], info->FinalTransformation[3][2]);
		itr->second->worldRotation = qtn;

		itr->second->glm_localToWorld = info->FinalTransformation;
		itr->second->glm_worldToLocal = glm::inverse(info->FinalTransformation);
	}

	for (int i = 0; i < info->pChildren.size(); i++)
	{
		DynamicBonePreUpdate(info->pChildren[i], NodeTransformation);
	}
}

void CC3DDyanmicBoneManager::LateUpdate(int db_index /*= -1*/)
{
	if (db_index == -1 || db_index >= dynamicBoneArray.size())
	{
		return;
	}

	dynamicBoneArray[db_index]->Update(0.05f);
}

void CC3DDyanmicBoneManager::ResetDynamicBone()
{
	auto itr = transformMap.begin();
	for (; itr != transformMap.end(); )
	{
		if (itr->second)
			SAFE_DELETE(itr->second);
		transformMap.erase(itr++);
	}

	for (auto itr : dynamicBoneArray)
	{
		itr.reset();
	}

	dynamicBoneArray.clear();
}

void CC3DDyanmicBoneManager::DeleteDynamicBone(const std::string& db_name)
{
	auto itr = transformMap.find(db_name);
	if (itr != transformMap.end())
	{
		auto child_node = itr->second;
		std::string childname;
		if ( child_node->getFirstChild())
		{
			childname = child_node->getFirstChild()->getId();
		}
	 
		SAFE_DELETE(itr->second);
		itr = transformMap.erase(itr);

		if (!childname.empty())
		{
			recuDeleteTransfromNode(childname);
		}
	}

	for (auto itr = dynamicBoneArray.begin(); itr != dynamicBoneArray.end(); )
	{
		auto bone = *itr;
		if (bone->GetID() == db_name)
		{
			bone.reset();
			itr = dynamicBoneArray.erase(itr);

		}
		else
		{
			itr++;
		}
	}

	for (auto itr = dynamicBoneNamesArray.begin(); itr != dynamicBoneNamesArray.end(); )
	{
		if ( (*itr).bone_name == db_name )
		{
			itr = dynamicBoneNamesArray.erase(itr);
		}
		else
		{
			itr++;
		}
	}
}

void CC3DDyanmicBoneManager::recuDeleteTransfromNode(const std::string& db_name)
{
	auto itr = transformMap.find(db_name);
	if (itr != transformMap.end())
	{
		auto child_node = itr->second;
		std::string childname;
		if (child_node->getFirstChild())
		{
			childname = child_node->getFirstChild()->getId();
		}

		SAFE_DELETE(itr->second);
		itr = transformMap.erase(itr);

		if (!childname.empty())
		{
			recuDeleteTransfromNode(childname);
		}
	}
}

void CC3DDyanmicBoneManager::UpdateDynamicBoneParameter(const CC3DImageFilter::dynamicBoneParameter& cf_boneNameParam)
{
	for (auto item : dynamicBoneArray)
	{
		if (item->GetID() == cf_boneNameParam.bone_name)
		{
			DynamicBoneInfo db_info;
			db_info._fDamping = cf_boneNameParam._fDamping;
			db_info._fElasticity = cf_boneNameParam._fElasticity;
			db_info._fStiffness = cf_boneNameParam._fStiffness;
			db_info._fInert = cf_boneNameParam._fInert;
			db_info._fRadius = cf_boneNameParam._fRadius;
			db_info._fEndLength = cf_boneNameParam._fEndLength;
			db_info._endOffset = cf_boneNameParam._endOffset;
			db_info._gravity = cf_boneNameParam._gravity;
			item->UpdateParticleParam(db_info);
		}
	}
}

bool CC3DDyanmicBoneManager::AddDynamicBoneCollider(std::string bone_name, DynamicBoneColliderBase::DynamicBoneColliderInfo dbc_info, CC3DTransformNode* trans_node)
{
	if (bone_name.empty())
	{
		return false;
	}

	for (auto db : dynamicBoneArray)
	{
		if (db->GetID() == bone_name)
		{
			DynamicBoneColliderBase* collider = new DynamicBoneCollider();
			collider->Init(dbc_info, trans_node);
			db->AddCollider(collider);

			colliderArray.push_back(collider);
			return true;
		}
	}

	return false;
}
