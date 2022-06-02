#ifndef FACE_INTERLATION_H 
#define FACE_INTERLATION_H
#include<vector>
#include "Point2D.h"
#define short3D unsigned short
#include "BaseDefine/Define.h"

namespace mt3dface
{

	/**
	* @brief 人脸点插值的算法基础类
	*/
	class MathUtil
	{
	public:
		MathUtil() {}
		~MathUtil() {}

		/**
		* @brief 圆弧的两个端点和近似圆心点插值出圆弧的中间点
		* @param [in] pt1 圆弧的终点
		* @param [in] pt2 圆弧的起点
		* @param [in] M 圆弧的近似圆心
		* @param [in] u 控制权值
		* @return 返回插值点
		*/
		static Point2D getFeatureInterpolatValue(Point2D pt1, Point2D pt2, Point2D M, float u);
		/**
		* @brief 拉格朗日插值
		* @param [in] polyfitIn 原始的点对
		* @param [out] polyfitOut 插值后的点对
		* @param [in] ptnum 插值点的个数
		* @return
		*/
		static void LagrangePolyfit(std::vector<Point2D> polyfitIn, std::vector<Point2D>& polyfitOut, int ptnum);
		/**
		* @brief 根据两直线的四个点来判断两直线是否交叉并求交叉点
		* @param [in] Point1 直线1的点
		* @param [in] Point2 直线1的点
		* @param [in] Point3 直线2的点
		* @param [in] Point4 直线2的点
		* @param [out] OutPoint 两直线的交叉点
		* @return 两直线交叉返回true，否则换回false
		*/
		static void Intersection(Point2D Point1, Point2D Point2, Point2D Point3, Point2D Point4, Point2D& OutPoint);
		//static void CalPedalPoints(Point2D PointA, Point2D PointB, Point2D PointC,Point2D& PointD);
	};

	/**
	 * @brief 基于人脸库输出的106个landmark点，通过插值算法衍生出235个landmark点和vt点。
	 *
	 * 该人脸插值点和索引图如下所示
	 * @image html point_index_Image.jpeg "人脸插值点和索引图" height=900px width=600px
	 */
	class FaceInterlation
	{
	public:
		FaceInterlation()= default;
		~FaceInterlation()= default;

