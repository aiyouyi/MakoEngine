
#include "CC3DSkeleton.h"
#include "CC3DDynamicBoneManager.h"

namespace CC3DImageFilter
{
	CC3DSkeleton::CC3DSkeleton()
	{
		dynamicBonaMgr = std::make_shared<CC3DDyanmicBoneManager>();
	}

	CC3DSkeleton::~CC3DSkeleton()
	{
		for (int i = 0; i < m_BoneNode.size(); i++)
		{
			SAFE_DELETE(m_BoneNode[i]);
		}
		m_BoneNode.clear();
	}

	void MatToTRS(float* m, CC3DBoneNodeInfo* pInfo)
	{
		glm::mat4* pMat2 = (glm::mat4*)m;
		pInfo->InitMat = pMat2[0];
		pInfo->Translate = Vector3(pInfo->InitMat[3][0], pInfo->InitMat[3][1], pInfo->InitMat[3][2]);
		pInfo->InitMat[3][0] = 0.0; pInfo->InitMat[3][1] = 0.0; pInfo->InitMat[3][2] = 0.0; pInfo->InitMat[3][3] = 1.0;
		Vector3 tmp = Vector3(pInfo->InitMat[0][0], pInfo->InitMat[0][1], pInfo->InitMat[0][2]);
		float s = tmp.length();
		pInfo->Scale = Vector3(s, s, s);
		for (int i = 0; i < 15; i++)
		{
			m[i] /= s;
		}
	}

	void CC3DSkeleton::InitSkeleton()
	{
		auto& pSkins = m_Model->skins;
		auto& nodes = m_Model->nodes;
		m_BoneNodeArray.resize(pSkins.size());
		m_nIndex = 0;
		for (int s = 0; s < pSkins.size(); s++)
		{
			auto& skin = pSkins[s];
			//SkeletonName = skin.name;
			auto& joints = skin.joints;
			auto& MatID = skin.inverseBindMatrices;

			uint32 nCount = 0;
			int type = 0;
			glm::mat4* pMat = (glm::mat4*)Getdata(MatID, nCount, type);
			if (nCount != joints.size())
			{
				continue;;
			}
			BoneSkinInfo pSkinInfo;
			for (int i = 0; i < nCount; i++)
			{
				auto NodeID = joints[i];
				CreateModelBoneTree(NodeID);
				CC3DBoneNodeInfo* pNode = m_BoneNode[m_NodeBoneMap[NodeID]];
				pNode->NodeID = NodeID;
				pNode->InverseBindMat = pMat[i];
				pSkinInfo.pNode = pNode;
				pSkinInfo.InverseBindMat = pMat[i];
				m_BoneNodeArray[s].push_back(pSkinInfo);

			}
		}

		for (int i = 0; i < m_BoneNode.size(); i++)
		{
			if (m_BoneNode[i]->pParent == NULL)
			{
				m_RootNode.push_back(m_BoneNode[i]);
			}
		}
		UseInitPos();
		UpdateBone();

		for (int i = 0; i < m_BoneNode.size(); i++)
		{
			m_BoneNode[i]->TPosMat = m_BoneNode[i]->FinalTransformation;
		}

		//InitDynamicBoneNode();
	}

	void CC3DSkeleton::UpdateBone()
	{
		if (m_RootNode.size() <= 0)
		{
			return;
		}

		if (!_initialized && _enableDynamicBone)
		{
			_initialized = true;
		}

		//将一帧当中每个子节点的变化矩阵都记录下来，为后面update 提供前更新
		if (_enableDynamicBone)
		{
			for (int i = 0; i < m_RootNode.size(); i++)
			{
				int NodeID = m_RootNode[i]->NodeID;
				auto pNode = m_ModelNode->m_Node[NodeID]->pParent;

				glm::mat4 Identity = m_RootNode[i]->ParentMat;
				if (pNode != NULL)
				{
					m_ModelNode->UpdateNodeParent(pNode);
					Identity *= pNode->FinalMeshMat;
				}
				dynamicBonaMgr->DynamicBonePreUpdate(m_RootNode[i], Identity);
			}
		}

		for (int i = 0; i < m_RootNode.size(); i++)
		{
			int NodeID = m_RootNode[i]->NodeID;
			auto pNode = m_ModelNode->m_Node[NodeID]->pParent;

			glm::mat4 Identity = m_RootNode[i]->ParentMat;
			if (pNode != NULL)
			{
				m_ModelNode->UpdateNodeParent(pNode);
				Identity *= pNode->FinalMeshMat;
			}

			DFSBoneTree(m_RootNode[i], Identity);
		}

		for (int i = 0; i < m_BoneNodeArray.size(); i++)
		{
			for (int j = 0; j < m_BoneNodeArray[i].size(); j++)
			{
				m_BoneNodeArray[i][j].FinalMat = m_BoneNodeArray[i][j].pNode->FinalTransformation * m_BoneNodeArray[i][j].InverseBindMat;
			}
		}

		// 	for (int i = 0; i < m_BoneNode.size(); i++)
		// 	{
		// 		m_BoneNode[i]->FinalTransformation *= m_BoneNode[i]->InverseBindMat;
		// 	}	
	}

