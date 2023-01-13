#ifndef _CC_DYNAMIC_BONE_MANAGER_H_
#define _CC_DYNAMIC_BONE_MANAGER_H_

#include "CC3DSkeleton.h"

#include "CC3DTransformNode.h"
#include "CC3DDynamicBone.h"
#include "CC3DDynamicBoneCollider.h"


struct CC3DBoneNodeInfo;


class CC3DDyanmicBoneManager
{
public:
	CC3DDyanmicBoneManager();
	~CC3DDyanmicBoneManager();
public:	
	/*
	  @��ʼ����̬���������е�transfrom�ӿڣ���¼λ����Ϣ��ת������
	  @param: db_index  ��̬����������Ϊ-1ʱ��ʾû�ж�̬����
	*/
	void InitParticle(CC3DTransformNode* transNode, int db_index = -1);
	/*
	 @��̬����transfrom�ֲ������ʼ��
	 @param: db_index  ��̬����������Ϊ-1ʱ��ʾû�ж�̬����
	*/
	void InitTransfrom(int db_index = -1);
	/*
	 @�ݹ��ʼ����̬�������ڵ㣬��ʼ��ÿ��transform�ӿ�
	 @param: CC3DBoneNodeInfo  ������Ϣ
	*/
	void DFSInitDynamicBoneNode(CC3DBoneNodeInfo* info, glm::mat4& ParentMatrix);
	/*
	 @��̬����ǰ���£�������ÿһ֡�ı仯��ͬ������̬������transform�ӿ�
	 @param: CC3DBoneNodeInfo  ������Ϣ
	*/
	void DynamicBonePreUpdate(CC3DBoneNodeInfo* info, glm::mat4& ParentMatrix);
	/*
	 @��̬��������£��ȴ����������������֮����¶�̬����λ��
	 @param: db_index  ��̬����������Ϊ-1ʱ��ʾû�ж�̬����
	*/
	void LateUpdate(int db_index = -1);

	void ResetDynamicBone();

	void DeleteDynamicBone(const std::string& db_name);

	void recuDeleteTransfromNode(const std::string& db_name);

	void UpdateDynamicBoneParameter(const CC3DImageFilter::dynamicBoneParameter& param);

	bool AddDynamicBoneCollider(std::string bone_name, DynamicBoneColliderBase::DynamicBoneColliderInfo dbc_info, CC3DTransformNode* trans_node);

public:
	//��̬��������particle��transform�ӿ�
	std::unordered_map<std::string, CC3DTransformNode*>		transformMap;
	//��̬�������������
	std::vector<CC3DImageFilter::dynamicBoneParameter>					dynamicBoneNamesArray;
	//��̬�����б�
	std::vector<std::shared_ptr<DynamicBone>>			dynamicBoneArray;
	//��̬������ײ��
	std::vector<DynamicBoneColliderBase*>				colliderArray;
};

#endif