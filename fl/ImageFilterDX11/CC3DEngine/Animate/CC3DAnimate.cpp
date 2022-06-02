
#include "CC3DAnimate.h"
// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/imgproc/imgproc.hpp>


CC3DAnimate::CC3DAnimate()
{
}

CC3DAnimate::~CC3DAnimate()
{
	for (int i = 0; i < m_ChannelInfo.size(); i++)
	{
		SAFE_DELETE(m_ChannelInfo[i]);
	}
	m_ChannelInfo.clear();
}

void CC3DAnimate::InitAnimate(uint32 AnimateIndex)
{
	if (AnimateIndex>= m_Model->animations.size())
	{
		return;
	}
	auto &Animate = m_Model->animations[AnimateIndex];

	m_AnimateName = Animate.name;

	for (int i=0;i<Animate.channels.size();i++)
	{
		auto &pChannel = Animate.channels[i];
		CC3DAnimationChannelInfo *pInfo = new CC3DAnimationChannelInfo();
		pInfo->nNodeID = pChannel.target_node;
		std::string sType = pChannel.target_path;
		if (sType == "translation")
		{
			pInfo->eType = TARGET_TRANSLATE;
		}
		else if(sType == "rotation")
		{
			pInfo->eType = TARGET_ROTATE;
		}
		else if (sType == "scale")
		{
			pInfo->eType = TARGET_SCALE;
		}
		else if (sType == "weights")
		{
			pInfo->eType = TARGET_WEIGHT;
		}
		auto sampleID = pChannel.sampler;
		auto Sample = Animate.samplers[sampleID];

		auto input = Sample.input;
		auto output = Sample.output;
		int type = 0;
		pInfo->pInputTime = (float*)Getdata(input, pInfo->nKeyFrame, type);
		pInfo->pOutputAnimateValue = (float*)Getdata(output, pInfo->nOutCount, type);
		auto maxVaue = m_Model->accessors[input].maxValues;
		m_ChannelInfo.push_back(pInfo);
		if (maxVaue.size()==1)
		{
			m_AnimateAllTime = (std::max)(m_AnimateAllTime*1.0, maxVaue[0]);
		}
		if (pInfo->nKeyFrame>0)
		{
			m_AnimateAllTime = (std::max)(m_AnimateAllTime, pInfo->pInputTime[pInfo->nKeyFrame-1]);
		}
		
	}
}

