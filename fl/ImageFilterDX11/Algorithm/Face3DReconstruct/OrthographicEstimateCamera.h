/*****************************************************************
* OrthographicEstimateCamera ,姿态预估类:利用在人脸FA上选取的点和对应的3dModel上的对估计一个姿态矩阵
* Copyright (c) 2016年 MEITU. All rights reserved.
*
* @version: 1.0
*
* @author:  lym
*
* @date: 2016-11-25
*
* @note:
*
* @usage：
*
******************************************************************/
#ifndef ORTHOGRAPHIC_ESTIMATA_CARAMERA_LYMDSKJF
#define ORTHOGRAPHIC_ESTIMATA_CARAMERA_LYMDSKJF

#include "Common3D.hpp"

namespace mt3dface {
class MT3DFace_EXPORT OrthographicEstimateCamera
{
public:
	OrthographicEstimateCamera();
	~OrthographicEstimateCamera();
	/*
	@param pModelPoints：model上选取的点
	@param pImagePoints 美图的106个点上选取的点
	@param nNumsOfPoints：选取点的个数
	@param nWidth: 输入图宽
	@param nHeight: 输入图高
	@param pProjectionMat :输出3*4姿态预估矩阵
	@param pPointWieght   : 选取点的权重
	@param pCameraParm : 输出，分别为旋转角rx,ry,rz,平移量tx，ty,缩放值共6个数
	@param isInit: 是否需要用pCameraParm做初始化，主要用于视频
	@return ：
	*/
	void EstimateProMat(float* pModelPoints, float* pImagePoints, int nNumsOfPoints, int nWidth, int nHeight, float* pProjectionMat,
		float *pPointWieght,float *pCameraParm = 0,bool isInit = false);

};
} //namespace mt3dface
#endif //ORTHOGRAPHIC_ESTIMATA_CARAMERA_LYMDSKJF
