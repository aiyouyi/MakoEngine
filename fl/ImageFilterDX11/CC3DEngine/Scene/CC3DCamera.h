#pragma once
#ifndef _H_CC3D_CAMERA_H_
#define _H_CC3D_CAMERA_H_

#include "Common/CC3DUtils.h"
#include <glm/gtc/quaternion.hpp>
class CC3DCamera
{
public:
	CC3DCamera(void);
	CC3DCamera(glm::vec3 cameraPos , glm::vec3 lookAtPos , glm::vec3 upDir);
	~CC3DCamera(void);

	/*****************************************************************
	* 获取相机矩阵 —— 直接传入OpenGL的program中
	*****************************************************************/
	float* Get();

	/*****************************************************************
	* 设置相机位置
	* @param [pos] 世界坐标下的位置
	*****************************************************************/
	void SetCameraPos(glm::vec3 pos);
	
	/*****************************************************************
	* 相机看的位置
	* @param [pos] 世界坐标下的位置
	*****************************************************************/
	void SetLookAtPos(glm::vec3 pos);
	
	/*****************************************************************
	* 相机正上方的朝向
	* @param [dir] 正上方朝向
	*****************************************************************/
	void SetUpDir(glm::vec3 dir);

	/*****************************************************************
	* 获取视图矩阵
	*****************************************************************/
	glm::mat4& GetViewMatrix();
	
	/*****************************************************************
	* 获取相机位置
	*****************************************************************/
	glm::vec3& GetCameraPos();

	/*****************************************************************
	* 相机看的位置
	*****************************************************************/
	glm::vec3& GetLookAtPos();
	
	/*****************************************************************
	* 相机正上方的朝向
	*****************************************************************/
	glm::vec3& GetUpDir();

public:
	//设置初始加载时物体离相机的距离
	void SetDistanceToRO(float z);
	void SetTranslate(float x, float y, float z)
	{

	}
	glm::mat4 GetWorldToOrign();

	void SetScale(float x, float y, float z);
	void setRoatate(float xAngle, float yAngle, float zAngle);
public:
	//刷新视图矩阵
	void UpdateViewMatrix();
	void UpdateViewMatrix(const Vector3& eye, const Vector3& center, const Vector3& up);

	//相机位置
	glm::vec3 m_CameraPos;
	//相机看的方向
	glm::vec3 m_LookAtPos = glm::vec3(0.0f);
	//相机正上方的朝向
	glm::vec3 m_UpDir;

	//视图矩阵
	glm::mat4 m_ViewMatrix;

	//投影矩阵
	glm::mat4 m_ProjectMatrix;

	//正射相机的近平面和远平面
	float lsNear = 0.0f;
	float lsFar = 0.0f;

	float distance_to_ro = 100.0f; // ro  => render object
	float moveDistanceEachStep = 1.0f;
};

#endif // _H_CC3D_CAMERA_H_