void CC3DAnimate::ReadSkeletonAnimate(CC3DModel * pModel, CC3DSkeleton * pSkeleton, const char * pAnimateFile)
{
	if (pSkeleton == NULL||pAnimateFile==NULL)
	{
		return;
	}
	FILE *pFileRead = fopen(pAnimateFile, "rb");
	fread(&m_AnimateAllTime, sizeof(float), 1, pFileRead);
	int nChannels;
	fread(&nChannels, sizeof(int), 1, pFileRead);
	for (int i = 0; i < nChannels; i++)
	{
		CC3DAnimationChannelInfo *pInfo = new CC3DAnimationChannelInfo();
		int nType ;
		fread(&nType, sizeof(int), 1, pFileRead);
		pInfo->eType = (CC3DChannelType)nType;
		if (pInfo->eType != TARGET_WEIGHT)
		{
			int nLen;
			fread(&nLen, sizeof(int), 1, pFileRead);
			char *boneName = new char[nLen + 1];
			boneName[nLen] = '\0';
			fread(boneName, nLen, 1, pFileRead);

			fread(&nLen, sizeof(int), 1, pFileRead);
			pInfo->nKeyFrame = nLen;
			pInfo->nOutCount = nLen;
			uint8 *pData = new uint8[nLen * sizeof(float)];

			fread(pData, nLen * sizeof(float), 1, pFileRead);
			pInfo->pInputTime = (float*)pData;
			m_pData.push_back(pData);


			if (pInfo->eType == TARGET_ROTATE)
			{
				pData = new uint8[nLen * sizeof(Vector4)];
				fread(pData, nLen * sizeof(Vector4), 1, pFileRead);
				pInfo->pOutputAnimateValue = (float *)pData;
			}
			else
			{
				pData = new uint8[nLen * sizeof(Vector3)];
				fread(pData, nLen * sizeof(Vector3), 1, pFileRead);
				pInfo->pOutputAnimateValue = (float *)pData;
			}
			m_pData.push_back(pData);

			//std::string boneNameStr;
			auto itr = pSkeleton->m_BoneMap.find(boneName);

			SAFE_DELETE_ARRAY(boneName);
			if (itr != pSkeleton->m_BoneMap.end())
			{
				int skletonID = itr->second;
				pInfo->nNodeID = pSkeleton->m_BoneNode[skletonID]->NodeID;
			}
			else
			{
				SAFE_DELETE(pInfo);
				continue;
			}
		}

		else
		{
			int nLen;
			fread(&nLen, sizeof(int), 1, pFileRead);
			char *MeshName = new char[nLen + 1];
			MeshName[nLen] = '\0';
			fread(MeshName, nLen, 1, pFileRead);
			fread(&nLen, sizeof(int), 1, pFileRead);
			int nBS = 0;
			fread(&nBS, sizeof(int), 1, pFileRead);
			pInfo->nKeyFrame = nLen;
			pInfo->nOutCount = nLen * nBS;
			uint8 *pData = new uint8[nLen * sizeof(float)];
			fread(pData, nLen * sizeof(float), 1, pFileRead);
			pInfo->pInputTime = (float*)pData;
			m_pData.push_back(pData);
			pData = new uint8[nLen *nBS* sizeof(float)];
			fread(pData, nLen *nBS * sizeof(float), 1, pFileRead);
			pInfo->pOutputAnimateValue = (float *)pData;

			m_pData.push_back(pData);

			//std::string boneNameStr;
			auto itr = pSkeleton->m_BoneMap.find(MeshName);
			for (int k = 0; k < pModel->m_ModelMesh.size(); k++)
			{
				if (pModel->m_ModelMesh[k]->m_Meshname == MeshName&&pModel->m_ModelMesh[k]->m_pBlendShape.size()== nBS)
				{
					pInfo->nNodeID = pModel->m_ModelMesh[k]->m_nNodeID;
				}
			}
			SAFE_DELETE_ARRAY(MeshName);
		}

		m_ChannelInfo.push_back(pInfo);
	}

	fclose(pFileRead);
}

void CC3DAnimate::play(float fSecond, CC3DModel * pModel, CC3DSkeleton *pSkeleton)
{
	int nChannels = m_ChannelInfo.size();
	for (int i=0;i<nChannels;i++)
	{
		CC3DAnimationChannelInfo *pInfo = m_ChannelInfo[i];
		if (pInfo->eType == TARGET_WEIGHT)
		{
			playBlendShape(fSecond, pModel, pInfo);
		}
		else
		{
			playSkeleton(fSecond, pSkeleton, pModel, pInfo);
		}

	}
//	WriteBin(pSkeleton,pModel);
// 	if (pSkeleton != NULL)
// 	{
// 		pSkeleton->UpdateBone();
// 	}
// 	pModel->m_CameraAnimate.UpdateNode();
// 	pModel->UpdateNode();
}

