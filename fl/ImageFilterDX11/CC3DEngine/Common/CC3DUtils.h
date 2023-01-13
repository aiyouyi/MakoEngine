#pragma once
#ifndef _H_CC3D_UTILS_H_
#define _H_CC3D_UTILS_H_

#include "Toolbox/tiny_gltf.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
//glm向量矩阵库
#include "glm/gtc/matrix_transform.hpp"
#include "glm/ext.hpp"
#include <string>
#include <memory>


#define BIT(i) (1 << i)
//一个CCC3DModel拥有的最大骨骼数
#define max_NUM_MODEL_BONE 100

//每一个顶点最多受到*个骨骼影响
#define max_BONES_PER_VEREX 4

static const int MAX_MATRICES = 200;

//每个顶点被骨骼IDs所影响
struct CC3DVertexBoneID
{
	CC3DVertexBoneID()
	{
		memset(BoneIDs, 0, max_BONES_PER_VEREX*sizeof(uint16));
	}
	uint16 BoneIDs[max_BONES_PER_VEREX];
};
//每个顶点骨骼影响的权重
struct CC3DVertexBoneWeight
{
	CC3DVertexBoneWeight()
	{
		memset(BoneWeights, 0, max_BONES_PER_VEREX*sizeof(float));
	}
	float BoneWeights[max_BONES_PER_VEREX];
};



//关键帧动画（Translation、Rotation、Scaling）信息
struct CC3DAnimationKey
{
	CC3DAnimationKey()
	{
		fTime = 0.0f;
		value.x = 0.0f;
		value.y = 0.0f;
		value.z = 0.0f;
		value.w = 0.0f;
	}
	//时间戳
	float fTime;
	//四维变化信息
	glm::fvec4 value;
};

enum CC3DChannelType
{
	TARGET_ROTATE  = 0,
	TARGET_SCALE,
	TARGET_TRANSLATE,
	TARGET_WEIGHT
};
//一个动画中某部分（对应某个Mesh、Bone）的信息
struct CC3DAnimationChannelInfo
{
	CC3DAnimationChannelInfo()
	{
		nNodeID = -1;
		eType = TARGET_ROTATE;
		pInputTime = NULL;
		pOutputAnimateValue = NULL;
		nKeyFrame = 0;
		nOutCount = 0;
	}
	void findKey(float during, int &nKeyL, int &nKeyR, float &alpha)
	{
		if (nKeyFrame < 1)
		{
			return;
		}
		int Index = -1;
		if (during<pInputTime[0])
		{
			nKeyL = 0;
			nKeyR = 0;
			alpha = 1.0;
			return;
		}

		if (during > pInputTime[nKeyFrame - 1])
		{
			nKeyL = -1;
			nKeyR = -1;
			return;
		}

		for (int i=0;i<nKeyFrame-1;i++)
		{
			if (pInputTime[i]<= during&&pInputTime[i+1]>= during)
			{
				nKeyL = i;
				nKeyR = i + 1;
				break;
			}
		}
		if (during >= pInputTime[nKeyFrame - 1])
		{
			nKeyL = nKeyFrame-1;
			nKeyR = nKeyFrame - 1;
		}
		float ticks = pInputTime[nKeyR] - pInputTime[nKeyL];
		if (ticks<0.001)
		{
			alpha = 1.0;
		}
		else
		{
			alpha = (during - pInputTime[nKeyL]) / ticks;
		}
	}
	uint32 nNodeID;
	CC3DChannelType eType;
	float *pInputTime;
	float *pOutputAnimateValue;
	uint32 nKeyFrame;
	uint32 nOutCount;
};


//Model中的一个动画的信息
struct CC3DAnimationInfo
{
	CC3DAnimationInfo()
	{
		nNumChannels = 0;
		pChannels = NULL;
	}
	//持续时间
	float fDuration;
	//每秒帧数
	float fTicksPerSecond;
	//有多少个Channels
	uint32 nNumChannels;
	//帧动画信息
	CC3DAnimationChannelInfo* pChannels;
};


//一个Model的骨骼树节点信息
struct CC3DBoneNodeInfo
{
	CC3DBoneNodeInfo()
	{
		InverseBindMat = glm::mat4();
		FinalTransformation = glm::mat4();
		InitMat = glm::mat4();
		ParentMat = glm::mat4();
		Rotation = Vector4(0, 0, 0, 1);
		Scale = Vector3(1, 1, 1);
		Translate = Vector3(0, 0, 0);
		worldPosition = Vector3(0);
		worldRotation = Vector4(0, 0, 0, 1);
		NodeID = -1;
		pParent = NULL;
	}
	~CC3DBoneNodeInfo()
	{
		pChildren.clear();
	}
	bool IsChildOfMaskBone(const std::string& node_name)
	{
		//如果mask 的骨骼名称为空，则视为使用普通blend,即所有的骨骼动画都生效
		//只有在mask的骨骼名称不为空，且当前骨骼名称不为mask骨骼的子节点，才返回false
		if (node_name.empty())
		{
			return true;
		}
		bool bFind = false;
		CC3DBoneNodeInfo* n = const_cast<CC3DBoneNodeInfo*>(this);
		if (n->bone_name == node_name)
		{
			bFind = true;
			return bFind;
		}
		while (n->pParent)
		{
			n = n->pParent;
			if (n->bone_name == node_name)
			{
				bFind = true;
				break;
			}
		}

		return bFind;
	}
	//节点名
	std::string bone_name;
	//骨骼原有的变化
	glm::mat4 InverseBindMat;
	//动画的每一帧骨骼的最终变化
	glm::mat4 FinalTransformation;

