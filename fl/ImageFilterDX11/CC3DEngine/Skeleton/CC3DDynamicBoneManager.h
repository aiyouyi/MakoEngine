#ifndef _CC_DYNAMIC_BONE_MANAGER_H_
#define _CC_DYNAMIC_BONE_MANAGER_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "glm/matrix.hpp"

#include "CC3DTransformNode.h"
#include "CC3DDynamicBone.h"
using std::string;


struct CC3DBoneNodeInfo;
struct dynamicBoneParameter;

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

public:
	//��̬��������particle��transform�ӿ�
	std::unordered_map<string, CC3DTransformNode*>		transformMap;
	//��̬�������������
	std::vector<dynamicBoneParameter>					dynamicBoneNamesArray;
	//��̬�����б�
	std::vector<std::shared_ptr<DynamicBone>>			dynamicBoneArray;
};

#endif