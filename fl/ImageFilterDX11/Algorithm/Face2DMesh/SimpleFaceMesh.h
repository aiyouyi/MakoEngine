#ifndef SIMPLE_FACE_RECONSTRUCTOR_H_
#define SIMPLE_FACE_RECONSTRUCTOR_H_
#include "SimpleMesh.h"

namespace mt3dface
{ 
	

	/**
	 * @brief 基于106个landmark点和86个Vt点进行人脸点插值和构建简单的人脸网格和纹理映射的基础类
	 *
	 * 106个landmark点为人脸库直接输出。86个Vt点为标准人脸原始的Vt值并通过插值算法扩展为106个Vt点
	 * 和landmark点一一对应。再进一步基于106个landmark点和vt点插值出235个landmark点和vt点，这些
	 * 点分布于人脸的额头，左脸颊，右脸颊，脸轮廓，五官点。利用这些点和已提前设计好的三角网格构建简单
	 * 的人脸网格并完成纹理映射。
	 * 该人脸网格图如下所示
	 * @image html face_mesh.jpeg "人脸网格图" height=900px width=600px
	 */
	class SimpleFaceMesh:public SimpleMesh
	{
	public:
		SimpleFaceMesh() = default;
		~SimpleFaceMesh() override = default;
		/**
		 * @brief 初始化相关simple face mesh的数据
		 */
        void setMemberInit()override;
		/**
		* @brief 总入口函数
		* @param [in] pImagePoint 人脸库输出的106个landmark点
		* @param [in] fWidthImage 图像宽度
		* @param [in] fHeightImage 图像高度
		* @param [in] fFacePitchAngle 人脸俯仰角，单位为角度
		* @return true表示成功，false表示失败
		*/
		bool Run(const float *pImagePoint, const float fWidthImage, const float fHeightImage,const float fFacePitchAngle) override ;

	private:

		/**
		 * @brief 利用变换和归一化的人脸landmark点更新标准人脸Vt[0,16,32，51-68]点
		 * @param VtCoord 标准人脸Vt点
		 * @param ImagePointVtCoord 变换和归一化的人脸landmark点
		 * @param pSimpleFaceMeshVt 更新后的标准人脸Vt点
		 */
		void LocalVtUpdate(std::vector<Point2D>& VtCoord, std::vector<Point2D>& ImagePointVtCoord, float *pSimpleFaceMeshVt) override ;
		/**
		 * @brief 在不同人脸的姿态下利用landmark映射的标准人脸Vt[0,16,32，51-68]点会存在不准的情况，进行局部调整
		 * @param ImagePointVtCoord 变换和归一化的人脸landmark点
		 * @param pSimpleFaceMeshVt 局部调整后的标准人脸Vt点
		 */
		void LocalVtAdjust(std::vector<Point2D>& ImagePointVtCoord,float *pSimpleFaceMeshVt);
		/**
		* @brief 利用人脸landmark点更新人脸轮廓的Vt
		* @param [in,out] pSimpleFaceMeshVt simple face mesh的Vt值
		* @param [in] pImagePoint 106个landmark点
		* @param [in] numVt  simple face mesh的Vt数量
		* @return true表示成功，false表示失败
		*/
		bool FaceContourLineVtUpdate(float *pSimpleFaceMeshVt, float *pImagePoint, short3D numVt) override ;
		/**
		* @brief 创建simple face mesh
		* @param [in] pImagePoint 106个landmark点
		* @return true表示成功，false表示失败
		*/
		bool UpdateSimpleFaceMesh(const float* pImagePoint) override ;

	};

}

#endif
