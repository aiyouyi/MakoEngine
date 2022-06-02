
#include "CC3DMesh.h"
#include <fstream>
CC3DMesh::CC3DMesh()
{
	m_mesh = NULL;
	m_Material = NULL;
	m_pBlendVerts = NULL;
	m_pGPUBuffer = NULL;
}

CC3DMesh::~CC3DMesh()
{
	SAFE_DELETE(m_mesh);
	SAFE_DELETE(m_pBlendVerts);
	SAFE_DELETE(m_pGPUBuffer);
}



void CC3DMesh::ObjOut(const char * pObjName)
{
	std::ofstream out(pObjName);


	for (int i = 0; i < m_mesh->nNumVertices; i++)
	{
		out << "v " << m_mesh->pVertices[i].x << " " << m_mesh->pVertices[i].y << " " << m_mesh->pVertices[i].z << std::endl;
	}


	for (int i = 0; i < m_mesh->nNumFaces; i++)
	{
		int i3 = i * 3;
		out << "f " << m_mesh->pFacesIndex[i3] + 1 << " " << m_mesh->pFacesIndex[i3 + 1] + 1 << " " << m_mesh->pFacesIndex[i3 + 2] + 1 << std::endl;
	}

	out.close();

}


void CC3DMesh::InitMesh(uint32 meshIndex, uint32 PrimitiveIndex, std::vector<CC3DMaterial*>&ModelMatrial, CC3DNode *m_ModelNode)
{
	if (m_mesh == NULL)
	{
		m_mesh = new CC3DMeshInfo();
		m_pGPUBuffer = new CC3DMeshBuffer;
	}
	auto &meshPrimitive = m_Model->meshes[meshIndex].primitives[PrimitiveIndex];
	m_Meshname = m_Model->meshes[meshIndex].name;

	auto Index = Getdata(meshPrimitive.indices, m_mesh->nNumFaces, m_mesh->type);
	m_mesh->nNumFaces /= 3;
	if (m_mesh->type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
	{
		m_mesh->pFacesIndex = (uint16*)Index;
	}
	else if (m_mesh->type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
	{
		uint16 *pData = new uint16[m_mesh->nNumFaces * 3];
		uint8 *pSrc = (uint8 *)Index;
		for (int i = 0; i < m_mesh->nNumFaces * 3; i++)
		{
			pData[i] = pSrc[i];
		}
		m_mesh->pFacesIndex = pData;
		m_pData.push_back((uint8*)pData);
	}
	else
	{
		m_mesh->pFacesIndex32 = (uint32*)Index;
	}

	for (const auto &attribute : meshPrimitive.attributes) {

		int type = 0;
		if (attribute.first == "POSITION")
		{
			m_mesh->pVertices = (Vector3*)Getdata(attribute.second, m_mesh->nNumVertices, type);
			auto &minVaue = m_Model->accessors[attribute.second].minValues;
			auto &maxVaue = m_Model->accessors[attribute.second].maxValues;
			if (minVaue.size() == 3 && maxVaue.size() == 3)
			{
				m_meshBox.minPoint.x = minVaue[0];
				m_meshBox.minPoint.y = minVaue[1];
				m_meshBox.minPoint.z = minVaue[2];
				m_meshBox.maxPoint.x = maxVaue[0];
				m_meshBox.maxPoint.y = maxVaue[1];
				m_meshBox.maxPoint.z = maxVaue[2];
				m_meshBox.centerPoint = m_meshBox.minPoint*0.5f + m_meshBox.maxPoint*0.5f;
			}

		}
		else if(attribute.first == "NORMAL")
		{
			m_mesh->pNormals = (Vector3*)Getdata(attribute.second, m_mesh->nNumVertices, type);
		}
		else if (attribute.first == "TEXCOORD_0")
		{
			m_mesh->pTextureCoords = (Vector2*)Getdata(attribute.second, m_mesh->nNumVertices, type);
		}
		else if (attribute.first == "TANGENT")
		{
			m_mesh->pTangents = (Vector4*)Getdata(attribute.second, m_mesh->nNumVertices, type);
		}
		else if (attribute.first == "JOINTS_0")
		{
			m_mesh->pBoneIDs = (CC3DVertexBoneID*)Getdata(attribute.second, m_mesh->nNumVertices, type);
		}
		else if (attribute.first == "WEIGHTS_0")
		{
			m_mesh->pBoneWeights = (CC3DVertexBoneWeight*)Getdata(attribute.second, m_mesh->nNumVertices, type);
		}
	}
	for (int i=0;i< meshPrimitive.targets.size();i++)
	{
		auto &target = meshPrimitive.targets[i];
		int type = 0;
		for (const auto &attribute : target) {
			if (attribute.first == "POSITION")
			{
				Vector3 *pVertices = (Vector3*)Getdata(attribute.second, m_mesh->nNumVertices, type);
				m_pBlendShape.push_back(pVertices);
			}

		}
	}
	if (m_pBlendShape.size()>0)
	{
		auto &BlendShapeName = m_Model->meshes[meshIndex].extras.Get("targetNames");
		m_pBlendShapeName.resize(m_pBlendShape.size());
		for (int i =0;i<m_pBlendShape.size();i++)
		{
			std::string name = BlendShapeName.Get(i).Get<std::string>();
			std::string subName = name.substr(name.find('.') + 1);
			m_pBlendShapeName[i] = subName;
			//m_pBlendShapeName[i] = BlendShapeName.Get(i).Get<std::string>();
		}
		
		
	}

	int nMaterial = meshPrimitive.material >= 0 ? meshPrimitive.material: 0;
	m_Material = ModelMatrial[nMaterial];
	if (m_Material->alphaMode != "OPAQUE")
	{
		m_isTransparent = true;
	}
	


	auto &Nodes = m_Model->nodes;


	m_MeshMat = glm::mat4();
	for (int i=0;i< Nodes.size();i++)
	{
		if (Nodes[i].mesh == meshIndex)
		{
			m_nNodeID = i;
			m_nSkinID = Nodes[i].skin;

			m_MeshMat = m_ModelNode->m_Node[m_nNodeID]->FinalMeshMat;
			break;
		}

	}

	m_pGPUBuffer->InitMesh(m_mesh);
	//ObjOut("out.obj");
}

void CC3DMesh::GenVertWithWeights(std::vector<float>& weight)
{
	if (weight.size()!= m_pBlendShape.size()||m_pBlendShape.size()==0)
	{
		return;
	}
	if (m_pBlendVerts == NULL)
	{
		m_pBlendVerts = new Vector3[m_mesh->nNumVertices];
	}

	if (m_FacePinchpBlendShape.size() > 0)
	{
		memcpy(m_pBlendVerts, m_FacePinchpBlendShape.data(), sizeof(Vector3)*m_mesh->nNumVertices);
	}
	else
	{
		memcpy(m_pBlendVerts, m_mesh->pVertices, sizeof(Vector3)*m_mesh->nNumVertices);
	}

	for (int i=0;i<weight.size();i++)
	{
		float w = weight[i];
		Vector3 *pBlendShape = m_pBlendShape[i];
		if (w < 0.001&&w>-0.001)
		{
			continue;
		}
		for (int j = 0; j < m_mesh->nNumVertices; j++)
		{
			m_pBlendVerts[j] += w * pBlendShape[j];
		}
	}
	m_pGPUBuffer->UpdateVert(m_pBlendVerts, m_mesh->nNumVertices);
}

void CC3DMesh::GenVertWithCoeffs(std::vector<float>& coeffs, std::unordered_map<std::string, int>& BlendShapeName)
{
	if (m_pBlendShape.size() == 0)
	{
		return;
	}
	if (m_pBlendShapeName.size() != m_pBlendShape.size())
	{
		return;
	}

// 	//表情驱动map，方便查找
// 	if (BlendShapeName.empty())
// 	{
// 		int index = 0;
// 		for (; index < BlendShapeName.size(); index++)
// 		{
// 			m_ExpressBSNameMap[BlendShapeName[index]] = index;
// 		}
// 	}



	if (m_pBlendVerts == NULL)
	{
		m_pBlendVerts = new Vector3[m_mesh->nNumVertices];
	}

	if (m_FacePinchpBlendShape.size()>0)
	{
		memcpy(m_pBlendVerts, m_FacePinchpBlendShape.data(), sizeof(Vector3)*m_mesh->nNumVertices);
	}
	else
	{
		memcpy(m_pBlendVerts, m_mesh->pVertices, sizeof(Vector3)*m_mesh->nNumVertices);
	}

	//表情驱动bs
	for (int i = 0; i < m_pBlendShapeName.size(); i++)
	{
		std::string name = m_pBlendShapeName[i];
		auto itr = BlendShapeName.find(name);
		if (itr != BlendShapeName.end())
		{
			int coeffIndex = itr->second;
			float w = coeffs[coeffIndex];
			if (w < 0.001&&w>-0.001)
			{
				continue;
			}
			Vector3 *pBlendShape = m_pBlendShape[i];
			for (int j = 0; j < m_mesh->nNumVertices; j++)
			{
				m_pBlendVerts[j] += w * pBlendShape[j];
			}
		}
	}


	m_pGPUBuffer->UpdateVert(m_pBlendVerts, m_mesh->nNumVertices);
}

void CC3DMesh::GenVertWithFaceCoeffs(const std::vector<float>& face_coeffs, std::unordered_map<std::string, int>& face_BlendShapeName)
{
	if (m_pBlendShape.size() == 0)
	{
		return;
	}
// 	//捏脸map，方便后续查找
// 	if (m_FacePinchBSNameMap.empty())
// 	{
// 		for (int i = 0; i < face_BlendShapeName.size(); i++)
// 		{
// 			m_FacePinchBSNameMap[face_BlendShapeName[i]] = i;
// 		}
// 	}
	if (m_FacePinchpBlendShape.size() != m_mesh->nNumVertices)
	{
		m_FacePinchpBlendShape.resize(m_mesh->nNumVertices);
	}
	memcpy(m_FacePinchpBlendShape.data(), m_mesh->pVertices, sizeof(Vector3)*m_mesh->nNumVertices);

	//捏脸bs
	for (int i = 0; i < m_pBlendShapeName.size(); i++)
	{
		std::string name = m_pBlendShapeName[i];
		auto itr = face_BlendShapeName.find(name);
		if (itr != face_BlendShapeName.end())
		{
			int coeffIndex = itr->second;
			float w = face_coeffs[coeffIndex];
			if (w < 0.001&&w>-0.001)
			{
				continue;
			}
			Vector3 *pBlendShape = m_pBlendShape[i];
			for (int j = 0; j < m_mesh->nNumVertices; j++)
			{
				m_FacePinchpBlendShape[j] += w * pBlendShape[j];
			}
		}
	}
	m_pGPUBuffer->UpdateVert(m_FacePinchpBlendShape.data(), m_mesh->nNumVertices);

}



Vector3 * CC3DMesh::GetBlendVerts()
{
	//return m_mesh->pVertices;
	if (m_pBlendVerts == NULL)
	{
		return m_mesh->pVertices;
	}
	return m_pBlendVerts;
}
