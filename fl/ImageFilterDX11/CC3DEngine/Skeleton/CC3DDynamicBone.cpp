#include "CC3DDynamicBone.h"
#include <algorithm>
#include "CC3DDynamicBoneColliderbase.h"
#include "glm/detail/type_vec4.hpp"
#include "Common/CC3DEnvironmentConfig.h"
#include "BaseDefine/Define.h"

DynamicBone::DynamicBone()
{
	m_fUpdateRate = 30.0f;
	m_fBoneTotalLength = 0.0f;
	m_fObjectScale = 0.0f;
	m_fTime = 0.0f;
	m_fWeight = 0.0f;
	m_fEndLength = 0.0f;
}

DynamicBone::~DynamicBone()
{
	this->Release();
}

void DynamicBone::SetObjectPrevPosition(float3 prevPosition)
{
	m_ObjectPrevPosition = prevPosition;
}

void DynamicBone::InitTransform()
{
	for (int i = 0; i < m_vecDynameicParticle.size(); ++i)
	{
		DynamicParticle* p = m_vecDynameicParticle[i];
		if (p->_gpTransform != nullptr)
		{
			p->_gpTransform->localPosition = p->_initLocalPosition;
			p->_gpTransform->localRotation = p->_initLocalRotation;
		}
	}
}

std::map<std::string, UnityNode*> DynamicBone::getNodeMap()
{
	std::map<std::string, UnityNode*> map;
	for (int i = 0; i < m_vecDynameicParticle.size(); i++) {
		map.insert(std::make_pair(m_vecDynameicParticle[i]->_gpTransform->getId(), m_vecDynameicParticle[i]->_gpTransform));
	}
	return map;
};

std::vector<DynamicBone::DynamicParticle*>& DynamicBone::GetDynamicParticleVector()
{
	return m_vecDynameicParticle;
}

//void DynamicBone::WriteConfig(const char* pFileName)
//{
//	File file(pFileName);
//	file.OpenPrintf(true);
//	file.Write((int)m_vecDynameicParticle.size());
//	for (int i = 0; i < m_vecDynameicParticle.size(); i++) {
//		DynamicParticle* p = m_vecDynameicParticle[i];
//		std::string nodeName = p->_gpTransform->getId();
//		file.Write((int)nodeName.size());
//		file.Write(const_cast<char*>(nodeName.c_str()), (int)nodeName.size());
//		file.Write(p->_fDamping);
//		file.Write(p->_fElasticity);
//		file.Write(p->_fStiffness);
//		file.Write(p->_fInert);
//		file.Write(p->_fRadius);
//	}
//	file.Close();
//}


void DynamicBone::Update(float delta)
{
	if (m_pGPRoot == nullptr)
		return;

	m_fObjectScale = 1.0f;
	// 一般上应该不会缩放
//        m_ObjectScale = Mathf.Abs(transform.lossyScale.x);
	//m_ObjectMove = transform->getTranslationWorld() - m_ObjectPrevPosition;
	//m_ObjectPrevPosition = transform->getTranslationWorld();
	m_ObjectMove.set(0.0f, 0.0f, 0.0f);
	//        m_ObjectPrevPosition.set(0.0f,0.0f,0.0f);


	int loop = 1;
	if (m_fUpdateRate > 0)
	{
		float dt = 1.0f / m_fUpdateRate;
		m_fTime += delta;
		loop = 0;

		while (m_fTime >= dt)
		{
			m_fTime -= dt;
			if (++loop >= 3)
			{
				m_fTime = 0;
				break;
			}
		}
	}

	float timevar = CC3DEnvironmentConfig::getInstance()->deltaTime * 60.0f;

	if (loop > 0)
	{
		for (int i = 0; i < loop; ++i)
		{
			UpdateParticle1(timevar);
			UpdateParticle2(timevar);
			m_ObjectMove.set(0.0f, 0.0f, 0.0f);
		}
	}
	else
	{
		SkipUpdateParticles();
	}

	ApplyParticlesToTransforms();

}

