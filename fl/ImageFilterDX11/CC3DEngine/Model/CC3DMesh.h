#pragma once
#ifndef _H_CC3D_MESH_H_
#define _H_CC3D_MESH_H_

#include "Common/CC3DUtils.h"
#include "CC3DNode.h"
#include"Render/CC3DMeshBuffer.h"
#include <unordered_map>

class CC3DMaterial;
class CC3DMaterialGL;

struct BoundingBox
{
	Vector3 minPoint = Vector3(0,0,0);
	Vector3 maxPoint = Vector3(0, 0, 0);;
	Vector3 centerPoint = Vector3(0, 0, 0);
	
	float radius = 0.0f;
	float GetRadius()
	{
		return sqrt(radius2());
	}

	float radius2()
	{
		return 0.25 * (maxPoint - minPoint).length2();
	}
};
class CC3DMesh : public CCglTFModel
{
public:
	CC3DMesh();
	~CC3DMesh();

	void InitMesh(uint32 meshIndex, uint32 PrimitiveIndex, std::vector<CC3DMaterial*>&ModelTexture,  CC3DNode *m_ModelNode);

	void GenVertWithWeights(std::vector<float>&weight);
	void GenVertWithCoeffs(std::vector<float>& coeffs, std::unordered_map<std::string, int>& BlendShapeName);

	void GenVertWithFaceCoeffs(const std::vector<float>& face_coeffs, std::unordered_map<std::string, int>& face_BlendShapeName);
	//for debug
	void ObjOut(const char * pObjName);

	Vector3 *GetBlendVerts();

	CC3DMeshInfo *m_mesh;
	std::string m_Meshname;
	CC3DMaterial *m_Material;
	bool m_isTransparent = false;

	glm::mat4 m_MeshMat;

	std::vector<Vector3 *>m_pBlendShape;

	std::vector<Vector3>m_FacePinchpBlendShape;

	std::vector<std::string>m_pBlendShapeName;
	//表情驱动blendshap名字对应的ID
	//std::map<std::string, int> m_ExpressBSNameMap;
	//std::map<std::string, int> m_FacePinchBSNameMap;

	BoundingBox m_meshBox;

	int m_nNodeID = -1;
	int m_nSkinID = -1;

	CC3DMeshBuffer *m_pGPUBuffer;

private:
	Vector3 *m_pBlendVerts;

};


#endif // 
