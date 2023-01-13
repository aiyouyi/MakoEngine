
#include "CC3DModel.h"
#include "MaterialCreator.h"
#include "GLMaterialCreator.h"
#include "CC3DEngine/Common/CC3DEnvironmentConfig.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CC3DModel::CC3DModel()
{
	m_ModelNode = NULL;
}

CC3DModel::~CC3DModel()
{
	ReleaseModel();
}

void CC3DModel::LoadModelInfo()
{
	ReleaseModel();
	LoadNode();
	
}

void CC3DModel::ReleaseModel()
{
	for (int i = 0; i < m_ModelMesh.size(); i++)
	{
		SAFE_DELETE(m_ModelMesh[i]);
	}
	for (int i = 0; i < m_ModelMaterial.size(); i++)
	{
		SAFE_DELETE(m_ModelMaterial[i]);
	}
	//for (int i = 0; i < m_ModelTexture.size(); i++)
	//{
	//	SAFE_DELETE(m_ModelTexture[i]);
	//}
	m_ModelMesh.clear();
	m_ModelMaterial.clear();
	m_ModelTexture.clear();
	SAFE_DELETE(m_ModelNode);
}

void CC3DModel::UpdateNode()
{
	for (int i = 0; i < m_ModelMesh.size(); i++)
	{
		m_ModelNode->UpdateNodeParent(m_ModelNode->m_Node[m_ModelMesh[i]->m_nNodeID]);
		m_ModelMesh[i]->m_MeshMat = m_ModelNode->m_Node[m_ModelMesh[i]->m_nNodeID]->FinalMeshMat;
	}
}

void CC3DModel::LoadResource(CC3DImageFilter::EffectConfig* EffectConfig)
{
	LoadTexture();
	LoadMaterial(EffectConfig);
	LoadMesh();
}

void CC3DModel::TransformBoundingBox(glm::mat4 & TransformMat)
{
	bool isInit = false;
	for (int i = 0; i < m_ModelMesh.size(); i++)
	{
		if (m_hasSkin && m_ModelMesh[i]->m_mesh->pBoneWeights == NULL)
		{
			continue;
		}

		glm::vec4 tmp = glm::vec4(m_ModelMesh[i]->m_meshBox.minPoint.x, m_ModelMesh[i]->m_meshBox.minPoint.y, m_ModelMesh[i]->m_meshBox.minPoint.z, 1.0);

		tmp = TransformMat * tmp;
		tmp = tmp / tmp.w;
		if (!isInit)
		{
			m_ModelBox.minPoint.x = tmp.x;
			m_ModelBox.minPoint.y = tmp.y;
			m_ModelBox.minPoint.z = tmp.z;
			m_ModelBox.maxPoint.x = tmp.x;
			m_ModelBox.maxPoint.y = tmp.y;
			m_ModelBox.maxPoint.z = tmp.z;
			isInit = true;
			continue;
		}


		m_ModelBox.minPoint.x = (std::min)(m_ModelBox.minPoint.x, tmp.x);
		m_ModelBox.minPoint.y = (std::min)(m_ModelBox.minPoint.y, tmp.y);
		m_ModelBox.minPoint.z = (std::min)(m_ModelBox.minPoint.z, tmp.z);

		m_ModelBox.maxPoint.x = (std::max)(m_ModelBox.maxPoint.x, tmp.x);
		m_ModelBox.maxPoint.y = (std::max)(m_ModelBox.maxPoint.y, tmp.y);
		m_ModelBox.maxPoint.z = (std::max)(m_ModelBox.maxPoint.z, tmp.z);
	}
	m_ModelBox.centerPoint = m_ModelBox.minPoint*0.5f + m_ModelBox.maxPoint*0.5f;

}

void CC3DModel::LoadTexture()
{
	for (int i=0;i<m_Model->images.size();i++)
	{
		auto& ModelImage = m_Model->images[i];

		uint8* pData = (uint8*)ModelImage.image.data();
		std::shared_ptr<CC3DTextureRHI> TextureRHI = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_NONE, ModelImage.width, ModelImage.height, pData, ModelImage.width * 4,true);
		m_ModelTexture.push_back(TextureRHI);
	}
	m_Model->images.clear();
}

void CC3DModel::LoadMaterial(CC3DImageFilter::EffectConfig* EffectConfig)
{
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		MaterialCreator::CreateMaterial(m_Model, m_ModelTexture, m_ModelMaterial,EffectConfig);
	}
	else if (GetDynamicRHI()->API == CC3DDynamicRHI::OPENGL)
	{
#ifndef _WIN64
		GLMaterialCreate::CreateMaterial(m_Model, m_ModelTexture, m_ModelMaterial,EffectConfig);
#endif
	}
}

