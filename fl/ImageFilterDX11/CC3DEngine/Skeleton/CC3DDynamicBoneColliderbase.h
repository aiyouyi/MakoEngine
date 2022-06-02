#ifndef _CC_DYNAMIC_BONE_COLLIDER_BASE_
#define _CC_DYNAMIC_BONE_COLLIDER_BASE_

#include "BaseDefine/Vectors.h"
#include "CC3DTransformNode.h"

class DynamicBoneColliderBase
{
	using float3 = Vector3;
public:
	DynamicBoneColliderBase();
	virtual ~DynamicBoneColliderBase();
	virtual void Collide(float3& particlePosition, float particleRadius) = 0;

	enum Direction
	{
		X = 0,
		Y,
		Z
	};

	enum Bound
	{
		Outside = 0,
		Inside
	};

	struct DynamicBoneColliderInfo {
		int _type = 0;
		Direction _dir = Direction::Y;
		Bound _bound = Bound::Outside;
		float3 _center;
		// Ô²ÐÎÅö×²Ìå
		float _radius = 0.5f;
		float _height = 0.0f;
		float _colliderScale = 1.0f;
	};

	virtual bool Init(DynamicBoneColliderInfo info, CC3DTransformNode* pTransform) = 0;

protected:
	Direction m_Direction = Direction::Y;
	Bound m_Bound = Bound::Outside;
	float3 m_vCenter;
	CC3DTransformNode* m_pTransform;
	float m_fRadiusScale = 1.0f;
};

#endif