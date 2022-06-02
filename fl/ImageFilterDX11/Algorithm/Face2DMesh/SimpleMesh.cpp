#include <cstdio>
#include <cassert>
#include <algorithm>
#include "SimpleMesh.h"

namespace mt3dface
{

    SimpleMesh::SimpleMesh()
    {

        m_pSimpleFaceMeshV= nullptr;
        m_pSimpleFaceMeshVt= nullptr;
        m_pSimpleFaceMeshTriIndex= nullptr;
        // size
        m_widthImage = 0.0f;
        m_heightImage = 0.0f;
        m_faceFrameX = 0.0f;
        m_faceFrameY = 0.0f;
        m_widthFace = 0.0f;
        m_heightFace = 0.0f;
        m_facePitchAngle = 0.0f;
        m_mouthLength = 0.0f;
        for (int i = 0; i < 8; i++)
        {
            m_IsNonLine[i] = true;
        }
    }

    SimpleMesh::~SimpleMesh()
    {
        SAFE_DELETE(m_pSimpleFaceMeshV);
        SAFE_DELETE(m_pSimpleFaceMeshVt);
        SAFE_DELETE(m_pSimpleFaceMeshTriIndex);
    }


    float* SimpleMesh::GetSimpleFaceMeshV()
    {
        return m_pSimpleFaceMeshV;
    }
    
    float* SimpleMesh::GetSimpleFaceMeshVt()
    {
        return m_pSimpleFaceMeshVt;

    }

    short3D* SimpleMesh::GetSimpleFaceMeshTriIndex()
    {
        return m_pSimpleFaceMeshTriIndex;
    }

    short3D SimpleMesh::GetNumOfSimpleFaceMeshV()
    {
        return m_numOfSimpleFaceMeshV;
    }
    short3D SimpleMesh::GetNumOfSimpleFaceMeshTri()
    {
        return m_numOfSimpleFaceMeshTri;
    }
	// 做位置的微调，让点更平滑
    void SimpleMesh::ModifyLandmark(const float *pImagePoint, float *pNewImagePoint)
    {
        memcpy(pNewImagePoint, pImagePoint, sizeof(float)*(m_numLandmark+12) * 2);
        Point2D point[3];
        // modify landmark
        short3D ModifyEyesLMIndex[] = { 51,52,53,53,54,55,61,62,63,63,64,65};
        for (int i = 0; i < 12; i += 3)
        {
            for (int j = 0; j < 3; j++)
            {
                point[j].x = pImagePoint[2 * ModifyEyesLMIndex[i+j]];
                point[j].y = pImagePoint[2 * ModifyEyesLMIndex[i+j] + 1];
            }

            if (point[1].x<point[0].x || point[1].x>point[2].x)
            {
                point[1].x = 0.5f*point[0].x + (1.f - 0.5f)*point[2].x;
                point[1].y = 0.6f*point[0].y + (1.f - 0.6f)*point[2].y;
            }

            pNewImagePoint[2 * ModifyEyesLMIndex[i + 1]] = point[1].x;
            pNewImagePoint[2 * ModifyEyesLMIndex[i + 1] + 1] = point[1].y;
        }
        // modify inner mouth landmark
        point[0].x = (pImagePoint[2 * 100] - m_faceFrameX) / m_widthFace;
        point[0].y = (pImagePoint[2 * 100 + 1] - m_faceFrameY) / m_heightFace;
        point[1].x = (pImagePoint[2 * 104] - m_faceFrameX) / m_widthFace;
        point[1].y = (pImagePoint[2 * 104 + 1] - m_faceFrameY) / m_heightFace;
        m_mouthLength = (point[0] - point[1]).length();
        short3D MouthLMIndex[] = { 98, 99,100,100,101,102,102,103,104,104,105,98,86,87,88,90,91,92,92,93,94,96,97,86 };
        for (int i = 0; i < 24; i += 3)
        {
            point[0].x = pImagePoint[2 * MouthLMIndex[i]];
            point[0].y = pImagePoint[2 * MouthLMIndex[i] + 1];
            point[1].x = pImagePoint[2 * MouthLMIndex[i + 1]];
            point[1].y = pImagePoint[2 * MouthLMIndex[i + 1] + 1];
            point[2].x = pImagePoint[2 * MouthLMIndex[i + 2]];
            point[2].y = pImagePoint[2 * MouthLMIndex[i + 2] + 1];
            //判断是用拉格朗日插值还是用线性插值
            Point2D Point[2];
            Point[0].x = (point[0].x - m_faceFrameX) / m_widthFace;
            Point[0].y = (point[0].y - m_faceFrameY) / m_heightFace;
            Point[1].x = (point[2].x - m_faceFrameX) / m_widthFace;
            Point[1].y = (point[2].y - m_faceFrameY) / m_heightFace;
            float Point1to2Length = (Point[0] - Point[1]).length();
            if (Point1to2Length > 0.12f)
                m_IsNonLine[i / 3] = true;
            else
                m_IsNonLine[i / 3] = false;
            if (point[1].x<point[0].x || point[1].x>point[2].x)
            {
                point[1].x = 0.5f*point[1].x + (1.f - 0.5f)*point[1].x;
                point[1].y = 0.6f*point[1].y + (1.f - 0.6f)*point[1].y;
            }
            if (!m_IsNonLine[i / 3])
            {
                pNewImagePoint[2 * MouthLMIndex[i + 1]] = point[1].x;
                pNewImagePoint[2 * MouthLMIndex[i + 1]+1] = point[1].y;
            }
        }
    }
    void SimpleMesh::CalFaceSize(const float* pImagePoint)
    {
        float minx = 100000, maxx = -100000;
        float miny = 100000, maxy = -100000;
        for (int i = 0; i < m_numLandmark; i++)
        {
            minx = (std::min) (minx, pImagePoint[2 * i]);
            maxx = (std::max)(maxx, pImagePoint[2 * i]);
            miny = (std::min)(miny, pImagePoint[2 * i + 1]);
            maxy = (std::max) (maxy, pImagePoint[2 * i + 1]);
        }
        float fFaceFrameWidth = maxx - minx;
        float fFaceFrameHeight = maxy - miny;
        if (fFaceFrameWidth > fFaceFrameHeight)
        {
            m_faceFrameX = minx;
            m_faceFrameY = miny;
            m_widthFace = fFaceFrameWidth;
            m_heightFace = fFaceFrameWidth;
        }
        else
        {
            m_faceFrameX = minx;
            m_faceFrameY = miny;
            m_widthFace = fFaceFrameHeight;
            m_heightFace = fFaceFrameHeight;
        }
    }

