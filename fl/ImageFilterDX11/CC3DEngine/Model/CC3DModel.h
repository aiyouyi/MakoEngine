#pragma once
#ifndef _H_CC3D_MODEL_H_
#define _H_CC3D_MODEL_H_
#include "Common/CC3DUtils.h"
#include "Material/CC3DMaterial.h"
#include "Model/CC3DMesh.h"
#include "Model/CC3DNode.h"
#include "Model/CC3DCameraNode.h"
#include "Toolbox/Render/DynamicRHI.h"
#include <vector>

class CC3DModel : public CCglTFModel
{
public:
	CC3DModel();
	virtual ~CC3DModel();
	void LoadModelInfo();
	void ReleaseModel();
	void UpdateNode();
	void update();

	void TransformBoundingBox(glm::mat4 &TransformMat);
	std::vector<CC3DMesh*> m_ModelMesh;
	std::vector<std::shared_ptr<CC3DTextureRHI>> m_ModelTexture;
	std::vector<CC3DMaterial*> m_ModelMaterial;

	CC3DCameraNode m_CameraNode;

	//平移矩阵，模型加载之后将模型移动至场景中心的矩阵
	glm::mat4 transform_matrix;

	bool m_hasSkin = false;


	CC3DNode* m_ModelNode;
	BoundingBox m_ModelBox;
private:


	void LoadTexture();
	void LoadMaterial();
	void LoadMesh();
	void LoadNode();

};


#endif // _H_CC3D_MODEL_H_
