
#include "CC3DCameraNode.h"


CC3DCameraNode::CC3DCameraNode()
{
	m_ModelNode = NULL;
}

CC3DCameraNode::~CC3DCameraNode()
{
}

void CC3DCameraNode::LoadModelInfo(CC3DNode* pNode)
{
	m_ModelNode = pNode;
	auto &pNodes = pNode->m_Node;
	auto &nodes = m_Model->nodes;
	for (int i=0;i<pNodes.size();i++)
	{
		int NodeID = pNodes[i]->NodeID;
		if (nodes[NodeID].camera>-1)
		{
			m_CameraNode.push_back(pNodes[i]);
		}
	}
	m_Project.resize(m_CameraNode.size());
	for (int i=0;i<m_Model->cameras.size();i++)
	{
		auto &perspective = m_Model->cameras[i].perspective;
		m_Project[i].Rest(perspective.yfov*180.f/CC_PI, perspective.aspectRatio, perspective.znear, perspective.zfar);
	}
}

void CC3DCameraNode::UpdateNode()
{
	for (int i=0;i<m_CameraNode.size();i++)
	{
		m_ModelNode->UpdateNodeParent(m_CameraNode[i]);
	}
}

void CC3DCameraNode::UpdateNodeIndex(int nIndex)
{
	if (nIndex>-1&&nIndex< m_CameraNode.size())
	{
		m_ModelNode->UpdateNodeParent(m_CameraNode[nIndex]);
	}

}

void CC3DCameraNode::update()
{

}