		/**
		 * @brief 基于landmark点和vt点人脸插值执行函数
		 * @param [in] pFacePoint106 人脸库输出的landmark点或对应的vt点
		 * @param [in] numLandmarkPoint 人脸库输出的landmark点或对应的vt点个数
		 * @param [in] numIntelatPoint 插值点个数
		 * @param [in] NonLineFlag 线性插值和非线性插值的标记
		 * @param [in] ForeheadWeight 额头最顶点的插值系数
		 * @param [in] ForeheadCTWeight 额头插值轮廓大小控制系数
		 * @param [in] VtFlag 是否进行Vt局部调整的标志
		 * @param [out] pImagePoint 插值后的landmark点或vt点
		 * @return 有人脸点返回true，否则返回false
		 */
		static bool RunFacePointInterpolation(const float *pFacePoint106, int numLandmarkPoint, int numIntelatPoint, bool *NonLineFlag, float ForeheadWeight, float ForeheadCTWeight, bool VtFlag, float *pImagePoint);
	    /**
		 * @brief 基于landmark点人脸和背景插值执行函数
		 * @param [in] pFacePoint106 人脸库输出的landmark点
		 * @param [in] numLandmarkPoint 人脸库输出的landmark点个数
		 * @param [in] numIntelatPoint 插值点个数
		 * @param [in] NonLineFlag 线性插值和非线性插值的标记
		 * @param [in] ForeheadWeight 额头最顶点的插值控制系数
		 * @param [in] ForeheadCTWeight 额头插值轮廓大小控制系数
	     * @param [in] PitchAngle 人脸姿态俯仰角(单位:度)
	     * @param [in] FaceHight 人脸框高度(该人脸为内部自定义人脸框非人脸库输出)
	     * @param [in] VtFlag 是否进行Vt局部调整的标志
		 * @param [out] pImagePoint 插值后的landmark点
		 * @return 有人脸点返回true，否则返回false
		 */
		static bool RunFaceAndBackPointInterpolation(const float *pFacePoint106, int numLandmarkPoint, int numIntelatPoint, bool *NonLineFlag,
													 float ForeheadWeight, float ForeheadCTWeight, float PitchAngle, float FaceHight, bool VtFlag, float *pImagePoint);
        /**
         * @brief 基于vt点背景插值执行函数
         * @param [in] pFacePointVt 人脸的Vt点
         * @param [in] numIntelatPoint 插值点个数
         * @param [in] ForeheadCTWeight  额头插值轮廓大小控制系数
         * @param [in] VtFlag 是否进行Vt局部调整的标志
         * @param [in,out]pImagePointVt 插值后的Vt点
         * @return
         */
		static bool RunFacePointVtInterpolation(const float *pFacePointVt, int numIntelatPoint, float ForeheadCTWeight, bool VtFlag, float *pImagePointVt);
	private:
		/**
		 * @brief 人脸库的输出的landmark点或对应的Vt点，插值索引为0-105
		 * @param [in] pFacePoint106 人脸库输出的landmark点或对应的vt点
		 * @param [in] numLandmarkPoint 人脸库输出的landmark点或对应的vt点个数
		 * @param [in] nCurrentIndex 当前将插值点的索引
		 * @param [int,out] pFacePoint 插值后的landmark点或vt点
		 * @return 下一个将插值点的索引
		 */
		static int CalFaceLandmarkPoint(const Point2D* pFacePoint106, int numLandmarkPoint, int nCurrentIndex, Point2D* pFacePoint);
        /**
         * @brief 人脸额头位置的插值，插值索引为106-128
         * @param ForeheadWeight 额头最顶点的插值系数
         * @param ForeheadCTWeight 额头插值轮廓大小控制系数
         * @param nCurrentIndex 当前将插值点的索引
         * @param [in] VtFlag 是否进行Vt局部调整的标志
         * @param pFacePoint 插值后的landmark点或vt点
         * @return 下一个将插值点的索引
         */
        static int CalForeheadPoint(float ForeheadWeight, float ForeheadCTWeight, int nCurrentIndex, bool ChooseFaorVtFlag, bool VtFlag, Point2D* pFacePoint);
		/**
		 * @brief 人脸左脸颊位置的插值，插值索引为129-149
		 * @param [in] nCurrentIndex 当前将插值点的索引
		 * @param [in,out] pFacePoint 插值后的landmark点或vt点
		 * @return 下一个将插值点的索引
		 */
		static int CalLeftCheckPoint(int nCurrentIndex, Point2D* pFacePoint);
		/**
		 * @brief 人脸右脸颊位置的插值，插值索引为150-170
		 * @param [in] nCurrentIndex 当前将插值点的索引
		 * @param [in,out] pFacePoint 插值后的landmark点或vt点
		 * @return 下一个将插值点的索引
		 */
		static int CalRightCheckPoint(int nCurrentIndex, Point2D* pFacePoint);
		/**
         * @brief 人脸眼睛位置的插值，插值索引为171-186
         * @param [in] nCurrentIndex 当前将插值点的索引
         * @param [in,out] pFacePoint 插值后的landmark点或vt点
         * @return 下一个将插值点的索引
         */
		static int CalEyePoint(int nCurrentIndex, Point2D* pFacePoint);
		/**
         * @brief 人脸嘴巴外轮廓位置的插值，插值索引为187-210
         * @param [in] nCurrentIndex 当前将插值点的索引
         * @param [in,out] pFacePoint 插值后的landmark点或vt点
         * @return 下一个将插值点的索引
         */
		static int CalOutMouthPoint(bool *ModefyPointFlag, int nCurrentIndex, Point2D* pFacePoint);
		/**
         * @brief 人脸嘴巴内轮廓位置的插值，插值索引为211-226
         * @param nCurrentIndex 当前将插值点的索引
         * @param pFacePoint 插值后的landmark点或vt点
         * @return 下一个将插值点的索引
         */
		static int CalInnerMouthPoint(bool *ModefyPointFlag, int nCurrentIndex, Point2D* pFacePoint);
		/**
         * @brief 人脸鼻嘴之间和下巴位置的插值，插值索引为227-234
         * @param [in] nCurrentIndex 当前将插值点的索引
         * @param [in,out] pFacePoint 插值后的landmark点或vt点
         * @return 下一个将插值点的索引
         */
		static int CalChainPoint(int nCurrentIndex, Point2D* pFacePoint);
		/**
		 * @brief 人脸以外的背景插值，插值索引为235-260
		 * @param [in] PitchAngle 人脸姿态俯仰角(单位:度)
		 * @param [in] nCurrentIndex 当前将插值点的索引
         * @param [in,out] pFacePoint 插值后的landmark点或vt点
		 * @return 下一个将插值点的索引
		 */
		static int CalBackgroundPoint(float PitchAngle,int nCurrentIndex, Point2D* pFacePoint);
};//class FaceInterlation

class EyePupilInterlation
{
public:
	EyePupilInterlation() = default;
	~EyePupilInterlation() = default;
	static bool RunEyePupilInterlation(const float * pFacePoint, int numLandmarkPoint,  int numIntelatPoint, float *pPuiplePoint);
private:
	static int EyePupilInterlate(const Point2D * pFacePoint, int nCurrentIndex, Point2D* pPuiplePoint);
};

}//namespace mt3dface

#endif 



