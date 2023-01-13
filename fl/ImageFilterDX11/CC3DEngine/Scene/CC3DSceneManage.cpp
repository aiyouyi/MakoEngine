
#include "CC3DSceneManage.h"
#include "Common/CC3DEnvironmentConfig.h"

namespace CC3DImageFilter
{
	CC3DSceneManage::CC3DSceneManage(void)
	{

	}

	CC3DSceneManage::~CC3DSceneManage(void)
	{
		Release();
		//SAFE_DELETE(cameraMan);
	}

	void CC3DSceneManage::Release()
	{
		for (int i = 0; i < m_Model.size(); i++)
		{
			SAFE_DELETE(m_Model[i]);
			SAFE_DELETE(m_gltfModel[i]);
		}
		for (int i = 0; i < m_Light.size(); i++)
		{
			SAFE_DELETE(m_Light[i]);
		}
		for (int i = 0; i < m_Animate.size(); i++)
		{
			SAFE_DELETE(m_Animate[i]);
		}
		for (int i = 0; i < m_Skeleton.size(); i++)
		{
			SAFE_DELETE(m_Skeleton[i]);
		}

		m_Skeleton.clear();
		m_Model.clear();
		m_gltfModel.clear();
		m_Light.clear();
		m_Animate.clear();
		SkeletonManage.Skeletons.clear();

	}

	void CC3DSceneManage::LoadMainModel(char* pFileName, const uint8* pDataBuffer, int nLen)
	{
		HasLoadModel = true;
		Release();

		AddModel(pFileName, NULL, pDataBuffer, nLen);

		SetDefaultModelControl();

	}

	void CC3DSceneManage::AddModel(char* pFileName, char* AttatchBone, const uint8* pDataBuffer, int nLen)
	{
		std::string ext = GetFilePathExtension(pFileName);
		std::string err;
		std::string warn;
		bool ret = false;

		tinygltf::Model* model = new tinygltf::Model();
		if (ext.compare("glb") == 0) {
			// assume binary glTF.
			if (pDataBuffer != NULL)
			{
				ret = gltf_ctx.LoadBinaryFromMemory(model, &err, &warn,
					pDataBuffer, nLen);
			}
			else
			{
				ret = gltf_ctx.LoadBinaryFromFile(model, &err, &warn,
					pFileName);
			}
		}
		else if (ext.compare("glTF") == 0 || ext.compare("gltf") == 0) {

			if (pDataBuffer != NULL)
			{
				ret = gltf_ctx.LoadASCIIFromString(model, &err, &warn,
					(const char*)pDataBuffer, nLen, "./");
			}
			else
			{
				ret = gltf_ctx.LoadASCIIFromFile(model, &err, &warn, pFileName);
			}
		}
		else
		{
			LOGE("Only Support gltf or glb!");
		}
		if (!warn.empty()) {
			LOGE("Warn: %s", warn.c_str());
		}

		if (!err.empty()) {
			LOGE("Err: %s", err.c_str());
		}

		if (!ret) {
			LOGE("Failed to parse glTF");
			SAFE_DELETE(model);
		}
		else
		{
			m_gltfModel.push_back(model);
			CC3DModel* ccModel = new CC3DModel();
			ccModel->initModel(model);
			ccModel->LoadModelInfo();
			m_Model.push_back(ccModel);
			LoadSkeleton(AttatchBone);
			LoadAnimate();
		}
	}

	void CC3DSceneManage::AddLight(CC3DLight* pLight)
	{
		m_Light.push_back(pLight);
	}

	void CC3DSceneManage::LoadAnimate()
	{
		int nModel = m_Model.size();
		if (nModel > 0)
		{
			CC3DAnimateManage* pAnimate = new CC3DAnimateManage();
			pAnimate->initModel(m_gltfModel[nModel - 1]);
			pAnimate->InitAnimate();
			m_Animate.push_back(pAnimate);
		}
	}