void DynamicBone::Release()
{
	for (auto particle : m_vecDynameicParticle) {
		SAFE_DELETE(particle);
	}
	for (auto collider : m_vecDynamicCollider) {
		SAFE_DELETE(collider);
	}
}

bool DynamicBone::Init(DynamicBoneInfo& info)
{
	m_Info._fDamping = info._fDamping;
	m_Info._fElasticity = info._fElasticity;
	m_Info._fStiffness = info._fStiffness;
	m_Info._fInert = info._fInert;
	m_Info._gravity = info._gravity;
	m_Info._fEndLength = info._fEndLength;
	m_Info._fRadius = info._fRadius;
	m_Info._force = info._force;
	m_Info._endOffset = info._endOffset;

	m_Info._mode = info._mode;
	m_Info._freezeAxis = info._freezeAxis;
	m_Info._radiusScale = info._radiusScale;

	m_Info._binPath = info._binPath;

	m_EndOffset = info._endOffset;

	m_Gravity = m_Info._gravity;

	return true;
}

bool DynamicBone::InitWithBinPath(std::string path)
{
	m_Info._binPath = path;
	return true;
}

void DynamicBone::InitParticle(UnityNode *rootTransform) 
{
	m_vecDynameicParticle.clear();

	m_pGPRoot = rootTransform;
	AppendParticles(m_pGPRoot, -1, 0.0f);
	UpdateParticleParam();
}

void DynamicBone::AddCollider(DynamicBoneColliderBase* pCollider)
{
	m_vecDynamicCollider.push_back(pCollider);
}

void DynamicBone::AppendParticles(UnityNode* dbTransform, int parentIndex, float boneLength)
{
	DynamicParticle* particle = new DynamicParticle();
	particle->_gpTransform = dbTransform;
	particle->_nParentIndex = parentIndex;
	if (dbTransform != nullptr)
	{
		particle->_position = particle->_prevPosition = dbTransform->worldPosition;
		particle->_initLocalPosition = dbTransform->localPosition;
		particle->_initLocalRotation = dbTransform->localRotation;

	}
	else     // end bone
	{
		UnityNode* pb = m_vecDynameicParticle[parentIndex]->_gpTransform;
		if ( m_fEndLength > 0)
		{
			UnityNode* ppb = pb->_parent; //TODO：这个可能会是空的，还没有设置父子关系
			if (ppb != nullptr)
			{
				float3 tempV = pb->worldPosition * 2 - ppb->worldPosition;
				glm::vec3 glmResult = pb->glm_worldToLocal * glm::vec4(tempV.x, tempV.y, tempV.z, 1.0f);
				particle->_endOffset = float3(tempV.x, tempV.y, tempV.z);
				particle->_endOffset *= m_fEndLength;
			}
			else
			{
				particle->_endOffset = float3(m_fEndLength, 0, 0);
			}
		}
		else
		{
			//将骨骼初始的世界坐标转为局部坐标
			float3 bb = m_EndOffset + pb->worldPosition;
			glm::vec3 glmResult = pb->glm_worldToLocal * glm::vec4( bb.x, bb.y, bb.z, 1.0f );  //先用glm的矩阵乘法，实在受不了了，后面再造轮子吧
			particle->_endOffset = Vector3(glmResult.x, glmResult.y, glmResult.z);
		}
		glm::vec3 transPoint;
		transPoint = pb->glm_localToWorld * glm::vec4( particle->_endOffset.x, particle->_endOffset.y, particle->_endOffset.z, 1.0f);
		particle->_position = particle->_prevPosition = Vector3(transPoint.x, transPoint.y, transPoint.z);
	}

	if (parentIndex >= 0)
	{
		boneLength += (m_vecDynameicParticle[parentIndex]->_gpTransform->worldPosition - particle->_position).length();
		particle->_fBoneLength = boneLength;
		m_fBoneTotalLength = (std::max)(m_fBoneTotalLength, boneLength);
	}

	int index = (int)m_vecDynameicParticle.size();
	m_vecDynameicParticle.push_back(particle);
	if (dbTransform != nullptr)
	{
		UnityNode* gpChild = dbTransform->getFirstChild();
		if (gpChild != nullptr) {
			for (int i = 0; i < dbTransform->getChildCount(); ++i)
			{
				// 这一段有一个exclude
				if (true) {
					AppendParticles(gpChild, index, boneLength);
					//gpChild = gpChild->getNextSibling();
				}
				else if (m_fEndLength > 0 || m_EndOffset != float3(0.0f, 0.0f, 0.0f))
					AppendParticles(nullptr, index, boneLength);
			}
		}

		// 按目前的测试流程 这一步也走不到
		if (dbTransform->getChildCount() == 0 && (m_fEndLength > 0 || m_EndOffset != float3(0.0f, 0.0f, 0.0f)))
			AppendParticles(nullptr, index, boneLength);
	}
}

