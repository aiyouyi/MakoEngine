/*****************************************************************
* PerspectiveEstimeCamera ,透视投影姿态预估类:利用在人脸FA上选取的点和对应的3dModel上的对估计一个姿态矩阵
* Copyright (c) 2016年 MEITU. All rights reserved.
*
* @version: 1.0
*
* @author:  lym
*
* @date: 2017-10-12
*
* @note:
*
* @usage：
*
******************************************************************/
#ifndef PERPECTTIVE_ESTIMATA_CARAMERA_LY2149i
#define PERPECTTIVE_ESTIMATA_CARAMERA_LY2149i

#include "Common3D.hpp"

namespace mt3dface {
    
struct  PerspectiveCameraParams
{
	int nWidth;
	int nHeight;
	float focalLength;
	float fovy,far2,near2;
};

class MT3DFace_EXPORT PerspectiveEstimeCamera
{
public:
	PerspectiveEstimeCamera();
	~PerspectiveEstimeCamera();

	/*
	@param pModelPoints：model上选取的点
	@param pImagePoints 美图的106个点上选取的点
	@param nNumsOfPoints：选取点的个数
	@param nWidth: 输入图宽
	@param nHeight: 输入图高
	@param pProjectionMat :输出3*4姿态预估矩阵
	@param pPointWieght   : 选取点的权重
	@param pCameraParm : 输出，分别为旋转角rx,ry,rz,平移量tx，ty,tz
	@param isInit: 是否需要用pCameraParm做初始化，主要用于视频
	@return ：
	*/
	void EstimateProMat(float* pModelPoints, float* pImagePoints, int nNumsOfPoints, int nWidth, int nHeight, float* pProjectionMat,
		float *pPointWieght, float *pCameraParm = 0, bool isInit = false, float FovAngle = 45.f);
private:
	PerspectiveCameraParams m_CameraParams;
	float m_numer, m_denom;

	float m_StablePoint[24];

private:
	float m_CostWithLastFrameParam;
//	vector<int> m_NoseIndex;

};

///-------------------------------------------------------------------------------------------------
/// @fn	void ProjectPoint(float *p, float *ProjectViewMat, float*out, float nWidth, int nHeight);
///
/// @brief	将模型上的3D点投影到图像坐标系
///
/// @author	Alop
/// @date	18/6/19
///
/// @param [in]	p			  	一个3D点
/// @param [in]	ProjectViewMat	the project view matrix. 其中project为透视投影
/// @param [out]	out			输出一个2D点
/// @param 		   	nWidth		The width.
/// @param 		   	nHeight		The height.
///-------------------------------------------------------------------------------------------------

void ProjectPoint(float *p, float *ProjectViewMat, float*out, float nWidth, int nHeight);





} //namespace mt3dface
#endif //PERPECTTIVE_ESTIMATA_CARAMERA_LY2149i