	void CC3DSceneManage::LoadSkeleton(char* AttatchBone)
	{
		int nModel = m_Model.size();
		if (nModel > 0)
		{
			CC3DSkeleton* pSkeleton = new CC3DSkeleton();
			pSkeleton->m_ModelNode = m_Model[nModel - 1]->m_ModelNode;
			pSkeleton->initModel(m_gltfModel[nModel - 1]);
			pSkeleton->InitSkeleton();
			m_Skeleton.push_back(pSkeleton);

			//反射到界面进行图层添加
			if (!pSkeleton->m_BoneMap.empty())
			{
				sigAddSkeletonLayer(pSkeleton->m_BoneMap);
			}


			SkeletonInfo skeletonInfo;
			skeletonInfo.pSkeleton = pSkeleton;
			skeletonInfo.AttachBone = "";
			if (AttatchBone != NULL)
			{
				skeletonInfo.AttachBone = AttatchBone;
			}

			SkeletonManage.Skeletons.push_back(skeletonInfo);
		}
	}

	void CC3DSceneManage::DeleteLastModel()
	{
		if (m_Model.size() > 1)
		{
			SAFE_DELETE(m_Model[m_Model.size() - 1]);
			SAFE_DELETE(m_gltfModel[m_Model.size() - 1]);
			SAFE_DELETE(m_Skeleton[m_Model.size() - 1]);
			SAFE_DELETE(m_Animate[m_Model.size() - 1]);
			m_Model.pop_back();
			m_gltfModel.pop_back();
			m_Animate.pop_back();
			m_Skeleton.pop_back();
			SkeletonManage.Skeletons.pop_back();
		}
	}

	void CC3DSceneManage::DeleteModel(int Index)
	{

		if (Index > 0 && m_Model.size() > Index)
		{
			SAFE_DELETE(m_Model[Index]);
			SAFE_DELETE(m_gltfModel[Index]);
			SAFE_DELETE(m_Skeleton[Index]);
			SAFE_DELETE(m_Animate[Index]);

			m_Model.erase(m_Model.begin() + Index);
			m_gltfModel.erase(m_gltfModel.begin() + Index);
			m_Skeleton.erase(m_Skeleton.begin() + Index);
			m_Animate.erase(m_Animate.begin() + Index);
			SkeletonManage.Skeletons.erase(SkeletonManage.Skeletons.begin() + Index);
		}
	}

	void CC3DSceneManage::DelayLoadModelResource(CC3DImageFilter::EffectConfig* EffectConfig)
	{
		for (auto Item : m_Model)
		{
			Item->LoadResource(EffectConfig);
		}

		SetDefaultCamera();
		setDefaultLookAt();
		SetDefaultLight();
		SetDefaultProject();
		HasLoadModel = false;
		m_nSelectCamera = 0;
	}

	void CC3DSceneManage::play(float fSecond)
	{
		for (int i = 0; i < m_Animate.size(); i++)
		{
			m_Animate[i]->play(fSecond, m_Model[i], m_Skeleton[i]);
			if (m_Animate[i]->m_hasModelAnimate)
			{
				m_Model[i]->m_CameraNode.UpdateNode();
				m_Model[i]->UpdateNode();
			}
		}

		SkeletonManage.UpdateSkeleton();

		//将被附着节点的变换矩阵应用到新增模型上
		for (int i = 1; i < m_Model.size(); i++)
		{
			m_Model[i]->UpdateNode();
			//for (int j = 0; j < m_Model[i]->m_ModelMesh.size(); j++)
			//{
			//	m_Model[i]->m_ModelMesh[j]->m_MeshMat = m_Skeleton[i]->HasNoBoneMat * m_Model[i]->m_ModelMesh[j]->m_MeshMat;
			//}
		}
	}

	void CC3DSceneManage::playOnce(float fSecond)
	{
		for (int i = 0; i < m_Animate.size(); i++)
		{
			m_Animate[i]->playOnce(fSecond, m_Model[i], m_Skeleton[i]);
			if (m_Animate[i]->m_hasModelAnimate)
			{
				m_Model[i]->m_CameraNode.UpdateNode();
				m_Model[i]->UpdateNode();
			}
		}

		SkeletonManage.UpdateSkeleton();
	}