	glm::mat4 InitMat;

	glm::mat4 ParentMat;

	glm::mat4 TPosMat;

	Vector4 Rotation;
	Vector3 Scale;
	Vector3 Translate;

	Vector4 TargetRotation;
	Vector3 TargetScale;
	Vector3 TargetTranslate;

	Vector4 worldRotation;
	Vector3 worldPosition;
	//glm::mat4 worldRotationMat = glm::mat4(1);

	int NodeID;
	//动态骨骼索引，默认为-1，表示没有动态骨骼
	int db_index = -1;
	//当设置某一个节点为动态骨骼附属节点时，此节点的所有子节点都会变成动态变化的节点
	bool bAttachToDynamic = false;

	CC3DBoneNodeInfo* pParent;

	//子节点
	std::vector<CC3DBoneNodeInfo*> pChildren;

};


/****************************************************
*	Mesh结构体
*	每个Mesh包含：
*		1、顶点数目
*		2、顶点位置
*		3、顶点颜色
*		4、顶点法向量
*		5、顶点纹理采样坐标
*		6、Faces数量
*		7、Faces索引
*		8、BoneIDs 影响该顶点的骨骼ID
*		9、BoneWeights 影响该顶点的骨骼权重
*		10、mesh使用的材质索引
*		11、mesh相对model的变化矩阵
****************************************************/
struct CC3DMeshInfo
{
	CC3DMeshInfo()
	{
		nNumVertices = 0;
		nNumFaces = 0;
		pVertices = NULL;
		pNormals = NULL;
		pTextureCoords = NULL;
		pFacesIndex = NULL;
		pBoneIDs = NULL;
		pBoneWeights = NULL;
		pFacesIndex32 = NULL;
		pTangents = NULL;
	}
	//顶点数目
	uint32 nNumVertices;
	//面数目（三角形数目）
	uint32 nNumFaces;
	//顶点位置
	Vector3* pVertices;
	//顶点法向量
	Vector3* pNormals;
	//顶点纹理采样坐标
	Vector2* pTextureCoords;

	//切线
	Vector4* pTangents;
	//faces（三角形）索引
	uint16* pFacesIndex;
	uint32* pFacesIndex32;
	int type;
	//骨骼ID
	CC3DVertexBoneID* pBoneIDs;
	//骨骼权重
	CC3DVertexBoneWeight* pBoneWeights;
};


/****************************************************
*	Node结构体
*	每个Node需要包含：
*		Mesh 变换矩阵
****************************************************/
struct CC3DNodeInfo
{
	CC3DNodeInfo()
	{
		Rotation = Vector4(0, 0, 0, 1);
		Scale = Vector3(1, 1, 1);
		Translate = Vector3(0, 0, 0);
		
		MeshID = -1;
		SkinID = -1;
		NodeID = -1;
		pParent = NULL;
		FinalMeshMat = glm::mat4();
	}
	~CC3DNodeInfo()
	{
		pChildren.clear();
	}

	Vector4 Rotation;
	Vector3 Scale;
	Vector3 Translate;
	
	int MeshID;
	int SkinID;

	int NodeID;
	CC3DNodeInfo *pParent;
	std::vector<CC3DNodeInfo *> pChildren;

	glm::mat4 FinalMeshMat;
};

/****************************************************
*	CC3D工具
****************************************************/
class CC3DUtils
{
public:
	/****************************************************
	*	旋转四元组转化成glm::mat4
	*	@param[vec4InQuaternion]	旋转四元组
	*	@return						旋转矩阵
	****************************************************/
	static glm::mat4 QuaternionToMatrix(const Vector4 vec4InQuaternion);
	
	/****************************************************
	*	旋转四元组插值
	*	@param[vec4Out]		插值后的四元组
	*	@param[vec4Start]	初始四元组
	*	@param[vec4End]		结束四元组
	*	@param[fFactor]		插值系数
	****************************************************/
	static void QuaternionInterpolate(Vector4 & vec4Out, const Vector4 &vec4Start, const Vector4 & vec4End, float fFactor);

	static void Slerp(Vector4 & vec4Out, const Vector4 &vec4Start, const Vector4 & vec4End, float fFactor);

	/****************************************************
	*	旋转四元组归一化
	*	@param[vec4Out]		四元组
	****************************************************/
	static void QuaternionNormalize(Vector4 & vec4Out);

	static std::string GetFileName(const std::string& FilePath);
	static std::string GetFileNameWithoutExtension(const std::string& FilePath);
};

std::string GetFilePathExtension(const std::string &FileName);


class CCglTFModel
{
public:

	CCglTFModel() { m_Model = NULL; }
	~CCglTFModel();

	virtual void initModel(tinygltf::Model *model) { m_Model = model; }

	virtual void *Getdata(int attributeIndex, uint32 &nCount, int &CommpontType);

	tinygltf::Model *m_Model;

	std::vector<uint8 *> m_pData;
};


unsigned char *ccLoadImage(char const *filename, int *x, int *y, int *comp, int req_comp);

float *ccLoadImagef(char const *filename, int *x, int *y, int *comp, int req_comp);

unsigned char *ccLoadImageFromBuffer(unsigned char *pBuffer, int nLen, int *x, int *y, int *comp, int req_comp);

float *ccLoadImagefFromBuffer(unsigned char *pBuffer, int nLen, int *x, int *y, int *comp, int req_comp);

void ccSavePng(const char*pFile, unsigned char *pBuffer, int x, int y, int comp);

void ccSaveJpg(const char*pFile, unsigned char *pBuffer, int x, int y, int comp);

#endif // _H_CC3D_UTILS_H_
