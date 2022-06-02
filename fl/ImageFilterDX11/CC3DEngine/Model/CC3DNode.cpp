#include "CC3DNode.h"

CC3DNode::CC3DNode()
{

}

CC3DNode::~CC3DNode()
{
	for (int i = 0; i < m_Node.size(); i++)
	{
		SAFE_DELETE(m_Node[i]);
	}
	m_Node.clear();
}

void CC3DNode::InitNode()
{
	auto &nodes = m_Model->nodes;

	for (int i = 0; i < nodes.size(); i++)
	{
		auto &Node = nodes[i];

		CC3DNodeInfo * pInfo = new CC3DNodeInfo();
		pInfo->MeshID = Node.mesh;
		pInfo->SkinID = Node.skin;
		pInfo->NodeID = i;

		glm::mat4 m_NodeMat = glm::mat4();

		if (Node.matrix.size() == 16)
		{
			float mat16[16];
			for (int m = 0; m < 16; m++)
			{
				mat16[m] = Node.matrix[m];
			}
			glm::mat4 *pMat2 = (glm::mat4 *)mat16;
			m_NodeMat = pMat2[0];
		}


		if (Node.translation.size() == 3)
		{
			m_NodeMat = glm::translate(m_NodeMat, glm::vec3(Node.translation[0], Node.translation[1], Node.translation[2]));
			pInfo->Translate = Vector3(Node.translation[0], Node.translation[1], Node.translation[2]);
		}

		if (Node.scale.size() == 3)
		{
			m_NodeMat = glm::scale(m_NodeMat, glm::vec3(Node.scale[0], Node.scale[1], Node.scale[2]));
			pInfo->Scale = Vector3(Node.scale[0], Node.scale[1], Node.scale[2]);
		}

		if (Node.rotation.size() == 4)
		{
			m_NodeMat *= CC3DUtils::QuaternionToMatrix(Vector4(Node.rotation[0], Node.rotation[1], Node.rotation[2], Node.rotation[3]));
			pInfo->Rotation = Vector4(Node.rotation[0], Node.rotation[1], Node.rotation[2], Node.rotation[3]);
		}

		pInfo->FinalMeshMat = m_NodeMat;

		m_Node.push_back(pInfo);

	}

}

void CC3DNode::CreateModelNodeTree(CC3DNodeInfo *info)
{
	if (info == NULL)
	{
		return;
	}

	int NodeID = info->NodeID;
	auto &child = m_Model->nodes[NodeID].children;
	for (int i = 0; i < child.size(); i++)
	{
		int nodeId = child[i];
		CC3DNodeInfo *pChild = m_Node[nodeId];
		pChild->pParent = info;
		info->pChildren.push_back(pChild);

		CreateModelNodeTree(pChild);
	}

}

void CC3DNode::InitGroupNode(uint32 nodeIndex)
{
	if (nodeIndex >= 0 && nodeIndex < m_Model->nodes.size())
	{
		CC3DNodeInfo * pGroupNode = m_Node[nodeIndex];

		auto &child = m_Model->nodes[nodeIndex].children;
		if (child.size() > 0)
		{
			CreateModelNodeTree(pGroupNode);
			
			glm::mat4 Identity = glm::mat4();

			DFSNodeTree(pGroupNode, Identity);

			m_GroupNode.push_back(pGroupNode);
		}
		

	}
}

void CC3DNode::DFSNodeTree(CC3DNodeInfo * info, glm::mat4 & ParentMatrix)
{
	glm::mat4 NodeTransformation = glm::mat4();

	NodeTransformation = ParentMatrix * info->FinalMeshMat;

	info->FinalMeshMat = NodeTransformation;

	for (int i = 0; i < info->pChildren.size(); i++)
	{
		DFSNodeTree(info->pChildren[i], NodeTransformation);
	}
}

void CC3DNode::UpdateNode()
{
	for (int i = 0; i < m_GroupNode.size(); i++)
	{
		glm::mat4 Identity = glm::mat4();

		DFSNodeTree(m_GroupNode[i], Identity);
	}

}

void CC3DNode::UpdateNodeParent(CC3DNodeInfo * pNode)
{
	if (pNode == NULL)
	{
		return;
	}
	glm::mat4 NodeTransformation;
	glm::mat4 mat4Scaling = glm::scale(NodeTransformation, glm::vec3(pNode->Scale.x, pNode->Scale.y, pNode->Scale.z));

	glm::mat4 mat4Rotation = CC3DUtils::QuaternionToMatrix(pNode->Rotation);

	glm::mat4 mat4Translation = glm::translate(NodeTransformation, glm::vec3(pNode->Translate.x, pNode->Translate.y, pNode->Translate.z));

	NodeTransformation =  mat4Translation *mat4Rotation* mat4Scaling;
	if (pNode->pParent !=NULL)
	{
		UpdateNodeParent(pNode->pParent);
		pNode->FinalMeshMat = pNode->pParent->FinalMeshMat*NodeTransformation;
	}
	else
	{
		pNode->FinalMeshMat = NodeTransformation;
	}


}