void CC3DAnimate::playCam(float fSecond, CC3DModel * pModel, int nCamIndex)
{
	int nChannels = m_ChannelInfo.size();
	for (int i = 0; i < nChannels; i++)
	{
		CC3DAnimationChannelInfo *pInfo = m_ChannelInfo[i];
		if (pInfo->eType != TARGET_WEIGHT)
		{
			uint32 nNodeID = pInfo->nNodeID;
			CC3DNodeInfo *pBoneInfo = pModel->m_ModelNode->m_Node[nNodeID];
			int nKeyL = -1, nKeyR = -1;
			float alpha;
			pInfo->findKey(fSecond+pInfo->pInputTime[0], nKeyL, nKeyR, alpha);
			if (nKeyL >= 0)
			{
				if (pInfo->eType == TARGET_ROTATE)
				{
					Vector4 *pL = (Vector4 *)pInfo->pOutputAnimateValue;
					Vector4 *pR = (Vector4 *)pInfo->pOutputAnimateValue;
					//pBoneInfo->TargetRotation = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
					CC3DUtils::QuaternionInterpolate(pBoneInfo->Rotation, pL[nKeyL], pR[nKeyR], alpha);
				}
				else if (pInfo->eType == TARGET_SCALE)
				{
					Vector3 *pL = (Vector3 *)pInfo->pOutputAnimateValue;
					Vector3 *pR = (Vector3 *)pInfo->pOutputAnimateValue;
					pBoneInfo->Scale = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
				}
				else if (pInfo->eType == TARGET_TRANSLATE)
				{
					Vector3 *pL = (Vector3 *)pInfo->pOutputAnimateValue;
					Vector3 *pR = (Vector3 *)pInfo->pOutputAnimateValue;
					pBoneInfo->Translate = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
				}
			}
		}
	}
	//pModel->m_CameraAnimate.UpdateNodeIndex(nCamIndex);
	pModel->m_CameraNode.UpdateNode();

}

void CC3DAnimate::playModel(float fSecond, CC3DModel * pModel,bool isLoop)
{
	int nChannels = m_ChannelInfo.size();
	for (int i = 0; i < nChannels; i++)
	{
		CC3DAnimationChannelInfo *pInfo = m_ChannelInfo[i];
		if (pInfo->eType != TARGET_WEIGHT)
		{
			uint32 nNodeID = pInfo->nNodeID;
			CC3DNodeInfo *pBoneInfo = pModel->m_ModelNode->m_Node[nNodeID];
			int nKeyL = -1, nKeyR = -1;
			float alpha;
			if (isLoop)
			{
				int nAllTime = pInfo->pInputTime[pInfo->nKeyFrame - 1] - pInfo->pInputTime[0];
				int nTmp = fSecond / nAllTime;

				float during = fSecond - nTmp * nAllTime;

				pInfo->findKey(during + pInfo->pInputTime[0], nKeyL, nKeyR, alpha);
			}
			else
			{
				pInfo->findKey(fSecond + pInfo->pInputTime[0], nKeyL, nKeyR, alpha);
			}

			if (nKeyL >= 0)
			{
				if (pInfo->eType == TARGET_ROTATE)
				{
					Vector4 *pL = (Vector4 *)pInfo->pOutputAnimateValue;
					Vector4 *pR = (Vector4 *)pInfo->pOutputAnimateValue;
					//pBoneInfo->TargetRotation = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
					CC3DUtils::QuaternionInterpolate(pBoneInfo->Rotation, pL[nKeyL], pR[nKeyR], alpha);
				}
				else if (pInfo->eType == TARGET_SCALE)
				{
					Vector3 *pL = (Vector3 *)pInfo->pOutputAnimateValue;
					Vector3 *pR = (Vector3 *)pInfo->pOutputAnimateValue;
					pBoneInfo->Scale = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
				}
				else if (pInfo->eType == TARGET_TRANSLATE)
				{
					Vector3 *pL = (Vector3 *)pInfo->pOutputAnimateValue;
					Vector3 *pR = (Vector3 *)pInfo->pOutputAnimateValue;
					pBoneInfo->Translate = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
				}
			}
		}
	}

	pModel->UpdateNode();
}