void CC3DModel::LoadMesh()
{
	for (int i = 0; i < m_Model->meshes.size(); i++)
	{
		auto &ModelMesh = m_Model->meshes[i];
		for (int j=0;j<ModelMesh.primitives.size();j++)
		{
			CC3DMesh *pMesh = new CC3DMesh();
			pMesh->initModel(m_Model);
			pMesh->InitMesh(i,j, m_ModelMaterial, m_ModelNode);

			if (pMesh->m_mesh->pBoneWeights != NULL)
			{
				m_hasSkin = true;
			}
			m_ModelMesh.push_back(pMesh);
		}
	}
	/*std::vector <std::string> name;
	for (int i = 0; i < m_ModelMesh.size(); i++)
	{

		for (int j = 0; j < m_ModelMesh[i]->m_pBlendShapeName.size(); j++)
		{
			std::vector <std::string> BsName = m_ModelMesh[i]->m_pBlendShapeName;
			std::string subName = BsName[j].substr(BsName[j].find('.')+1);
			name.push_back(subName);
		}
	}
	sort(name.begin(), name.end());
	name.erase(unique(name.begin(), name.end()), name.end());
	for (int i = 0; i < name.size(); i++)
	{
		std::cout << "\""<< name[i] << "\"" << endl;
	}*/

	bool isInit = false;
	for (int i = 0; i < m_ModelMesh.size(); i++)
	{
// 		if (m_hasSkin && m_ModelMesh[i]->m_mesh->pBoneWeights == NULL)
// 		{
// 			continue;
// 		}

		glm::vec4 tmp = glm::vec4(m_ModelMesh[i]->m_meshBox.minPoint.x, m_ModelMesh[i]->m_meshBox.minPoint.y, m_ModelMesh[i]->m_meshBox.minPoint.z, 1.0);
		glm::vec4 tmp2 = glm::vec4(m_ModelMesh[i]->m_meshBox.maxPoint.x, m_ModelMesh[i]->m_meshBox.maxPoint.y, m_ModelMesh[i]->m_meshBox.maxPoint.z, 1.0);

// 		if (m_ModelMesh[i]->m_nSkinID<0)
// 		{
			tmp = m_ModelMesh[i]->m_MeshMat * tmp;
			tmp = tmp / tmp.w;

			tmp2 = m_ModelMesh[i]->m_MeshMat * tmp2;
			tmp2 = tmp2 / tmp2.w;
//		}
		if (!isInit)
		{
			m_ModelBox.minPoint.x = tmp.x;
			m_ModelBox.minPoint.y = tmp.y;
			m_ModelBox.minPoint.z = tmp.z;
			m_ModelBox.maxPoint.x = tmp.x;
			m_ModelBox.maxPoint.y = tmp.y;
			m_ModelBox.maxPoint.z = tmp.z;
			isInit = true;
		}


		m_ModelBox.minPoint.x = (std::min)(m_ModelBox.minPoint.x, tmp.x);
		m_ModelBox.minPoint.y = (std::min)(m_ModelBox.minPoint.y, tmp.y);
		m_ModelBox.minPoint.z = (std::min)(m_ModelBox.minPoint.z, tmp.z);

		m_ModelBox.maxPoint.x = (std::max)(m_ModelBox.maxPoint.x, tmp.x);
		m_ModelBox.maxPoint.y = (std::max)(m_ModelBox.maxPoint.y, tmp.y);
		m_ModelBox.maxPoint.z = (std::max)(m_ModelBox.maxPoint.z, tmp.z);

		m_ModelBox.minPoint.x = (std::min)(m_ModelBox.minPoint.x, tmp2.x);
		m_ModelBox.minPoint.y = (std::min)(m_ModelBox.minPoint.y, tmp2.y);
		m_ModelBox.minPoint.z = (std::min)(m_ModelBox.minPoint.z, tmp2.z);

		m_ModelBox.maxPoint.x = (std::max)(m_ModelBox.maxPoint.x, tmp2.x);
		m_ModelBox.maxPoint.y = (std::max)(m_ModelBox.maxPoint.y, tmp2.y);
		m_ModelBox.maxPoint.z = (std::max)(m_ModelBox.maxPoint.z, tmp2.z);

	}
// 	float flength = (m_ModelBox.maxPoint - m_ModelBox.minPoint).length();
// 	m_ModelBox.centerPoint = m_ModelBox.minPoint*0.5f + m_ModelBox.maxPoint*0.5f;
// 	if (flength < 2.f)
// 	{
// 		float Scale = 2.f / flength;
// 		glm::mat4 ScaleMat = glm::scale(glm::mat4(), glm::vec3(Scale, Scale, Scale));
// 		glm::mat4 Translate = glm::translate(glm::mat4(), glm::vec3(-m_ModelBox.centerPoint.x, -m_ModelBox.centerPoint.y, -m_ModelBox.centerPoint.z));
// 		for (int i = 0; i < m_ModelMesh.size(); i++)
// 		{
// 
// 			m_ModelMesh[i]->m_MeshMat = ScaleMat * Translate*m_ModelMesh[i]->m_MeshMat;
// 		}
// 		m_ModelBox.maxPoint = (m_ModelBox.maxPoint- m_ModelBox.centerPoint)*Scale;
// 		m_ModelBox.minPoint = (m_ModelBox.minPoint - m_ModelBox.centerPoint)*Scale;
// 	}
	m_ModelBox.centerPoint = m_ModelBox.minPoint*0.5f + m_ModelBox.maxPoint*0.5f;

//	m_hasSkin = false;
}

void CC3DModel::LoadNode()
{
	if (m_Model->nodes.size() > 0)
	{
		CC3DNode *pNode = new CC3DNode();
		pNode->initModel(m_Model);

		pNode->InitNode();
		
		auto &Scenes = m_Model->scenes;
		if (Scenes.size() > 0)
		{
			auto &NodeIds = Scenes[0].nodes;
			for (int k = 0; k < NodeIds.size(); k++)
			{
				pNode->InitGroupNode(NodeIds[k]);
			}
		}
		m_ModelNode = pNode;

	}
	m_CameraNode.initModel(m_Model);
	m_CameraNode.LoadModelInfo(m_ModelNode);
}
