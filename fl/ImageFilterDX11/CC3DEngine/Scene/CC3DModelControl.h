#pragma once
#ifndef _H_CC3D_MODELCONTROL_H_
#define _H_CC3D_MODELCONTROL_H_
#include "Common/CC3DUtils.h"
class CC3DModelControl
{
public:
	CC3DModelControl(void);
	virtual ~CC3DModelControl(void);

	float* Get();
	float* GetNormal();
	glm::mat4& GetModelMatrix();

	//当前状态
	void SetTranslate(float x, float y, float z);
	void AddTranslate(float x, float y, float z);

	void SetScale(float x, float y, float z);
	void AddScale(float x, float y, float z);

	void SetRotate(float xAngle, float yAngle, float zAngle);
	void AddRotateX(float xAngle);
	void AddRotateY(float yAngle);
	void AddRotateZ(float zAngle);
public:
	void UpdateModelMatrix();
	
	//位移矩阵
	glm::mat4 m_TransMatrix;
	//缩放矩阵
	glm::mat4 m_ScaleMatrix;
	//旋转矩阵
	glm::mat4 m_RotateMatrix;
	//模型矩阵
	glm::mat4 m_ModelMatrix;

	glm::mat4 m_InitMatrix;




};


#endif // _H_CC3D_MODELCONTROL_H_