void CC3DAnimate::WriteBin(CC3DSkeleton * pSkeleton, CC3DModel *pModel )
{
	int nChannels = m_ChannelInfo.size();
	int j = 0;
	for (int i = 0; i < nChannels; i++)
	{
		CC3DAnimationChannelInfo *pInfo = m_ChannelInfo[i];
		if (pInfo->eType != TARGET_WEIGHT)
		{
			uint32 nNodeID = pInfo->nNodeID;
			if (pSkeleton != NULL && pSkeleton->m_NodeBoneMap.find(nNodeID) != pSkeleton->m_NodeBoneMap.end())
			{
				j++;
			}
		}
		else
		{
			for (int i = 0; i < pModel->m_ModelMesh.size(); i++)
			{
				if (pModel->m_ModelMesh[i]->m_nNodeID == pInfo->nNodeID)
				{
					j++;
					break;
				}
			}
		}
	}
	if (j<1)
	{
		return;
	}


	FILE *pFileWrite = fopen("animate.bin", "wb");

	fwrite(&m_AnimateAllTime, sizeof(float), 1, pFileWrite);
	fwrite(&j, sizeof(int), 1, pFileWrite);
	for (int i = 0; i < nChannels; i++)
	{
		CC3DAnimationChannelInfo *pInfo = m_ChannelInfo[i];
		if (pInfo->eType != TARGET_WEIGHT)
		{
			uint32 nNodeID = pInfo->nNodeID;
			if (pSkeleton != NULL && pSkeleton->m_NodeBoneMap.find(nNodeID) != pSkeleton->m_NodeBoneMap.end())
			{

				int nType = pInfo->eType;
				fwrite(&nType, sizeof(int), 1, pFileWrite);
				CC3DBoneNodeInfo *pBoneInfo = pSkeleton->m_BoneNode[pSkeleton->m_NodeBoneMap[nNodeID]];
				std::string boneName = pBoneInfo->bone_name;
				int nlen = boneName.size() * sizeof(char);

				fwrite(&nlen, sizeof(int), 1, pFileWrite);
				fwrite(boneName.c_str(), nlen, 1, pFileWrite);

				nlen = pInfo->nKeyFrame;
				fwrite(&nlen, sizeof(int), 1, pFileWrite);
				fwrite(pInfo->pInputTime, nlen*sizeof(float), 1, pFileWrite);
				if (pInfo->eType == TARGET_ROTATE)
				{
					fwrite(pInfo->pOutputAnimateValue, nlen * sizeof(Vector4), 1, pFileWrite);
				}
				else
				{
					fwrite(pInfo->pOutputAnimateValue, nlen * sizeof(Vector3), 1, pFileWrite);
				}
			}
		}
		else
		{
			uint32 nNodeID = pInfo->nNodeID;
			for (int i = 0; i < pModel->m_ModelMesh.size(); i++)
			{

				if (pModel->m_ModelMesh[i]->m_nNodeID == nNodeID)
				{
					int nBlendShape = pInfo->nOutCount / pInfo->nKeyFrame;
					int nType = pInfo->eType;
					fwrite(&nType, sizeof(int), 1, pFileWrite);
					std::string m_Meshname = pModel->m_ModelMesh[i]->m_Meshname;
					int nlen = m_Meshname.size() * sizeof(char);
					fwrite(&nlen, sizeof(int), 1, pFileWrite);
					fwrite(m_Meshname.c_str(), nlen, 1, pFileWrite);
					nlen = pInfo->nKeyFrame;
					fwrite(&nlen, sizeof(int), 1, pFileWrite);
					fwrite(&nBlendShape, sizeof(int), 1, pFileWrite);
					fwrite(pInfo->pInputTime, nlen * sizeof(float), 1, pFileWrite);
					fwrite(pInfo->pOutputAnimateValue, nlen * nBlendShape*sizeof(float), 1, pFileWrite);
				}
			}

		}
	}

	fclose(pFileWrite);
}

