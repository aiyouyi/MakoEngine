#ifndef __MLS__H__
#define __MLS__H__
#include "BaseDefine/Define.h"
#include <vector>
using namespace std;

class MLS
{
public:
	MLS(void);
	~MLS(void);

	/**
	* @brief : 通过控制点p和q以及原图像素点坐标仿射变换来进行转化，把轮廓坐标在效果图的坐标输出
	*/
	void MLSD2DpointsDefAffine(float** srcPoints, float** desPoints, float** p, float** q, int nplength, int nPoints);
	/**
	* @brief : 通过控制点p和q以及原图像素点坐标刚性变换来进行转化，把轮廓坐标在效果图的坐标输出
	*/
	void MLSD2DpointsDefRigid(float** srcPoints, float** desPoints, float** p, float** q, int nplength, int nPoints);

protected:
	/**
	* @brief : 利用各像素点坐标和控制点p计算权重
	*/
	void PrecomputeWeights(float** p, float** v, float** w, int nplength, int nvlength);

	/**
	* @brief : 计算p*或q*的值
	*/
	void PrecomputeWCentroids(float** p, float** weight, float** pPstar, int nlength, int nwheight);

	/**
	* @brief : 矩阵相乘
	*/
	void muliplymatrix(float** leftMatrix, float** rightMatrix, float** desMatrix, int nwidth, int nheight, int nmiddle);

	/**
	* @brief : 根据仿射变换对原图坐标信息和控制各点p构造公式的A矩阵
	*/
	void PrecomputeAffine(float** p, float** v, float** weight, float** A, int nvlength, int nplength);
	/**
	* @brief : 根据刚性变换对原图坐标信息和控制各点p构造公式的A矩阵信息以及|v-p*|值
	*/
	void PrecomputeRigid(float** p, float** v, float** weight, float* normof_v_Pstar, vector<float**> &vA, int ngvlength, int nplength);

	/**
	* @brief : 按照仿射变换
	*/
	void PointsTransformAffine(float** fA, float** weight, float** q, float** fv, int nqlength, int nlength);

	/**
	* @brief : 计算公式中转换矩阵中的各A矩阵信息，存入vector容器中
	*/
	void PrecomputeA(float** Pstar, vector<float**> vPhat, float** v, float** weight, vector<float**> &vA, float* v_Pstar,int nlength, int nheight);

	/**
	* @brief : 按照刚性变换公式2.23构造出转换函数fr(v)
	*/
	void PointsTransformRigid(vector<float**> vA, float* normof_v_Pstar, float** weight, float** q, float** fv, int nqlength, int nlength);
};

#endif