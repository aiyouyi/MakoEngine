#pragma once
#ifndef _H_CC3D_PREJECTION_H_
#define _H_CC3D_PREJECTION_H_
#include "Common/CC3DUtils.h"
//投影矩阵
class CC3DProject
{
public:
	CC3DProject(void);
	virtual ~CC3DProject(void);
	//获取矩阵信息
	float* Get();
	//获取投影矩阵
	glm::mat4& GetProjectMatrix();
	//更新投影矩阵
	virtual void UpdateProjectMatrix(){};

	//投影矩阵
	glm::mat4 m_ProjectMatrix;
//protected:

};
//正交投影矩阵
class CC3DOrthographicProject : public CC3DProject
{
public:
	CC3DOrthographicProject(void);
	CC3DOrthographicProject(float fLeft , float fRight , float fTop , float fBottom , float fNear , float fFar);
	virtual	~CC3DOrthographicProject(void);
	//重置矩阵
	void Rest(float fLeft, float fRight, float fTop, float fBottom, float fNear, float fFar);
	//更新投影矩阵
	virtual void UpdateProjectMatrix();
	void SetLeft(float fLeft);
	void SetRight(float fRight);
	void SetTop(float fTop);
	void SetBottom(float fBottom);
	void SetNear(float fNear);
	void SetFar(float fFar);
protected:
	float m_fLeft;
	float m_fRight;
	float m_fTop;
	float m_fBottom;
	float m_fNear;
	float m_fFar;
};
//透视投影矩阵
class CC3DPerspectiveProject : public CC3DProject
{
public:
	CC3DPerspectiveProject(void);
	CC3DPerspectiveProject(float fFovy, float fAspect, float fNear, float fFar);
	~CC3DPerspectiveProject(void);
	//重置矩阵
	void Rest(float fFovy, float fAspect, float fNear, float fFar);

	//更新投影矩阵
	virtual void UpdateProjectMatrix();

	void SetFovy(float fFovy);
	void SetAspect(float fAspect);
	void SetNear(float fNear);
	void SetFar(float fFar);
protected:
	float m_fFovy;
	float m_fAspect;
	float m_fNear;
	float m_fFar;
};
#endif // _H_CC3D_PREJECTION_H_
