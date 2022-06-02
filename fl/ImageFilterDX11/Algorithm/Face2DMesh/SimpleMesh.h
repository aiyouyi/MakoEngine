#ifndef SIMPLE_MESH_H
#define SIMPLE_MESH_H
#include "FaceInterlation.h"
namespace mt3dface
{

    class SimpleMesh
    {
    public:

        SimpleMesh();
        virtual ~SimpleMesh();
        // 初始化相关simple mesh的数据
        virtual void setMemberInit()=0;
        // 输出相关数据
        /**
         * @brief 输出simple face mesh的V
         * @return m_pSimpleFaceMeshV指针
         */
        float* GetSimpleFaceMeshV();
        /**
         * @brief 输出simple face mesh的Vt
         * @return m_pSimpleFaceMeshVt指针
         */
        float* GetSimpleFaceMeshVt();
        /**
         * @brief 输出simple face mesh的面
         * @return m_pSimpleFaceMeshTriIndex指针
         */
        short3D* GetSimpleFaceMeshTriIndex();
        /**
         * @brief 输出simple face mesh的V数量
         * @return m_pSimpleFaceMeshTriIndex的值
         */
        short3D GetNumOfSimpleFaceMeshV();
        /**
         * @brief 输出simple face mesh的面数量
         * @return m_numOfSimpleFaceMeshTri的值
         */
        short3D GetNumOfSimpleFaceMeshTri();
        /**
         * @brief 总入口函数
         * @param [in] pImagePoint 人脸库输出的106个landmark点
         * @param [in] fWidthImage 图像宽度
         * @param [in] fHeightImage 图像高度
         * @param [in] fFacePitchAngle 人脸俯仰角，单位为角度
         * @return true表示成功，false表示失败
         */
         virtual bool Run(const float *pImagePoint, const float fWidthImage, const float fHeightImage,const float fFacePitchAngle)=0;
    protected:

        /**
        * @brief 利用变换和归一化的人脸landmark点更新标准人脸Vt[0,16,32，51-68]点
        * @param VtCoord 标准人脸Vt点
        * @param ImagePointVtCoord 变换和归一化的人脸landmark点
        * @param pSimpleFaceMeshVt 更新后的标准人脸Vt点
        */
        virtual void LocalVtUpdate(std::vector<Point2D>& VtCoord, std::vector<Point2D>& ImagePointVtCoord, float *pSimpleFaceMeshVt)=0;
        /**
         * @brief 利用人脸landmark点更新人脸轮廓的Vt
         * @param [in,out] pSimpleFaceMeshVt simple face mesh的Vt值
         * @param [in] pImagePoint 106个landmark点
         * @param [in] numVt  simple face mesh的Vt数量
         * @return true表示成功，false表示失败
         */
        virtual bool FaceContourLineVtUpdate(float *pSimpleFaceMeshVt, float *pImagePoint, short3D numVt)=0;
        /**
         * @brief 创建simple face mesh
         * @param [in] pImagePoint 106个landmark点
         * @return true表示成功，false表示失败
         */
        virtual bool UpdateSimpleFaceMesh(const float* pImagePoint)=0;

    protected:
        /**
		 * @brief 利用106个人脸landmark点插值出新的landmark个点
		 * @param [in] pImagePoint 原始的106个landmark点
		 * @param [out] pNewImagePoint  校正后106个landmark点
		 * @return true表示成功，false表示失败
		 */
         void ModifyLandmark(const float *pImagePoint, float *pNewImagePoint);
        /**
		 * @brief 利用人脸landmark点重新计算人脸框大小和原点
		 * @param [in]pImagePoint 原始的106个landmark点
		 */
        void CalFaceSize(const float* pImagePoint);
        /**
         * @brief 对人脸landmark点进行坐标变换和归一化
         * @param [in] pImagePoint 插值后的人脸landmark点
         * @param [out] pImagePointVtCoord 变换和归一化的人脸landmark点
         */
        void FacePointCoorTransform(const float *pImagePoint, float* pImagePointVtCoord);
        /**
         * @brief 人脸轮廓的Vt插值
         * @param [in,out] pSimpleFaceMeshVt simple face mesh的Vt值
         * @param [in] numVt  simple face mesh的Vt数量
         * @return true表示成功，false表示失败
         */
        bool FaceCourtonLineVtInterpolation(float *pSimpleFaceMeshVt, short3D numVt);
		
    protected:
        // size
        float m_widthImage;
        float m_heightImage;
        float m_faceFrameX;
        float m_faceFrameY;
        float m_widthFace;
        float m_heightFace;
        float m_facePitchAngle;
        float m_mouthLength;

		//输入标记点信息
		short3D m_numLandmark;

        // simple face mesh基本信息
		short3D m_nNumOfFacePt;
        short3D m_numOfSimpleFaceMeshV;
        short3D m_numOfSimpleFaceMeshTri;
        short3D* m_pSimpleFaceMeshTriIndex;
        float* m_pSimpleFaceMeshV;
        float* m_pSimpleFaceMeshVt;
        bool m_IsNonLine[8];

    };

}



#endif //SIMPLE_MESH_H