	void CC3DSkeleton::UpdateNeckBone(std::vector<float>& headRotation, const char* neckBoneName)
	{
		if (m_RootNode.size() <= 0)
		{
			return;
		}
		if (headRotation.size() == 3)
		{
			//Y-Z-X 转四元数顺序
			float Offset = 8 / 180.0 * CC_PI;
			glm::quat Quat = glm::quat(glm::vec3(-headRotation[1], headRotation[2], -headRotation[0]));
			auto itr = m_BoneMap.find(neckBoneName);
			if (itr != m_BoneMap.end())
			{
				int NeckIndex = itr->second;
				CC3DBoneNodeInfo* pInfo = m_BoneNode[NeckIndex];
				Vector4 TargetQuat = Vector4(Quat.x, Quat.y, Quat.z, Quat.w);
				pInfo->TargetRotation = TargetQuat;
			}

		}
		UpdateBone();

	}

	void CC3DSkeleton::AddDynamicBone(const std::vector<dynamicBoneParameter>& db_paramter_array)
	{
		dynamicBonaMgr->ResetDynamicBone();
		for (int i = 0; i < db_paramter_array.size(); ++i)
		{
			dynamicBonaMgr->dynamicBoneNamesArray.push_back(db_paramter_array[i]);
		}

		InitDynamicBoneNode();
	}

	void CC3DSkeleton::ResetDynamicBone()
	{
		dynamicBonaMgr->ResetDynamicBone();
		dynamicBonaMgr->dynamicBoneNamesArray.clear();
	}

	void CC3DSkeleton::DeleteDynamicBone(const std::string& db_name)
	{
		dynamicBonaMgr->DeleteDynamicBone(db_name);
	}

	void CC3DSkeleton::UpdateDynamicBoneParameter(const dynamicBoneParameter& param)
	{
		dynamicBonaMgr->UpdateDynamicBoneParameter(param);
	}

	bool CC3DSkeleton::HasDynamicBone() const
	{
		return !dynamicBonaMgr->dynamicBoneArray.empty();
	}

	void CC3DSkeleton::UseInitPos()
	{
		for (int i = 0; i < m_BoneNode.size(); i++)
		{
			CC3DBoneNodeInfo* pInfo = m_BoneNode[i];
			pInfo->TargetRotation = pInfo->Rotation;
			pInfo->TargetScale = pInfo->Scale;
			pInfo->TargetTranslate = pInfo->Translate;
		}

	}

	void CC3DSkeleton::InitDynamicBoneNode()
	{
		if (m_RootNode.size() <= 0)
		{
			return;
		}

		for (int i = 0; i < m_RootNode.size(); i++)
		{
			glm::mat4 boneTransformation = glm::mat4();
			dynamicBonaMgr->DFSInitDynamicBoneNode(m_RootNode[i], boneTransformation);
		}

		for (int i = 0; i < m_BoneNode.size(); i++)
		{
			auto pInfo = m_BoneNode[i];

			if (pInfo->db_index != -1)
			{
				auto itr = dynamicBonaMgr->transformMap.find(pInfo->bone_name);
				if (itr != dynamicBonaMgr->transformMap.end())
				{
					dynamicBonaMgr->InitParticle(itr->second, pInfo->db_index);
					dynamicBonaMgr->InitTransfrom(pInfo->db_index);
				}
			}
		}
	}