void CC3DAnimate::playBlendShape(float fSecond, CC3DModel * pModel, CC3DAnimationChannelInfo * pInfo)
{
	uint32 nNodeID = pInfo->nNodeID;
	
	if (nNodeID>= m_Model->nodes.size())
	{
		return;
	}

	auto pNode = m_Model->nodes[nNodeID];
	if (pNode.mesh>=0&&pNode.mesh<pModel->m_ModelMesh.size())
	{
		int nBlendShape = pInfo->nOutCount / pInfo->nKeyFrame;
		int nKeyL = -1, nKeyR = -1;
		float alpha;
		pInfo->findKey(fSecond, nKeyL, nKeyR, alpha);
		std::vector<float>vWeight(nBlendShape);
		if (nKeyL>=0)
		{
			float *pL = pInfo->pOutputAnimateValue + nKeyL * nBlendShape;
			float *pR = pInfo->pOutputAnimateValue + nKeyR * nBlendShape;

			for (int i=0;i<nBlendShape;i++)
			{
				vWeight[i] = pL[i] * (1.0 - alpha) + pR[i] * alpha;
			}
		}
		for (int i = 0; i < pModel->m_ModelMesh.size(); i++)
		{

			if (pModel->m_ModelMesh[i]->m_nNodeID == nNodeID)
			{
				pModel->m_ModelMesh[i]->GenVertWithWeights(vWeight);
			}
		}


	}
}


void CC3DAnimate::playSkeleton(float fSecond, CC3DSkeleton * pSkeleton, CC3DModel *pModel, CC3DAnimationChannelInfo * pInfo)
{
	int nNodeID = pInfo->nNodeID;
	if (pSkeleton != NULL&&pSkeleton->m_NodeBoneMap.find(nNodeID) != pSkeleton->m_NodeBoneMap.end())
	{
		CC3DBoneNodeInfo *pBoneInfo= pSkeleton->m_BoneNode[pSkeleton->m_NodeBoneMap[nNodeID]];
		int nKeyL = -1, nKeyR = -1;
		float alpha;
		pInfo->findKey(fSecond, nKeyL, nKeyR, alpha);
		if (nKeyL >=0)
		{
			if (pInfo->eType == TARGET_ROTATE)
			{
				Vector4 *pL = (Vector4 *)pInfo->pOutputAnimateValue;
				Vector4 *pR = (Vector4 *) pInfo->pOutputAnimateValue;
				//pBoneInfo->TargetRotation = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
				CC3DUtils::QuaternionInterpolate(pBoneInfo->TargetRotation, pL[nKeyL], pR[nKeyR],alpha);
			}
			else if (pInfo->eType == TARGET_SCALE)
			{
				Vector3 *pL = (Vector3 *)pInfo->pOutputAnimateValue;
				Vector3 *pR = (Vector3 *)pInfo->pOutputAnimateValue;
				pBoneInfo->TargetScale = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
			}
			else if (pInfo->eType == TARGET_TRANSLATE)
			{
				Vector3 *pL = (Vector3 *)pInfo->pOutputAnimateValue;
				Vector3 *pR = (Vector3 *)pInfo->pOutputAnimateValue;
				pBoneInfo->TargetTranslate = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
			}
		}
	}
	else
	{
		CC3DNodeInfo *pNodeInfo = pModel->m_ModelNode->m_Node[nNodeID];
		int nKeyL = -1, nKeyR = -1;
		float alpha;
		pInfo->findKey(fSecond, nKeyL, nKeyR, alpha);
		if (nKeyL >= 0)
		{
			if (pInfo->eType == TARGET_ROTATE)
			{
				Vector4 *pL = (Vector4 *)pInfo->pOutputAnimateValue;
				Vector4 *pR = (Vector4 *)pInfo->pOutputAnimateValue;
				//pBoneInfo->TargetRotation = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
				CC3DUtils::QuaternionInterpolate(pNodeInfo->Rotation, pL[nKeyL], pR[nKeyR], alpha);
			}
			else if (pInfo->eType == TARGET_SCALE)
			{
				Vector3 *pL = (Vector3 *)pInfo->pOutputAnimateValue;
				Vector3 *pR = (Vector3 *)pInfo->pOutputAnimateValue;
				pNodeInfo->Scale = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
			}
			else if (pInfo->eType == TARGET_TRANSLATE)
			{
				Vector3 *pL = (Vector3 *)pInfo->pOutputAnimateValue;
				Vector3 *pR = (Vector3 *)pInfo->pOutputAnimateValue;
				pNodeInfo->Translate = pL[nKeyL] * (1.0 - alpha) + pR[nKeyR] * alpha;
			}
		}
		hasModelAnimate = true;
	}
}

