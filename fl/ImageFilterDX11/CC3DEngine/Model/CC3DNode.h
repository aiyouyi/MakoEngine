#pragma once
#ifndef _H_CC3D_NODE_H_
#define _H_CC3D_NODE_H_

#include "Common/CC3DUtils.h"


class CC3DNode : public CCglTFModel
{
public:
	CC3DNode();
	~CC3DNode();

	void InitNode();
	void InitGroupNode(uint32 nodeIndex);
	void CreateModelNodeTree(CC3DNodeInfo *info);
	void UpdateNode();
	void UpdateNodeParent(CC3DNodeInfo *pNode);
	void DFSNodeTree(CC3DNodeInfo * info, glm::mat4 & ParentMatrix);

	std::vector<CC3DNodeInfo *>m_Node;
	std::vector<CC3DNodeInfo *>m_GroupNode;

};

#endif // 