void DynamicBone::UpdateParticleParam()
{
	// 没有绑定根节点
	if (m_pGPRoot == nullptr)
		return;	
	// 先假设重力是0
	glm::vec3 resultP;
	resultP = m_pGPRoot->glm_worldToLocal * glm::vec4(m_Info._gravity.x, m_Info._gravity.y, m_Info._gravity.z, 0.0f);
	m_LocalGravity = float3(resultP.x, resultP.y, resultP.z);
	// 初始化的过程中进行文件的IO
// 	bool needBinFile = !m_Info._binPath.empty();
// 	bool bBinFileCheck = needBinFile;
// 	if (needBinFile) {
// 		unsigned long len;
// 		char* pBuffer = ReadFile2String(m_Info._binPath.c_str(), len);
// 
// 		int nOffset = 0;
// 		int nSize = *((int*)(pBuffer + nOffset));
// 		nOffset += sizeof(int);
// 
// 		// 确保大小一致 不然肯定不对
// 		if (nSize != m_vecDynameicParticle.size()) {
// 			bBinFileCheck = false;
// 		}
// 		else {
// 			for (int i = 0; i < nSize; i++) {
// 				int nStrLen = *((int *)(pBuffer + nOffset));
// 				nOffset += sizeof(int);
// 
// 				char *name = (pBuffer + nOffset);
// 				char *resultName = new char[nStrLen + 1];
// 				memcpy(resultName, name, sizeof(char) * nStrLen);
// 				resultName[nStrLen] = '\0';
// 				std::string jointName = resultName;
// 				nOffset += nStrLen * sizeof(char);
// 				delete[] resultName;
// 
// 				float fDamping = *((float *)(pBuffer + nOffset));
// 				nOffset += sizeof(float);
// 				float fElasticity = *((float *)(pBuffer + nOffset));
// 				nOffset += sizeof(float);
// 				float fStiffness = *((float *)(pBuffer + nOffset));
// 				nOffset += sizeof(float);
// 				float fInert = *((float *)(pBuffer + nOffset));
// 				nOffset += sizeof(float);
// 				float fRadius = *((float *)(pBuffer + nOffset));
// 				nOffset += sizeof(float);
// 
// 				DynamicParticle *p = m_vecDynameicParticle[i];
// 				p->_fDamping = fDamping;
// 				p->_fElasticity = fElasticity;
// 				p->_fStiffness = fStiffness;
// 				p->_fInert = fInert;
// 				p->_fRadius = fRadius;
// 			}
// 		}
// 		SAFE_DELETE(pBuffer);
// 	}
	for (int i = 0; i < m_vecDynameicParticle.size(); ++i) {
		DynamicParticle *p = m_vecDynameicParticle[i];
		p->_fDamping = m_Info._fDamping;
		p->_fElasticity = m_Info._fElasticity;
		p->_fStiffness = m_Info._fStiffness;
		p->_fInert = m_Info._fInert;
		p->_fRadius = m_Info._fRadius;

		p->_fDamping = (std::min)((std::max)(p->_fDamping, 0.0f), 1.0f);
		p->_fElasticity = (std::min)((std::max)(p->_fElasticity, 0.0f), 1.0f);
		p->_fStiffness = (std::min)((std::max)(p->_fStiffness, 0.0f), 1.0f);
		p->_fInert = (std::min)((std::max)(p->_fInert, 0.0f), 1.0f);
		p->_fRadius = (std::max)(p->_fRadius, 0.0f);
	}
}


