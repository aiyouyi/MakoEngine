#include "Skeleton/CC3DDynamicBoneCollider.h"

DynamicBoneCollider::DynamicBoneCollider()
{
	m_fRadius = 0.5f;
	m_fHeight = 0.0f;
}

DynamicBoneCollider::~DynamicBoneCollider()
{
	m_pTransform = nullptr;
}

void DynamicBoneCollider::Collide(float3& particlePosition, float particleRadius)
{
	float radius = m_fRadius * m_fRadiusScale;
	float h = m_fHeight * 0.5f - m_fRadius;
	if (h <= 0.0)
	{
		float3 transformResult;
		//m_pTransform->transformPoint(m_vCenter, &transformResult);
		glm::vec3 resultP = m_pTransform->glm_localToWorld * glm::vec4(m_vCenter.x, m_vCenter.y, m_vCenter.z, 1.0f);
		transformResult = float3(resultP.x, resultP.y, resultP.z);
		if (m_Bound == Bound::Outside) {
			OutsideSphere(particlePosition, particleRadius, transformResult, radius);
		}
		else {
			InsideSphere(particlePosition, particleRadius, transformResult, radius);
		}
	}
	else
	{
		float3 c0 = m_vCenter;
		float3 c1 = m_vCenter;

		switch (m_Direction)
		{
		case Direction::X:
			c0.x -= h;
			c1.x += h;
			break;
		case Direction::Y:
			c0.y -= h;
			c1.y += h;
			break;
		case Direction::Z:
			c0.z -= h;
			c1.z += h;
			break;
		}
		float3 c0Result;
		float3 c1Result;
		//m_pTransform->transformPoint(c0, &c0Result);
		//m_pTransform->transformPoint(c1, &c1Result);

		glm::vec3 p1;
		glm::vec3 p2;
		p1 = m_pTransform->glm_localToWorld * glm::vec4(c0.x, c0.y, c0.z, 1.0f);
		c0Result = float3(p1.x, p1.y, p1.z);

		p2 = m_pTransform->glm_localToWorld * glm::vec4(c1.x, c1.y, c1.z, 1.0f);
		c1Result = float3(p2.x, p2.y, p2.z);

		if (m_Bound == Bound::Outside) {
			OutsideCapsule(particlePosition, particleRadius, c0Result, c1Result, radius);
		}
		else {
			InsideCapsule(particlePosition, particleRadius, c0Result, c1Result, radius);
		}
	}

}

bool DynamicBoneCollider::Init(DynamicBoneColliderInfo info, CC3DTransformNode* pTransform)
{
	// 也是同样不管理这个内存 只是获取而已
	m_pTransform = pTransform;
	m_vCenter = info._center;
	m_Direction = info._dir;
	m_Bound = info._bound;
	m_fRadius = info._radius;
	m_fHeight = info._height;
	m_fRadiusScale = info._colliderScale;
	return true;
}

void DynamicBoneCollider::OutsideSphere(float3& particlePosition, float particleRadius, float3 sphereCenter, float sphereRadius)
{
	float r = sphereRadius + particleRadius;
	float r2 = r * r;
	float3 d = particlePosition - sphereCenter;
	float len2 = d.length2();
	// if is inside sphere, project onto sphere surface
	if (len2 > 0 && len2 < r2)
	{
		float len = sqrt(len2);
		particlePosition = sphereCenter + d * (r / len);
	}
}

void DynamicBoneCollider::InsideSphere(float3& particlePosition, float particleRadius, float3 sphereCenter, float sphereRadius)
{
	float r = sphereRadius - particleRadius;
	float r2 = r * r;
	float3 d = particlePosition - sphereCenter;
	float len2 = d.length2();
	// if is outside sphere, project onto sphere surface
	if (len2 > r2)
	{
		float len = sqrt(len2);
		particlePosition = sphereCenter + d * (r / len);
	}
}

void DynamicBoneCollider::OutsideCapsule(float3& particlePosition, float particleRadius, float3 capsuleP0, float3 capsuleP1, float capsuleRadius)
{
	float r = capsuleRadius + particleRadius;
	float r2 = r * r;
	float3 dir = capsuleP1 - capsuleP0;
	float3 d = particlePosition - capsuleP0;
	//float t = gameplay::Vector3::dot(d, dir);
	float t = d.dot(dir);

	if (t <= 0)
	{
		// check sphere1
		float len2 = d.length2();
		if (len2 > 0 && len2 < r2)
		{
			float len = sqrt(len2);
			particlePosition = capsuleP0 + d * (r / len);
		}
	}
	else
	{
		float dl = dir.length2();
		if (t >= dl)
		{
			// check sphere2
			d = particlePosition - capsuleP1;
			float len2 = d.length2();
			if (len2 > 0 && len2 < r2)
			{
				float len = sqrt(len2);
				particlePosition = capsuleP1 + d * (r / len);
			}
		}
		else if (dl > 0)
		{
			// check cylinder
			t /= dl;
			d -= dir * t;
			float len2 = d.length2();
			if (len2 > 0 && len2 < r2)
			{
				float len = sqrt(len2);
				particlePosition += d * ((r - len) / len);
			}
		}
	}
}

void DynamicBoneCollider::InsideCapsule(float3& particlePosition, float particleRadius, float3 capsuleP0, float3 capsuleP1, float capsuleRadius)
{
	float r = capsuleRadius - particleRadius;
	float r2 = r * r;
	float3 dir = capsuleP1 - capsuleP0;
	float3 d = particlePosition - capsuleP0;
	//float t = gameplay::Vector3::dot(d, dir);
	float t = d.dot(dir);

	if (t <= 0)
	{
		// check sphere1
		float len2 = d.length2();
		if (len2 > r2)
		{
			float len = sqrt(len2);
			particlePosition = capsuleP0 + d * (r / len);
		}
	}
	else
	{
		float dl = dir.length2();
		if (t >= dl)
		{
			// check sphere2
			d = particlePosition - capsuleP1;
			float len2 = d.length2();
			if (len2 > r2)
			{
				float len = sqrt(len2);
				particlePosition = capsuleP1 + d * (r / len);
			}
		}
		else if (dl > 0)
		{
			// check cylinder
			t /= dl;
			d -= dir * t;
			float len2 = d.length2();
			if (len2 > r2)
			{
				float len = sqrt(len2);
				particlePosition += d * ((r - len) / len);
			}
		}
	}
}