	void CC3DSceneManage::ReadSkeletonAnimate(const char* pAnimateFile)
	{
		int nModel = m_Model.size();
		if (nModel > 0)
		{
			SAFE_DELETE(m_Animate[0]);
			CC3DAnimateManage* pAnimate = new CC3DAnimateManage();
			pAnimate->initModel(m_gltfModel[nModel - 1]);
			pAnimate->ReadSkeletonAnimate(m_Model[0], m_Skeleton[0], pAnimateFile);
			m_Animate[0] = pAnimate;
		}
	}

	void CC3DSceneManage::ReadSkeletonAnimateData(const char* pAnimateData)
	{
		int nModel = m_Model.size();
		if (nModel > 0)
		{
			SAFE_DELETE(m_Animate[0]);
			CC3DAnimateManage* pAnimate = new CC3DAnimateManage();
			pAnimate->initModel(m_gltfModel[nModel - 1]);
			pAnimate->ReadSkeletonAnimateData(m_Model[0], m_Skeleton[0], pAnimateData);
			m_Animate[0] = pAnimate;
		}
	}

	void CC3DSceneManage::AddSkeletonAnnimateData(const char* pAnimateData, const std::string& animName)
	{
		int nModel = m_Model.size();
		if (nModel > 0)
		{
			if (m_Animate.empty())
			{
				CC3DAnimateManage* pAnimate = new CC3DAnimateManage();
				pAnimate->initModel(m_gltfModel[nModel - 1]);
				pAnimate->AddSkeletonAnimateData(m_Model[0], m_Skeleton[0], pAnimateData, animName);
				m_Animate.push_back(pAnimate);
			}
			else
			{
				m_Animate[0]->AddSkeletonAnimateData(m_Model[0], m_Skeleton[0], pAnimateData, animName);
			}
		}
	}

	void CC3DSceneManage::AddSkeletonAnnimateDataForBlend(const char* pAnimateData, const std::string& animName, const std::string& boneName, float blendWeight)
	{
		int nModel = m_Model.size();
		if (nModel > 0)
		{
			if (m_Animate.empty())
			{
				CC3DAnimateManage* pAnimate = new CC3DAnimateManage();
				pAnimate->initModel(m_gltfModel[nModel - 1]);
				pAnimate->AddSkeletonAnimateDataForBlend(m_Model[0], m_Skeleton[0], pAnimateData, animName, boneName, blendWeight);
				m_Animate.push_back(pAnimate);
			}
			else
			{
				m_Animate[0]->AddSkeletonAnimateDataForBlend(m_Model[0], m_Skeleton[0], pAnimateData, animName, boneName, blendWeight);
			}
		}
	}

	void CC3DSceneManage::ReSortAnimateTimeline(float blendTime)
	{
		if (!m_Animate.empty())
		{
			m_Animate[0]->ReSortAnimateTimeline(blendTime);
		}
	}

	void CC3DSceneManage::LoadAnimationJson(const std::string& fileName)
	{
		if (m_Animate.size() > 0 && m_Skeleton.size() > 0)
		{
			m_Animate[0]->LoadAnimateJson(fileName, m_Skeleton[0]);
		}
	}

	void CC3DSceneManage::SetDefaultLight()
	{
		if (m_Model.size() > 0)
		{
			Vector3 MaxPoint = m_Model[0]->m_ModelBox.maxPoint;
			Vector3 minPoint = m_Model[0]->m_ModelBox.minPoint;
			Vector3 CenterPoint = m_Model[0]->m_ModelBox.centerPoint;
			CC3DDirectionalLight* light1 = new CC3DDirectionalLight();

			Vector3 Lightdir = Vector3(CenterPoint.x, CenterPoint.y, MaxPoint.z) - CenterPoint;

			glm::vec4  tmp = glm::vec4(Lightdir.x, Lightdir.y, Lightdir.z, 0.0);

			tmp = m_ModelControl.m_InitMatrix * tmp;

			light1->SetLightDirection(glm::vec3(Lightdir.x, Lightdir.y, Lightdir.z));
			light1->SetLightAmbientIntensity(1.0);
			light1->SetLightDiffuseIntensity(3.f);

			AddLight(light1);
		}

	}