void DynamicBone::UpdateParticle1(float timevar)
{
	Vector3 force = m_Gravity;
	Vector3 tempG = m_Gravity;
	Vector3 fdir = tempG.normalize();
	Vector3 rf;
	glm::vec3 resultP = m_pGPRoot->glm_localToWorld * glm::vec4(m_LocalGravity.x, m_LocalGravity.y, m_LocalGravity.z, 0.0f);
	rf = float3(resultP.x, resultP.y, resultP.z);

	float3 pf = fdir * (std::max)(rf.dot(fdir), 0.0f);
	force -= pf;
	force = (force + m_Info._force) * (m_fObjectScale * timevar);

	for (int i = 0; i < m_vecDynameicParticle.size(); ++i)
	{
		DynamicParticle* p = m_vecDynameicParticle[i];
		if (p->_nParentIndex >= 0)
		{
			// verlet integration
			Vector3 v = p->_position - p->_prevPosition; //自动添加的动态骨骼节点
			Vector3 rmove = m_ObjectMove * p->_fInert;
			p->_prevPosition = p->_position + rmove;
			float damping = p->_fDamping;
			//if (p->m_isCollide)
			//{
			//	damping += p.m_Friction;
			//	if (damping > 1)
			//		damping = 1;
			//	p.m_isCollide = false;
			//}
			p->_position += v * (1 - damping) + force + rmove;
		}
		else
		{
			p->_prevPosition = p->_position; //胸部骨骼的节点位置应该进行改变，在每一帧的运动之后需要进行更新
			p->_position = p->_gpTransform->worldPosition; //这个worldPosition是否需要在外面每一帧动作完成之后进行变化呢
		}
	}
}

void DynamicBone::UpdateParticle2(float timevar)
{
	for (int i = 1; i < m_vecDynameicParticle.size(); ++i)
	{
		DynamicParticle* p = m_vecDynameicParticle[i]; //添加的动态骨骼节点
		DynamicParticle* p0 = m_vecDynameicParticle[p->_nParentIndex]; //胸部节点lPectoral，也就是所谓的父节点

		float restLen;
		if (p->_gpTransform != nullptr)
		{
			restLen = (p0->_gpTransform->worldPosition - p->_gpTransform->worldPosition).length();
		}
		else
		{
			restLen = (p0->_gpTransform->localToWorld * p->_endOffset).length();
		}

		//TODO:keep shape
		float stiffness = p->_fStiffness;
		if (stiffness > 0.0f || p->_fElasticity > 0.0f)
		{
			glm::mat4 tempMat = p0->_gpTransform->glm_localToWorld;
			tempMat[3][0] = p0->_position.x;
			tempMat[3][1] = p0->_position.y;
			tempMat[3][2] = p0->_position.z;

			float3 restPos;
			if (p->_gpTransform != nullptr)
			{
				//parentMatrix.transformPoint(p->_gpTransform->localPosition, &restPos);
				glm::vec3 resultP = tempMat * glm::vec4( p->_gpTransform->localPosition.x, p->_gpTransform->localPosition.y,
																					p->_gpTransform->localPosition.z, 1.0f );
				restPos = float3(resultP.x, resultP.y, resultP.z);
			}
			else 
			{
				//parentMatrix.transformPoint(p->_endOffset, &restPos);
				glm::vec3 resultP = tempMat * glm::vec4(p->_endOffset.x, p->_endOffset.y, p->_endOffset.z, 1.0f);
				restPos = Vector3(resultP.x, resultP.y, resultP.z);
			}

			float3 d = restPos - p->_position;
			p->_position += d * (p->_fElasticity * timevar);

			if (stiffness > 0.0f)
			{
				d = restPos - p->_position;
				float len = d.length();
				float maxlen = restLen * (1.0f - stiffness) * 2.0f;
				if (len > maxlen)
					p->_position += d * ((len - maxlen) / len);
			}
		}

		if (!m_vecDynamicCollider.empty())
		{
			float particleRadius = p->_fRadius * m_Info._radiusScale;
			for (int j = 0; j < m_vecDynamicCollider.size(); ++j)
			{
				DynamicBoneColliderBase* c = m_vecDynamicCollider[j];
				if (c != nullptr) {
					c->Collide(p->_position, particleRadius);
				}
			}
		}

		// keep length
		float3 dd = p0->_position - p->_position;
		float leng = dd.length();
		if (leng > 0)
			p->_position += dd * ((leng - restLen) / leng);
	}
}