	void CC3DSkeleton::CreateModelBoneTree(int NodeID)
	{

		int nodeID = NodeID;
		if (m_NodeBoneMap.find(NodeID) != m_NodeBoneMap.end())
		{
			return;
		}

		auto& nodes = m_Model->nodes;

		if (nodeID < nodes.size())
		{
			CC3DBoneNodeInfo* pInfo = new CC3DBoneNodeInfo();
			auto& node = nodes[nodeID];
			string boneName = node.name;
			m_BoneMap[boneName] = m_nIndex;
			m_NodeBoneMap[NodeID] = m_nIndex;
			pInfo->bone_name = boneName;
			pInfo->NodeID = NodeID;
			if (node.matrix.size() == 16)
			{
				float mat16[16];
				for (int m = 0; m < 16; m++)
				{
					mat16[m] = node.matrix[m];
				}
				MatToTRS(mat16, pInfo);
			}

			if (node.scale.size() == 3)
			{
				pInfo->Scale.x = node.scale[0];
				pInfo->Scale.y = node.scale[1];
				pInfo->Scale.z = node.scale[2];
			}
			if (node.translation.size() == 3)
			{
				pInfo->Translate.x = node.translation[0];
				pInfo->Translate.y = node.translation[1];
				pInfo->Translate.z = node.translation[2];
			}
			if (node.rotation.size() == 4)
			{
				pInfo->Rotation.x = node.rotation[0];
				pInfo->Rotation.y = node.rotation[1];
				pInfo->Rotation.z = node.rotation[2];
				pInfo->Rotation.w = node.rotation[3];
			}
			m_BoneNode.push_back(pInfo);

			m_nIndex++;

			auto& child = m_Model->nodes[nodeID].children;
			for (int i = 0; i < child.size(); i++)
			{
				int ChildNodeID = child[i];
				CreateModelBoneTree(ChildNodeID);
				CC3DBoneNodeInfo* pChild = m_BoneNode[m_NodeBoneMap[ChildNodeID]];
				pChild->pParent = pInfo;
				pInfo->pChildren.push_back(pChild);
			}
		}

	}

	void CC3DSkeleton::DFSBoneTree(CC3DBoneNodeInfo* info, glm::mat4& ParentMatrix)
	{

		glm::mat4 NodeTransformation;

		glm::mat4 mat4Scaling = glm::scale(NodeTransformation, glm::vec3(info->TargetScale.x, info->TargetScale.y, info->TargetScale.z));

		glm::mat4 mat4Rotation = CC3DUtils::QuaternionToMatrix(info->TargetRotation);

		glm::mat4 mat4Translation = glm::translate(NodeTransformation, glm::vec3(info->TargetTranslate.x, info->TargetTranslate.y, info->TargetTranslate.z));

		//不能去掉！解析模型的时候M16不一定是TRS，所以这里的InitMat存储的转换矩阵，不一定是单位阵。特别是头发处会出问题
		if ((info->TargetRotation - Vector4(0, 0, 0, 1)).length() < 0.0001)
		{
			mat4Rotation = info->InitMat;
		}

		NodeTransformation = ParentMatrix * mat4Translation * mat4Rotation * mat4Scaling;

		//更新动态骨骼
		auto itr = dynamicBonaMgr->transformMap.find(info->bone_name);
		if (itr != dynamicBonaMgr->transformMap.end())
		{
			CC3DMath::CC3DMatrix rot_mat(NodeTransformation[0][0], NodeTransformation[1][0], NodeTransformation[2][0], NodeTransformation[3][0],
				NodeTransformation[0][1], NodeTransformation[1][1], NodeTransformation[2][1], NodeTransformation[3][1],
				NodeTransformation[0][2], NodeTransformation[1][2], NodeTransformation[2][2], NodeTransformation[3][2],
				NodeTransformation[0][3], NodeTransformation[1][3], NodeTransformation[2][3], NodeTransformation[3][3]);

			dynamicBonaMgr->LateUpdate(info->db_index);

			float3 de_scale, de_translate;
			quaternion4f de_rotate;
			glm::mat4 newMat;
			rot_mat.decompose(&de_scale, &de_rotate, &de_translate);
			de_translate.set(NodeTransformation[3][0], NodeTransformation[3][1], NodeTransformation[3][2]);
			glm::mat4 temp_scale = glm::scale(newMat, glm::vec3(de_scale.x, de_scale.y, de_scale.z));
			glm::mat4 temp_rotation = CC3DUtils::QuaternionToMatrix(Vector4(itr->second->worldRotation.x, itr->second->worldRotation.y, itr->second->worldRotation.z, itr->second->worldRotation.w));
			glm::mat4 temp_translate = glm::translate(newMat, glm::vec3(de_translate.x, de_translate.y, de_translate.z));

			//最终只需要使用旋转参数进行所有的变化，所以需要把旋转的变换应用到该附属节点上，这里采用直接替换旋转四元数的方式才是正确的
			NodeTransformation = temp_translate * temp_rotation * temp_scale;
		}

		info->FinalTransformation = NodeTransformation;

		for (int i = 0; i < info->pChildren.size(); i++)
		{
			DFSBoneTree(info->pChildren[i], NodeTransformation);
		}

	}
}