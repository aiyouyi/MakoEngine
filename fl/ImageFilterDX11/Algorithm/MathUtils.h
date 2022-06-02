#ifndef _H_MATHUTILS_H_
#define _H_MATHUTILS_H_

#include "BaseDefine/Define.h"
#include "Matrices.h"
#include <cmath>
#include <vector>
using namespace std;

typedef unsigned char byte;
typedef unsigned char BYTE;

#define MATRIX_SIZE ( sizeof(float) * 16)
#define MATH_TOLERANCE              2e-37f

class CMathUtils
{
	friend class Matrix;
	friend class Vector3;
public:
	//获取两个三角形的仿射变换矩阵;
	//@param [src] 包含3个点,表示原始三角形
	//@param [dst] 包含3个点,表示目标三角形
	static Matrix3 getAffineTransform(Vector2 src[],Vector2 dst[]);
	//将一组顶点通过仿射变换矩阵仿射到目标;
	//@param [srcArr] 原始点集
	//@param [dstArr] 目标点集
	//@param [mat]	  仿射变换矩阵
	static void transform(vector<Vector2> &srcArr,vector<Vector2> &dstArr,Matrix3 &mat);

	static void transform(Vector2* srcArr, Vector2* dstArr, int nPoints,Matrix3 &mat);
	//拉格朗日插值算法;
	//@param [polyfitIn],[InNum] 原始点和个数
	//@param [polyfitOut],[OutNum] 输出点和个数
	static void LagrangePolyfit(Vector2* polyfitIn,int InNum,Vector2* polyfitOut,int OutNum);
	//拉格朗日插值算法;
	//@param [polyfitIn]原始点
	//@param [polyfitOut],[OutNum] 输出点和个数
	static void LagrangePolyfit( vector<Vector2> polyfitIn, vector<Vector2>& polyfitOut, int ptnum );
	//计算两点间的单位向量
	//@param [startPt] 输入起始点
	//@param [endPt]   输入终点
	//@param [UnitVec] 结果单位向量
	static void CalcUnitVector(Vector2 &startPt, Vector2 &endPt ,Vector2 &UnitVec);
	//计算点集的凸包
	//@warn 计算完后点的顺序和数量会发生变化
	static void CalcConvexHull(vector<Vector2> &vecSrc);
	//判断点在多边形内(凹凸均可用);
	//@param [polygon] 多边形点
	//@param [N] 多边形的点数共N+1个点,且!!polygon[N] == poylgon[0]!!
	//@param [p] 要判断的点
	static bool InsidePolygon(Vector2 *polygon,int N,Vector2 p);
	//填充相关区域(要求凸多边形),内部用fillColor
	//@param [pMask] 要填充的mask
	//@param [width][height] mask宽高
	//@param [points] 多边形的点集,!!第一个点和最后一个点无需重合!!
	//@param [fillColor] 内部填充值
	static bool FillRect5(byte* pMask,int width,int height,vector<Vector2>& points,byte fillColor = 255);
	//填充相关区域(要求凸多边形),外部用fillColor
	//@param [pMask] 要填充的mask
	//@param [width][height] mask宽高
	//@param [points] 多边形的点集,!!第一个点和最后一个点无需重合!!
	//@param [fillColor] 外部填充值
	static bool FillRect6(byte* pMask,int width,int height,vector<Vector2>& points,byte fillColor = 255);
	//画线函数
	//@param [nWidth]画布的图片宽
	//@param [nHeight]画布的图片高
	//@param [points] 线的点集
	//@return 画完线的单通道图
	static BYTE* DrawLine(int nWidth,int nHeight,int pointCount,Vector2* points);
	//获取点集的外接矩形
	//@param [srcArr]点集
	//@param [left,top,right,bottom] 最终的外接矩形
	//@return 
	static void GetOutSideRect(vector<Vector2> &srcArr,int& left,int& top,int& right,int& bottom);



	static void GetOutSideRect(Vector2 *srcArr,int nCount, int& left, int& top, int& right, int& bottom);

	// 点连线
	static void ConnectPoint(Vector2** ppPoints, int nCount, BYTE* pBuffer, int nWidth, int nHeight);
	// 种子填充
	static void SeedFill(BYTE *pMask, int width, int height, int startX, int startY);
	//2次曲线平滑
	//@param [pOutPts]最终的输出点
	//@param [pInPts]输入点
	//@param [tension] 曲率
	//@param [numOfSegments] 每2个点直接要插值的点
	//@return 最终生成的点的个数
	static int SmoothLines(vector<Vector2>& pOutPts, vector<Vector2>& pInPts ,float tension = 0.5,int numOfSegments = 16);

	static int SmoothLines2(Vector2* pOutPts, Vector2* pInPts, int nPoints, float tension = 0.5, int numOfSegments = 16);
private:
	template<typename _Tp> static  int
		LUImpl(_Tp* A, size_t astep, int m, _Tp* b, size_t bstep, int n);

	static bool SolveMat(double _src[36],double _src2arg[6],double _dst[6]);

	template<typename T, typename WT> static void
		transform_( const T* src, T* dst, const WT* m, int len, int cn, int dcn );

public:
	 static void addMatrix(const float* m, float scalar, float* dst);

	 static void addMatrix(const float* m1, const float* m2, float* dst);

	 static void subtractMatrix(const float* m1, const float* m2, float* dst);

	 static void multiplyMatrix(const float* m, float scalar, float* dst);

	 static void multiplyMatrix(const float* m1, const float* m2, float* dst);

	 static void negateMatrix(const float* m, float* dst);

	 static void transposeMatrix(const float* m, float* dst);

	 static void transformVector4(const float* m, float x, float y, float z, float w, float* dst);

	 static void transformVector4(const float* m, const float* v, float* dst);

	 static void crossVector3(const float* v1, const float* v2, float* dst);

};
#endif