	void CC3DSceneManage::SetDefaultCamera()
	{
		if (m_Model.size() > 0)
		{

			Vector3 Distanse = m_Model[0]->m_ModelBox.maxPoint - m_Model[0]->m_ModelBox.minPoint;

			float z = (std::max)(Distanse.x, (std::max)(Distanse.y, Distanse.z));

			z = z * 2;
			m_Camera.SetDistanceToRO(z);
			m_Camera.SetCameraPos(glm::vec3(0, 0, z));

		}
	}

	void CC3DSceneManage::SetDefaultProject()
	{
		if (m_Model.size() > 0)
		{
			Vector3 Distanse = m_Model[0]->m_ModelBox.maxPoint - m_Model[0]->m_ModelBox.minPoint;

			float z = (std::max)(Distanse.x, (std::max)(Distanse.y, Distanse.z));

			m_Project.SetFar(4 * z);
			m_Project.SetNear(0.5 * z);
		}
	}

	void CC3DSceneManage::setDefaultLookAt()
	{
		if (m_Model.size() > 0)
		{
			//Vector3 Distanse = m_Model[0]->m_ModelBox.maxPoint - m_Model[0]->m_ModelBox.minPoint;

			//float z = (std::max)(Distanse.x, (std::max)(Distanse.y, Distanse.z));

			Vector3 model_center = m_Model[0]->m_ModelBox.maxPoint * 0.5f + m_Model[0]->m_ModelBox.minPoint * 0.5f;

			//m_Camera.SetLookAtPos(glm::vec3(model_center.x, model_center.y, model_center.z));

			//m_Model[0]->transform_matrix = glm::translate(m_Model[0]->transform_matrix, -glm::vec3(model_center.x, model_center.y, model_center.z));
			//m_Model[0]->transform_matrix = glm::translate(m_Model[0]->transform_matrix, glm::vec3(0.0f, 0.0f, -4.0f));
		}
	}

	void CC3DSceneManage::SetDefaultModelControl()
	{
		if (m_Model.size() > 0)
		{
			m_ModelControl.m_InitMatrix = glm::mat4();
			// 		int nodeNum = m_gltfModel[0]->nodes.size();
			// 		for (int i=0;i<nodeNum;i++)
			// 		{
			// 			if (m_gltfModel[0]->nodes[i].mesh == 0)
			// 			{
			// 				auto &node = m_gltfModel[0]->nodes[i];
			// 				if (node.rotation.size()==4)
			// 				{
			// 				//	m_ModelControl.m_InitMatrix = CC3DUtils::QuaternionToMatrix(Vector4(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]));
			// 				}
			// 			}
			// 		}
			m_ModelControl.m_InitMatrix = glm::translate(m_ModelControl.m_InitMatrix, glm::vec3(-m_Model[0]->m_ModelBox.centerPoint.x, -m_Model[0]->m_ModelBox.centerPoint.y, -m_Model[0]->m_ModelBox.centerPoint.z));

			//m_ModelControl.moveToOrignMat = glm::translate(m_ModelControl.m_InitMatrix, glm::vec3(0.0, 0.0, 4.0f));
			//m_ModelControl.moveToInitMat = glm::translate(m_ModelControl.m_InitMatrix, glm::vec3(0.0, 0.0, -4.0f));

			//m_ModelControl.m_InitMatrix = glm::translate(m_ModelControl.m_InitMatrix, glm::vec3(0.0,0.0, -4.0f));
	// 
	// 		if (m_Skeleton.size() > 0 && m_Skeleton[0]->m_RootNode.size() > 0 && m_Model[0]->m_hasSkin)
	// 		{
	// 
	// 			glm::mat4 transformMat = glm::inverse(m_Skeleton[0]->m_RootNode[0]->FinalTransformation *m_Skeleton[0]->m_RootNode[0]->InverseBindMat);
	// 
	// 			m_ModelControl.m_InitMatrix = m_ModelControl.m_InitMatrix*transformMat;
	// 		}
			m_ModelControl.UpdateModelMatrix();

		}
	}
}