    void SimpleMesh::FacePointCoorTransform(const float *pImagePoint, float* pImagePointVtCoord)
    {
        std::vector<Point2D> CartesianVtCoord(m_nNumOfFacePt);
        for (int i = 0; i < m_nNumOfFacePt; i++)
        {
            CartesianVtCoord[i].x = (pImagePoint[2 * i] - m_faceFrameX) / m_widthFace;
            CartesianVtCoord[i].y = (pImagePoint[2 * i + 1] - m_faceFrameY) / m_heightFace;
        }

        //图像坐标系转笛卡尔坐标系
        for (int i = 0; i < m_nNumOfFacePt; i++)
        {
            CartesianVtCoord[i].x = CartesianVtCoord[i].x - 0.5f;
            CartesianVtCoord[i].y = -CartesianVtCoord[i].y + 0.5f;
        }

        //平移到原点，绕原点旋转，平移到原位置
        Point2D VtCoorDifferentValue = CartesianVtCoord[65] - CartesianVtCoord[51];
        float fAngle = atan2(VtCoorDifferentValue.y, VtCoorDifferentValue.x);
        Point2D CentreOf65And51 = CartesianVtCoord[74];
        for (int i = 0; i < m_nNumOfFacePt; i++)
        {
            CartesianVtCoord[i] = CartesianVtCoord[i] - CentreOf65And51;
        }
		// 顺时针旋转将主播脸摆正
        for (int i = 0; i < m_nNumOfFacePt; i++)
        {
            Point2D TemVtCoord = CartesianVtCoord[i];
            CartesianVtCoord[i].x = TemVtCoord.x*cos(fAngle) + TemVtCoord.y*sin(fAngle);
            CartesianVtCoord[i].y = -TemVtCoord.x*sin(fAngle) + TemVtCoord.y*cos(fAngle);
        }

        for (int i = 0; i < m_nNumOfFacePt; i++)
        {
            CartesianVtCoord[i] = CartesianVtCoord[i] + CentreOf65And51;
        }
        //笛卡尔坐标转换成图像坐标

        for (int i = 0; i < m_nNumOfFacePt; i++)
        {
            pImagePointVtCoord[2 * i] = CartesianVtCoord[i].x + 0.5f;
            pImagePointVtCoord[2 * i + 1] = -CartesianVtCoord[i].y + 0.5f;
        }
    }

    bool SimpleMesh::FaceCourtonLineVtInterpolation(float *pSimpleFaceMeshVt, short3D numVt)
    {
        if (pSimpleFaceMeshVt == nullptr)
        {
            return false;
        }
        std::vector<Point2D> VtCoord(numVt);
        for (int i = 0; i < numVt; i++)
        {
            VtCoord[i].x = pSimpleFaceMeshVt[2 * i];
            VtCoord[i].y = pSimpleFaceMeshVt[2 * i + 1];
        }

        std::vector<Point2D> InPoint;
        std::vector<Point2D> OuPoint;
        Point2D pt1; Point2D pt2;

        //////////////////////////////////////////////////////////////////////////
        //眼睛中间点
		Point2D Mid = (VtCoord[0] + VtCoord[16] + VtCoord[32]) / 3.0f;

        pt1 = MathUtil::getFeatureInterpolatValue(VtCoord[16], VtCoord[0], Mid, 0.838f);
        pt2 = MathUtil::getFeatureInterpolatValue(VtCoord[16], VtCoord[32], Mid, 0.838f);
        InPoint.clear();
        OuPoint.clear();
        InPoint.push_back(VtCoord[0]);
        InPoint.push_back(pt1);
        InPoint.push_back(VtCoord[16]);
        MathUtil::LagrangePolyfit(InPoint, OuPoint, 17);
        short3D nCurrentIndex = 0;
        for (int i = 0; i < 17; i++)
        {
            pSimpleFaceMeshVt[2 * nCurrentIndex] = OuPoint[i].x;
            pSimpleFaceMeshVt[2 * nCurrentIndex + 1] = OuPoint[i].y;
            nCurrentIndex++;
        }


        InPoint.clear();
        OuPoint.clear();
        InPoint.push_back(VtCoord[32]);
        InPoint.push_back(pt2);
        InPoint.push_back(VtCoord[16]);
        MathUtil::LagrangePolyfit(InPoint, OuPoint, 17);
        for (int i = 15; i >= 0; i--)
        {
            pSimpleFaceMeshVt[2 * nCurrentIndex] = OuPoint[i].x;
            pSimpleFaceMeshVt[2 * nCurrentIndex + 1] = OuPoint[i].y;
            nCurrentIndex++;
        }
        return true;

    }


}

