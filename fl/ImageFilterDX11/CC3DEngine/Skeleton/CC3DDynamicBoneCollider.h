#ifndef _CC_DYNAMIC_BONE_COLLIDER_H_
#define _CC_DYNAMIC_BONE_COLLIDER_H_

#include "BaseDefine/Vectors.h"
#include "Skeleton/CC3DDynamicBoneColliderbase.h"
#include "CC3DTransformNode.h"

class DynamicBoneCollider : public DynamicBoneColliderBase 
{
public:
	DynamicBoneCollider();
	virtual ~DynamicBoneCollider();

	using float3 = Vector3;

	virtual void Collide(float3& particlePosition, float particleRadius);
	/*
	 @param: pTransform 是指需要被碰撞的物体的transform接口，比如有一个箱子需要参与碰撞，箱子会有一个translatation 偏移值，
	  那么这个pTransform就是这个物体的transform矩阵，后面计算用到的m_vCenter是物体局部坐标中心
	*/
	virtual bool Init(DynamicBoneColliderInfo info, CC3DTransformNode* pTransform);

protected:

	void OutsideSphere(float3& particlePosition, float particleRadius, float3 sphereCenter, float sphereRadius);
	void InsideSphere(float3& particlePosition, float particleRadius, float3 sphereCenter, float sphereRadius);
	void OutsideCapsule(float3& particlePosition, float particleRadius, float3 capsuleP0, float3 capsuleP1, float capsuleRadius);
	void InsideCapsule(float3& particlePosition, float particleRadius, float3 capsuleP0, float3 capsuleP1, float capsuleRadius);

	float m_fRadius;
	float m_fHeight;

};

#endif