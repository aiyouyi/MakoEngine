#ifndef _CC_DYNAMIC_BONE_H_
#define _CC_DYNAMIC_BONE_H_

#include <vector>
#include <string>
#include <map>
#include "BaseDefine/Vectors.h"
#include "Toolbox/CC3DMatrix.h"
#include "Toolbox/CC3DQuaternion.h"
#include "CC3DTransformNode.h"

typedef Vector3 float3;
typedef CC3DMath::CC3DMatrix mat4f;
typedef CC3DMath::CC3DQuaternion quaternion4f;

enum DynamicBoneUpdateMode
{
	DynamicBone_UpdateMode_Normal = 0,
	DynamicBone_UpdateMode_AnimatePhysics = 1,
	DynamicBone_UpdateMode_UnscaledTime = 2
};

enum DynamicBoneFreezeAxis
{
	DynamicBone_FreezeAxis_None = 0,
	DynamicBone_FreezeAxis_X = 1,
	DynamicBone_FreezeAxis_Y = 2,
	DynamicBone_FreezeAxis_Z = 3
};

using UnityNode = CC3DTransformNode;

struct DynamicBoneInfo
{
	DynamicBoneUpdateMode _mode;
	DynamicBoneFreezeAxis _freezeAxis;
	// ϵ��
	// TODO:���µĶ�����ʵ����һ������ϵ��
	float _fDamping;		//����
	float _fElasticity;		//����
	float _fStiffness;		//����
	float _fInert;			//����
	float _fRadius;			//�뾶
	float _fEndLength;		//
	float3 _endOffset;		//
	//����
	float3 _gravity = float3(0.0f);
	float3 _force = float3(0.0f);

	// vector<glm::vec4> m_Exclusions = null;
	// ����ȥû��Ҫ �ȱ���
	bool _bDistantDisable;
	float _fDistanceToObject;

	// bin�ļ���path
	std::string _binPath;

	float _radiusScale = 1.0f;

	UnityNode* _pRootTransform;
};

class DynamicBoneColliderBase;

class DynamicBone
{
public:
	// ��¼ÿ������(�ڵ�)����Ϣ
	struct DynamicParticle 
	{
		float3 _initLocalPosition;
		quaternion4f _initLocalRotation;
		int _nParentIndex;
		float _fDamping;
		float _fElasticity;
		float _fStiffness;
		float _fInert;
		float _fRadius;
		float _fBoneLength;

		float3 _position;
		float3 _prevPosition;
		float3 _endOffset;

		UnityNode* _gpTransform;
	};
public:
	DynamicBone();
	~DynamicBone();

	// ȥ�����ڲ����нڵ����Ϣ
	void Update(float delta); //���½ڵ㣬ÿһ֡������и��£����¶�̬������λ��
	bool Init(DynamicBoneInfo& boneInfo); //3.���ý��������õĸ��ԡ����ԵȲ���
	bool InitWithBinPath(std::string path);
	void SetObjectPrevPosition(float3 prevPosition);
	void InitParticle(UnityNode* rootTransform);  //1.��ʼ��ÿ���ڵ���Ϣ
	void AddCollider(DynamicBoneColliderBase* pCollider);
	void InitTransform(); //2.��ʼ����̬�����ľֲ�λ�ú;ֲ���ת
	std::map<std::string, UnityNode*> getNodeMap();

	void Release();
	std::vector<DynamicParticle*>& GetDynamicParticleVector();
	//void WriteConfig(const char* pFileName);

	void UpdateParticleParam(DynamicBoneInfo& m_info);
	std::string GetID() { return db_name; }

private:
	void AppendParticles(UnityNode* dbTransform, int nParentIndex, float boneLength);
	void UpdateParticleParam();

	void UpdateParticle1(float timevar);
	void UpdateParticle2( float timevar );

	void SkipUpdateParticles();

	void ApplyParticlesToTransforms();

	// �����ȥ���� һ����Ǹ�����̬������
	UnityNode* m_pGPRoot;
	DynamicBoneUpdateMode m_UpdateMode = DynamicBone_UpdateMode_Normal;
	DynamicBoneFreezeAxis m_FreezeAxis = DynamicBone_FreezeAxis_None;

	bool _bIsRefObject;
	mat4f _refObjectTransform;
	// TODO:
	//std::vector<Collider>  m_Colliders;

	float3 m_Gravity;
	float3 m_LocalGravity;
	float3 m_ObjectMove;
	float3 m_ObjectPrevPosition;
	float3 m_EndOffset;

	float m_fBoneTotalLength;
	float m_fObjectScale;
	float m_fTime;
	float m_fWeight;
	float m_fEndLength;
	// ����Ĭ��30֡
	float m_fUpdateRate;

	bool m_DistantDisabled = false;

	std::vector<DynamicParticle*> m_vecDynameicParticle;
	std::vector<DynamicBoneColliderBase*> m_vecDynamicCollider;

	DynamicBoneInfo m_Info;

	std::string db_name;
};

#endif