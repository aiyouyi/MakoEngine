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

void CC3DDyanmicBoneManager::InitParticle(CC3DTransformNode* transNode, int db_index)
{
	if (transNode == nullptr || db_index == -1 || db_index >= dynamicBoneArray.size())
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

	//�������ù�������Ӷ�̬��������
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
			//Ѱ�Ҹ��ڵ�����Ӧ��transform�ӿ�
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
