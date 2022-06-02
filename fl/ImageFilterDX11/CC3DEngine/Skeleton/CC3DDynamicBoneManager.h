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
	  @初始化动态骨骼所持有的transfrom接口，记录位置信息与转换矩阵
	  @param: db_index  动态骨骼索引，为-1时表示没有动态骨骼
	*/
	void InitParticle(CC3DTransformNode* transNode, int db_index = -1);
	/*
	 @动态骨骼transfrom局部坐标初始化
	 @param: db_index  动态骨骼索引，为-1时表示没有动态骨骼
	*/
	void InitTransfrom(int db_index = -1);
	/*
	 @递归初始化动态骨骼各节点，初始化每个transform接口
	 @param: CC3DBoneNodeInfo  骨骼信息
	*/
	void DFSInitDynamicBoneNode(CC3DBoneNodeInfo* info, glm::mat4& ParentMatrix);
	/*
	 @动态骨骼前更新，将动画每一帧的变化都同步到动态骨骼的transform接口
	 @param: CC3DBoneNodeInfo  骨骼信息
	*/
	void DynamicBonePreUpdate(CC3DBoneNodeInfo* info, glm::mat4& ParentMatrix);
	/*
	 @动态骨骼后更新，等待骨骼动画动作完成之后更新动态骨骼位置
	 @param: db_index  动态骨骼索引，为-1时表示没有动态骨骼
	*/
	void LateUpdate(int db_index = -1);

public:
	//动态骨骼各个particle的transform接口
	std::unordered_map<string, CC3DTransformNode*>		transformMap;
	//动态骨骼配置项参数
	std::vector<dynamicBoneParameter>					dynamicBoneNamesArray;
	//动态骨骼列表
	std::vector<std::shared_ptr<DynamicBone>>			dynamicBoneArray;
};

#endif