// only update stiffness and keep bone length
void DynamicBone::SkipUpdateParticles()
{
	for (int i = 0; i < m_vecDynameicParticle.size(); ++i)
	{
		DynamicParticle* p = m_vecDynameicParticle[i];
		if (p->_nParentIndex >= 0)
		{
			p->_prevPosition += m_ObjectMove;
			p->_position += m_ObjectMove;

			DynamicParticle* p0 = m_vecDynameicParticle[p->_nParentIndex];

			float restLen;
			if (p->_gpTransform != nullptr)
				restLen = (p0->_gpTransform->getTranslationWorld() - p->_gpTransform->getTranslationWorld()).length();
			else
				restLen = 0.0f;

			// keep shape
			float stiffness = p->_fStiffness;
			if (stiffness > 0)
			{
				float* pSource = const_cast<float*>(p0->_gpTransform->getWorldMatrix().m);
				float m0[16];
				memcpy(m0, pSource, 16 * sizeof(float));
				m0[12] = p0->_position.x;
				m0[13] = p0->_position.y;
				m0[14] = p0->_position.z;
				m0[15] = 0.0f;

				mat4f parentMatrix;
				parentMatrix.set(m0);

				float3 restPos;
				if (p->_gpTransform != nullptr)
				{
					parentMatrix.transformPoint(p->_gpTransform->getTranslation(), &restPos);
				}
				else {
					parentMatrix.transformPoint(p->_endOffset, &restPos);
				}

				float3 d = restPos - p->_position;
				p->_position += d * p->_fElasticity;

				if (stiffness > 0.0f)
				{
					float len = d.length();
					float maxlen = restLen * (1.0f - stiffness) * 2.0f;
					if (len > maxlen)
						p->_position += d * ((len - maxlen) / len);
				}
			}

			// keep length
			float3 dd = p0->_position - p->_position;
			float leng = dd.length();
			if (leng > 0)
				p->_position += dd * ((leng - restLen) / leng);
		}
		else
		{
			p->_prevPosition = p->_position;
			p->_position = p->_gpTransform->getTranslationWorld();
		}
	}
}


void DynamicBone::ApplyParticlesToTransforms()
{
	for ( int i = 1; i < m_vecDynameicParticle.size(); ++i )
	{
		DynamicParticle* p = m_vecDynameicParticle[i];
		DynamicParticle* p0 = m_vecDynameicParticle[p->_nParentIndex];

		if (p0->_gpTransform->_childCount <= 1)
		{
			float3 v;
			if (p->_gpTransform != nullptr)
			{
				v = p->_gpTransform->localPosition;
			}
			else
			{
				v = p->_endOffset;
			}

			float3 v2 = p->_position - p0->_position;
			glm::vec3 resultP = p0->_gpTransform->glm_localToWorld * glm::vec4(v.x, v.y, v.z, 0.0f);
			float3 transPoint{ resultP.x, resultP.y, resultP.z };

			quaternion4f rot = quaternion4f::FromToRotation(transPoint, v2); //经过验证，这个FromToRotation()没有问题，计算结果与unity一致
			p0->_gpTransform->worldRotation = rot * p0->_gpTransform->worldRotation;	
		}

		if (p->_gpTransform != nullptr)
		{
			p->_gpTransform->worldPosition = p->_position;
		}
	}
}
