#include <iostream>
#include "FaceInterlation.h"
namespace mt3dface
{

	/////////////////////////////////////////////////////
	//定义MathUtil类

	Point2D MathUtil::getFeatureInterpolatValue(Point2D pt1, Point2D pt2, Point2D M, float u)
	{
		Point2D pt1_M = u*pt1 + (1 - u)*M;
		Point2D pt2_M = u*pt2 + (1 - u)*M;
		Point2D direction = pt2_M - M;
		return  direction + pt1_M;
	}

	void MathUtil::LagrangePolyfit(std::vector<Point2D> polyfitIn, std::vector<Point2D>& polyfitOut, int ptnum)
	{
		polyfitOut.clear();
		Point2D firstPt = polyfitIn.at(0);
		Point2D lastPt = polyfitIn.at(polyfitIn.size() - 1);
		Point2D vectorL2R;//方向向量
		vectorL2R.x = lastPt.x - firstPt.x;
		vectorL2R.y = lastPt.y - firstPt.y;
		float ss = vectorL2R.x*vectorL2R.x + vectorL2R.y* vectorL2R.y;
		float normValue = sqrt(ss);
		if (fabs(normValue) < 0.001f)
		{
			normValue = 0.001f;
		}
		//顺时针旋转
		float sinVal = -vectorL2R.y / normValue;
		float cosVal = vectorL2R.x / normValue;

		int Innum = polyfitIn.size();
		std::vector<Point2D> tmpInVec;
		Point2D tmpPt;

		for (int i = 0; i<Innum; i++)
		{
			polyfitIn[i].x -= firstPt.x;
			polyfitIn[i].y -= firstPt.y;
			tmpPt.x = cosVal*polyfitIn[i].x - sinVal*polyfitIn[i].y;
			tmpPt.y = cosVal*polyfitIn[i].y + sinVal*polyfitIn[i].x;
			tmpInVec.push_back(tmpPt);
		}

		std::vector<Point2D> tmpOutVec;

		float gap = (tmpInVec.back().x - tmpInVec[0].x) / (ptnum - 1);
		//设定拟合的横坐标值
		for (int i = 0; i<ptnum; i++)
		{
			tmpPt.x = tmpInVec[0].x + gap*i;
			tmpPt.y = -1;
			tmpOutVec.push_back(tmpPt);
		}
		//算出每一个拟合值
		for (int i = 0; i<ptnum; i++)
		{
			float tmp_y = 0.0f;
			for (int j = 0; j<Innum; j++)
			{
				float Ltmp = 1.0f;
				for (int k = 0; k<Innum; k++)
				{
					if (k == j)
					{
						continue;
					}
					float fDen = (tmpInVec[j].x - tmpInVec[k].x);
					if (fabs(fDen) < 0.0001f)
					{
						fDen = 0.0001f;
					}
					Ltmp = Ltmp*(tmpOutVec[i].x - tmpInVec[k].x) / fDen;
				}
				Ltmp *= tmpInVec[j].y;
				tmp_y += Ltmp;
			}
			tmpOutVec[i].y = tmp_y;
		}

		//逆时针旋转
		sinVal = -sinVal;
		for (unsigned int i = 0; i<tmpOutVec.size(); i++)
		{
			tmpPt.x = cosVal*tmpOutVec[i].x - sinVal*tmpOutVec[i].y;
			tmpPt.y = cosVal*tmpOutVec[i].y + sinVal*tmpOutVec[i].x;
			tmpPt.x += firstPt.x;
			tmpPt.y += firstPt.y;
			polyfitOut.push_back(tmpPt);
		}
	}

	void MathUtil::Intersection(Point2D Point1, Point2D Point2, Point2D Point3, Point2D Point4, Point2D& OutPoint)
	{
		float a = Point2.x - Point1.x;
		float b = Point3.x - Point4.x;
		float c = Point2.y - Point1.y;
		float d = Point3.y - Point4.y;
		float g = Point3.x - Point1.x;
		float h = Point3.y - Point1.y;
		float f = a*d - b*c;  //行列式
		float t = (d*g - b*h) / f;
		float s = (-c*g + a*h) / f;
	
		OutPoint.x = Point1.x + t*(Point2.x - Point1.x);
		OutPoint.y = Point1.y + t*(Point2.y - Point1.y);		
	}


	bool FaceInterlation::RunFacePointInterpolation(const float *pFacePoint106, int numLandmarkPoint, int numIntelatPoint,bool *NonLineFlag, float ForeheadWeight, float ForeheadCTWeight,bool VtFlag, float *pImagePoint)
	{
		if (pFacePoint106 == nullptr)
		{
			return false;
		}
		std::vector<Point2D> FacePoint106(numLandmarkPoint);
		for (int i = 0; i < numLandmarkPoint; i++)
		{
			FacePoint106[i].x = pFacePoint106[2 * i];
			FacePoint106[i].y = pFacePoint106[2 * i + 1];
		}
		
		std::vector<Point2D> FacePoint(numIntelatPoint);
		int nCurrentIndex = 0;
		bool InnerMouthFlag[4], OutMouthFlag[4];
		for (int i = 0; i < 4; i++)
		{
			InnerMouthFlag[i] = NonLineFlag[i];
			OutMouthFlag[i] = NonLineFlag[i + 4];
		}
		nCurrentIndex = CalFaceLandmarkPoint(FacePoint106.data(), numLandmarkPoint, nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalForeheadPoint(ForeheadWeight, ForeheadCTWeight, nCurrentIndex,true, VtFlag, FacePoint.data());
		nCurrentIndex = CalLeftCheckPoint(nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalRightCheckPoint(nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalEyePoint(nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalOutMouthPoint(OutMouthFlag,nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalInnerMouthPoint(InnerMouthFlag,nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalChainPoint(nCurrentIndex, FacePoint.data());
		if(nCurrentIndex== numIntelatPoint)
			for (int i = 0; i < numIntelatPoint; i++)
			{
				pImagePoint[2 * i] = FacePoint[i].x;
				pImagePoint[2 * i+1] = FacePoint[i].y;
			}
		return true;
	}

	bool FaceInterlation::RunFaceAndBackPointInterpolation(const float *pFacePoint106, int numLandmarkPoint, int numIntelatPoint, bool *NonLineFlag,
		float ForeheadWeight, float ForeheadCTWeight, float PitchAngle, float FaceHight, bool VtFlag, float *pImagePoint)
	{
		if (pFacePoint106 == nullptr)
		{
			return false;
		}
		std::vector<Point2D> FacePoint106(numLandmarkPoint);
		for (int i = 0; i < numLandmarkPoint; i++)
		{
			FacePoint106[i].x = pFacePoint106[2 * i];
			FacePoint106[i].y = pFacePoint106[2 * i + 1];
		}

		std::vector<Point2D> FacePoint(numIntelatPoint);
		int nCurrentIndex = 0;
		bool InnerMouthFlag[4], OutMouthFlag[4];
		for (int i = 0; i < 4; i++)
		{
			InnerMouthFlag[i] = NonLineFlag[i];
			OutMouthFlag[i] = NonLineFlag[i + 4];
		}
		nCurrentIndex = CalFaceLandmarkPoint(FacePoint106.data(), numLandmarkPoint, nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalForeheadPoint(ForeheadWeight, ForeheadCTWeight, nCurrentIndex,true, VtFlag , FacePoint.data());
		nCurrentIndex = CalLeftCheckPoint(nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalRightCheckPoint(nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalEyePoint(nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalOutMouthPoint(OutMouthFlag,nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalInnerMouthPoint(InnerMouthFlag,nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalChainPoint(nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalBackgroundPoint(PitchAngle,nCurrentIndex, FacePoint.data());
		if (nCurrentIndex == numIntelatPoint)
		{
			for (int i = 0; i < numIntelatPoint; i++)
			{
				pImagePoint[2 * i] = FacePoint[i].x;
				pImagePoint[2 * i + 1] = FacePoint[i].y;
			}
		}
		return true;
	}

	bool FaceInterlation::RunFacePointVtInterpolation(const float *pFacePointVt,int numIntelatPoint, float ForeheadCTWeight, bool VtFlag, float *pImagePointVt)
	{
		if (pFacePointVt == nullptr)
		{
			return false;
		}
		std::vector<Point2D> FacePoint(numIntelatPoint);
		for (int i = 0; i < numIntelatPoint; i++)

		{
			FacePoint[i].x = pFacePointVt[2 * i];
			FacePoint[i].y = pFacePointVt[2 * i + 1];
		}
		int nCurrentIndex = 0;
		//0-105不变
		nCurrentIndex = 106;
		nCurrentIndex = CalForeheadPoint(0.0f, ForeheadCTWeight, nCurrentIndex,false, VtFlag , FacePoint.data());
		nCurrentIndex = CalLeftCheckPoint(nCurrentIndex, FacePoint.data());
		nCurrentIndex = CalRightCheckPoint(nCurrentIndex, FacePoint.data());
		//171-226不变
		nCurrentIndex = 227;
		nCurrentIndex = CalChainPoint(nCurrentIndex, FacePoint.data());
		//235-260不变
		nCurrentIndex = 261;
		if (nCurrentIndex == numIntelatPoint)
		{
			for (int i = 0; i < numIntelatPoint; i++)
			{
				pImagePointVt[2 * i] = FacePoint[i].x;
				pImagePointVt[2 * i + 1] = FacePoint[i].y;
			}
		}
		return true;

	}
	// 原有的106点保持原有值
	int FaceInterlation::CalFaceLandmarkPoint(const Point2D* pFacePoint106, int numLandmarkPoint, int nCurrentIndex, Point2D* pFacePoint)
	{
		for (int i = 0; i < numLandmarkPoint; i++)
		{
			pFacePoint[i] = pFacePoint106[i];
		}
		nCurrentIndex += numLandmarkPoint;
		return nCurrentIndex;
	}

    int FaceInterlation::CalForeheadPoint(float ForeheadWeight, float ForeheadCTWeight,int nCurrentIndex, bool ChooseFaorVtFlag, bool VtFlag, Point2D* pFacePoint)
    {
        std::vector<Point2D> InPoint;
        std::vector<Point2D> OuPoint;
        Point2D pt1, pt2, pt;
        float fRadio = 0.0f;
        //眼睛中间点
        Point2D Mid = pFacePoint[71];
        //额头最顶点
		if (ChooseFaorVtFlag)
		{
			fRadio = ForeheadWeight;
			// pt = Mid + fRadio(眼睛中点-鼻子底部中点)
			pt = (1.0f + fRadio) * Mid - fRadio * pFacePoint[80];

		}
		else
		{
			pt = pFacePoint[106];
		}
        

        //额头插值
        //106-109
        Point2D PointEnd = pFacePoint[0];
        pt1 = MathUtil::getFeatureInterpolatValue(PointEnd, pt, Mid, ForeheadCTWeight);
        InPoint.clear();
        OuPoint.clear();
        InPoint.push_back(pt);
        InPoint.push_back(pt1);
        InPoint.push_back(pFacePoint[0]);
        MathUtil::LagrangePolyfit(InPoint, OuPoint, 5);
        for (int i = 0; i < 4; i++)
        {
            pFacePoint[nCurrentIndex] = OuPoint[i];
            nCurrentIndex++;
        }
		
        //110-112
        PointEnd = pFacePoint[32];
        pt2 = MathUtil::getFeatureInterpolatValue(PointEnd, pt, Mid, ForeheadCTWeight);
        InPoint.clear();
        OuPoint.clear();
        InPoint.push_back(pt);
        InPoint.push_back(pt2);
        InPoint.push_back(pFacePoint[32]);
        MathUtil::LagrangePolyfit(InPoint, OuPoint, 5);
        for (int i = 1; i < 4; i++)
        {
            pFacePoint[nCurrentIndex] = OuPoint[i];
            nCurrentIndex++;
        }

		if (VtFlag)
		{
			pFacePoint[108].x = pFacePoint[108].x - 0.01f;
			pFacePoint[109].x = pFacePoint[109].x - 0.030f;
			pFacePoint[111].x = pFacePoint[111].x + 0.01f;
			pFacePoint[112].x = pFacePoint[112].x + 0.030f;
		}

        //113
        pt = (pFacePoint[109] + pFacePoint[0]) / 2.0f;
        fRadio = 0.6f;
        pt1 = (1.0f - fRadio) *  pt + fRadio * pFacePoint[33];
        pFacePoint[nCurrentIndex] = pt1;
        nCurrentIndex++;
        //114
        pt = pFacePoint[109];
        fRadio = 0.25f;
        pt1 = (1.0f - fRadio) *  pt + fRadio * pFacePoint[34];
        pFacePoint[nCurrentIndex] = pt1;
        nCurrentIndex++;
        //115
        pt = pFacePoint[108];
        pt1 = pFacePoint[34];
        fRadio = 0.5f;
        pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
        pFacePoint[nCurrentIndex] = pt2;
        nCurrentIndex++;
        //116-117
        pt = pFacePoint[107];
        pt1 = pFacePoint[36];
        fRadio = 0.33f;
        for (int i = 1; i<3; i++)
        {
            pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
            pFacePoint[nCurrentIndex] = pt2;
            nCurrentIndex++;
        }
        //118-119
        pt = pFacePoint[106];
        pt1 = pFacePoint[37];
        fRadio = 0.33f;
        for (int i = 1; i<3; i++)
        {
            pt2 = (1.0f - i*fRadio) * pt + i*fRadio * pt1;
            pFacePoint[nCurrentIndex] = pt2;
            nCurrentIndex++;
        }
        //120-121
        pt1 = pFacePoint[42];
        fRadio = 0.33f;
        for (int i = 1; i<3; i++)
        {
            pt2 = (1.0f - i*fRadio) * pt + i*fRadio * pt1;
            pFacePoint[nCurrentIndex] = pt2;
            nCurrentIndex++;
        }
        //122-123
        pt = pFacePoint[110];
        pt1 = pFacePoint[43];
        fRadio = 0.33f;
        for (int i = 1; i<3; i++)
        {
            pt2 = (1.0f - i*fRadio) * pt + i*fRadio * pt1;
            pFacePoint[nCurrentIndex] = pt2;
            nCurrentIndex++;
        }
        //124
        pt = pFacePoint[111];
        pt1 = pFacePoint[45];
        fRadio = 0.5f;
        pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
        pFacePoint[nCurrentIndex] = pt2;
        nCurrentIndex++;
        //125
        pt = pFacePoint[112];
        pt1 = pFacePoint[45];
        fRadio = 0.5f;
        pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
        pFacePoint[nCurrentIndex] = pt2;
        nCurrentIndex++;

        //126
        pt = (pFacePoint[112] + pFacePoint[32]) / 2.0f;
        pt1 = pFacePoint[46];
        fRadio = 0.6f;
        pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
        pFacePoint[nCurrentIndex] = pt2;
        nCurrentIndex++;
        //127
        pt = pFacePoint[51];
        pt1 = pFacePoint[0];
        fRadio = 0.5f;
        pt2= (1.0f - fRadio) *  pt + fRadio * pt1;
        pFacePoint[nCurrentIndex] = pt2;
        nCurrentIndex++;
        //128
        pt = pFacePoint[65];
        pt1 = pFacePoint[32];
        fRadio = 0.5f;
        pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
        pFacePoint[nCurrentIndex] = pt2;
        nCurrentIndex++;
        return nCurrentIndex;
    }


	int FaceInterlation::CalLeftCheckPoint(int nCurrentIndex, Point2D* pFacePoint)
	{
		Point2D pt1, pt2, pt;
		float fRadio = 0.0f;
		//129
		pt = pFacePoint[72];
		pt1 = pFacePoint[76];
		fRadio = 0.5f;
		pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
		pFacePoint[nCurrentIndex] = pt2;
		nCurrentIndex++;
		//130
		pt = pFacePoint[55];
		pt1 = pFacePoint[129];
		fRadio = 0.5f;
		pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
		pFacePoint[nCurrentIndex] = pt2;
		nCurrentIndex++;
		//131-132
		pt = pFacePoint[57];
		pt1 = pFacePoint[76];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//133-134
		pt = pFacePoint[51];
		pt1 = pFacePoint[77];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//135-136
		pt = pFacePoint[1];
		pt1 = pFacePoint[133];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//137-138
		pt = pFacePoint[3];
		pt1 = pFacePoint[134];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//139-141
		pt = pFacePoint[4];
		pt1 = pFacePoint[77];
		fRadio = 0.25f;
		for (int i = 1; i<4; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//142-144
		pt = pFacePoint[6];
		pt1 = (pFacePoint[87] + pFacePoint[78]) / 2.0f;
		fRadio = 0.33f;
		for (int i = 1; i<4; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//145-146
		pt = pFacePoint[86];
		pt1 = pFacePoint[11];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//147-148
		pt = pFacePoint[7];
		pt1 = pFacePoint[145] * (1.0f - 0.6f) + 0.6f * pFacePoint[143];
		fRadio = 0.45f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//149
		pt = pFacePoint[8];
		pt1 = pFacePoint[145];
		fRadio = 0.5f;
		pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
		pFacePoint[nCurrentIndex] = pt2;
		nCurrentIndex++;
		return nCurrentIndex;

	}
	int FaceInterlation::CalRightCheckPoint(int nCurrentIndex, Point2D* pFacePoint)
	{
		Point2D pt1, pt2, pt;
		float fRadio = 0.0f;
		//150
		pt = pFacePoint[72];
		pt1 = pFacePoint[84];
		fRadio = 0.5f;
		pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
		pFacePoint[nCurrentIndex] = pt2;
		nCurrentIndex++;
		//151
		pt = pFacePoint[61];
		pt1 = pFacePoint[150];
		fRadio = 0.5f;
		pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
		pFacePoint[nCurrentIndex] = pt2;
		nCurrentIndex++;
		//152-153
		pt = pFacePoint[67];
		pt1 = pFacePoint[84];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//154-155
		pt = pFacePoint[65];
		pt1 = pFacePoint[83];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//156-157
		pt = pFacePoint[31];
		pt1 = pFacePoint[154];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//158-159
		pt = pFacePoint[29];
		pt1 = pFacePoint[155];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//160-162
		pt = pFacePoint[28];
		pt1 = pFacePoint[83];
		fRadio = 0.25f;
		for (int i = 1; i<4; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//163-165
		pt = pFacePoint[26];
		pt1 = (pFacePoint[91] + pFacePoint[82]) / 2.0f;
		fRadio = 0.33f;
		for (int i = 1; i<4; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//166-167
		pt = pFacePoint[92];
		pt1 = pFacePoint[21];
		fRadio = 0.33f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//168-169
		pt = pFacePoint[25];
		pt1 = pFacePoint[166] * (1.0f - 0.6f) + 0.6f * pFacePoint[164];
		fRadio = 0.45f;
		for (int i = 1; i<3; i++)
		{
			pt2 = pt * (1.0f - i*fRadio) + i*fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		//170
		pt = pFacePoint[24];
		pt1 = pFacePoint[166];
		fRadio = 0.5f;
		pt2 = (1.0f - fRadio) *  pt + fRadio * pt1;
		pFacePoint[nCurrentIndex] = pt2;
		nCurrentIndex++;
		return nCurrentIndex;
	}

	
	int FaceInterlation::CalEyePoint(int nCurrentIndex, Point2D* pFacePoint)
	{
		std::vector<Point2D> InPoint;
		std::vector<Point2D> OuPoint;
		//左眼171-178
		for (int i = 51; i < 55; i += 2)
		{
			InPoint.clear();
			OuPoint.clear();
			InPoint.push_back(pFacePoint[i]);
			InPoint.push_back(pFacePoint[i + 1]);
			InPoint.push_back(pFacePoint[i + 2]);
			MathUtil::LagrangePolyfit(InPoint, OuPoint, 7);
			for (int j = 1; j<3; j++)
			{
				pFacePoint[nCurrentIndex] = OuPoint[j];
				nCurrentIndex++;
			}
			for (int j = 4; j<6; j++)
			{
				pFacePoint[nCurrentIndex] = OuPoint[j];
				nCurrentIndex++;
			}
		}
		//右眼180-186
		for (int i = 61; i < 65; i += 2)
		{
			InPoint.clear();
			OuPoint.clear();
			InPoint.push_back(pFacePoint[i]);
			InPoint.push_back(pFacePoint[i + 1]);
			InPoint.push_back(pFacePoint[i + 2]);
			MathUtil::LagrangePolyfit(InPoint, OuPoint, 7);
			for (int j = 1; j<3; j++)
			{
				pFacePoint[nCurrentIndex] = OuPoint[j];
				nCurrentIndex++;
			}
			for (int j = 4; j<6; j++)
			{
				pFacePoint[nCurrentIndex] = OuPoint[j];
				nCurrentIndex++;
			}
		}
		return nCurrentIndex;
	}
	
	

	int FaceInterlation::CalOutMouthPoint(bool *ModefyPointFlag,int nCurrentIndex, Point2D* pFacePoint)
	{
		std::vector<Point2D> InPoint;
		std::vector<Point2D> OuPoint;
		Point2D pt1, pt2, pt;
		float fRadio = 0.0f;
		//187-210
		for (int i = 86; i < 98; i += 2)
		{
			InPoint.clear();
			OuPoint.clear();
			if (i != 96)
			{
				InPoint.push_back(pFacePoint[i]);
				InPoint.push_back(pFacePoint[i + 1]);
				InPoint.push_back(pFacePoint[i + 2]);
			}
			else
			{
				InPoint.push_back(pFacePoint[i]);
				InPoint.push_back(pFacePoint[i + 1]);
				InPoint.push_back(pFacePoint[86]);
			}
			MathUtil::LagrangePolyfit(InPoint, OuPoint, 7);
			for (int j = 1; j<3; j++)
			{
				pFacePoint[nCurrentIndex] = OuPoint[j];
				nCurrentIndex++;
			}
			for (int j = 4; j<6; j++)
			{
				pFacePoint[nCurrentIndex] = OuPoint[j];
				nCurrentIndex++;
			}
		}
		//修正嘴巴外轮廓的LM
		short3D MouthOutIndex[] = { 86,87,88,90,91,92,92,93,94,96,97,86 };
		short3D MouthOutILIndex[] = { 187,188,189,190,195,196,197,198,199,200,201,202,207,208,209,210 };
		for (int i = 0; i < 4; i++)
		{
			if (!ModefyPointFlag[i])
			{
				pt = pFacePoint[MouthOutIndex[(i * 3) + 0]];
				pt1 = pFacePoint[MouthOutIndex[(i * 3) + 1]];
				fRadio = 0.33f;
				for (int j = 1; j<3; j++)
				{
					pt2 = (1 - j*fRadio)*pt + j*fRadio*pt1;
					pFacePoint[MouthOutILIndex[i * 4 + (j - 1)]] = pt2;
				}
				pt = pFacePoint[MouthOutIndex[(i * 3) + 1]];
				pt1 = pFacePoint[MouthOutIndex[(i * 3) + 2]];
				for (int j = 1; j<3; j++)
				{
					pt2 = (1 - j*fRadio)*pt + j*fRadio*pt1;
					pFacePoint[MouthOutILIndex[i * 4 + (j + 1)]] = pt2;				
				}
			}
		}
		return nCurrentIndex;
	}
	int FaceInterlation::CalInnerMouthPoint(bool *ModefyPointFlag,int nCurrentIndex, Point2D* pFacePoint)
	{
		//211-226
		std::vector<Point2D> InPoint;
		std::vector<Point2D> OuPoint;
		Point2D pt1, pt2, pt;
		float fRadio = 0.0f;
		short3D PiontIndex[] = { 98,99,100,100,101,102,102,103,104,104,105,98 };
		for (int i = 0; i < 12; i += 3)
		{

			//选择插值方式
			if (ModefyPointFlag[i/3])//拉格朗日插值
			{
				InPoint.clear();
				OuPoint.clear();

				InPoint.push_back(pFacePoint[PiontIndex[i]]);
				InPoint.push_back(pFacePoint[PiontIndex[i + 1]]);
				InPoint.push_back(pFacePoint[PiontIndex[i + 2]]);

				MathUtil::LagrangePolyfit(InPoint, OuPoint, 7);
				if (PiontIndex[i] == 98 || PiontIndex[i] == 102)
				{
					for (int j = 1; j<2; j++)
					{
						pFacePoint[nCurrentIndex] = OuPoint[j];
						nCurrentIndex++;
					}
					for (int j = 3; j<6; j++)
					{
						pFacePoint[nCurrentIndex] = OuPoint[j];
						nCurrentIndex++;
					}
				}
				else
				{
					for (int j = 1; j<4; j++)
					{
						pFacePoint[nCurrentIndex] = OuPoint[j];
						nCurrentIndex++;
					}
					for (int j = 5; j<6; j++)
					{
						pFacePoint[nCurrentIndex] = OuPoint[j];
						nCurrentIndex++;
					}
				}
			}
			else//线性插值
			{
				if (PiontIndex[i] == 98 || PiontIndex[i] == 102)
				{
					pt = pFacePoint[PiontIndex[i]];
					pt1 = pFacePoint[PiontIndex[i + 1]];
					fRadio = 0.5f;
					pt2 = (1 - fRadio)*pt + fRadio*pt1;
					pFacePoint[nCurrentIndex] = pt2;
					nCurrentIndex++;

					pt = pFacePoint[PiontIndex[i + 1]];
					pt1 = pFacePoint[PiontIndex[i + 2]];
					fRadio = 0.25f;
					for (int j = 1; j<4; j++)
					{
						pt2 = (1 - j*fRadio)*pt + j*fRadio*pt1;
						pFacePoint[nCurrentIndex] = pt2;
						nCurrentIndex++;
					}

				}
				else
				{
					pt = pFacePoint[PiontIndex[i]];
					pt1 = pFacePoint[PiontIndex[i + 1]];
					fRadio = 0.25f;
					for (int j = 1; j<4; j++)
					{
						pt2 = (1 - j*fRadio)*pt + j*fRadio*pt1;
						pFacePoint[nCurrentIndex] = pt2;
						nCurrentIndex++;
					}
					pt = pFacePoint[PiontIndex[i + 1]];
					pt1 = pFacePoint[PiontIndex[i + 2]];
					fRadio = 0.5f;
					pt2 = (1 - fRadio)*pt + fRadio*pt1;
					pFacePoint[nCurrentIndex] = pt2;
					nCurrentIndex++;
				}
			}
		}
		return nCurrentIndex;
	}
	
	int FaceInterlation::CalChainPoint(int nCurrentIndex, Point2D* pFacePoint)
	{
		Point2D pt1, pt2, pt;
		float fRadio = 0.0f;
		//227-229
		fRadio = 0.4f;
		pt1 = (1.0f - fRadio) * pFacePoint[80] + fRadio * pFacePoint[89];
		pt = pFacePoint[144];
		fRadio = 0.5f;
		pt2 = (1.0f - fRadio) * pt + fRadio * pt1;
		pFacePoint[nCurrentIndex] = pt2;
		nCurrentIndex++;
		pFacePoint[nCurrentIndex] = pt1;
		nCurrentIndex++;
		pt = pFacePoint[165];
		pt2 = (1.0f - fRadio) * pt + fRadio * pt1;
		pFacePoint[nCurrentIndex] = pt2;
		nCurrentIndex++;
		//230-234
		int MouthCountorIndex[] = { 208,206,95,203,201 };
		int FaceCountorIndex[] = { 12,14,16,18,20 };
		for (int i = 0; i < 5; i++)
		{
			fRadio = 0.5f;
			pt = pFacePoint[MouthCountorIndex[i]];
			pt1 = pFacePoint[FaceCountorIndex[i]];
			pt2 = (1.0f - fRadio) * pt + fRadio * pt1;
			pFacePoint[nCurrentIndex] = pt2;
			nCurrentIndex++;
		}
		return nCurrentIndex;
	}

	int FaceInterlation::CalBackgroundPoint(float PitchAngle,int nCurrentIndex, Point2D* pFacePoint)
	{
		Point2D pt1,pt2,InterPoint, ParallelDirect;;
		int PauseIndex = 0;
		float fRadio_array[] = { 0.7f,0.75f,0.70f,0.50f,0.75f,0.70f,0.50f };
		//眼睛中间点
		Point2D Mid = pFacePoint[71];
		for (int i = 106; i <= 112; i++)
		{
			pt1 = (1.0f + fRadio_array[i - 106]) * pFacePoint[i] - fRadio_array[i - 106] * Mid;
			pFacePoint[nCurrentIndex] = pt1;
			nCurrentIndex++;
		}
		
		//抬头时往下拉伸人脸点并重新更新
		float fRadio = 0.0f;
		if (PitchAngle <= 0.0f)
		{
			if (PitchAngle < -45.0f)
			{
				PitchAngle = -45.0f;
			}
			PauseIndex = nCurrentIndex;
			for (int i = 0; i <= 15; i++)
			{
				fRadio = 0.011f * (-PitchAngle);				
				MathUtil::Intersection(pFacePoint[i], pFacePoint[32 - i], pFacePoint[16],pFacePoint[71], InterPoint);
				ParallelDirect = pFacePoint[16] - InterPoint;
				pFacePoint[i] = pFacePoint[i] + fRadio * ParallelDirect;
				pFacePoint[32 - i] = pFacePoint[32 - i] + fRadio * ParallelDirect;				
			}
			nCurrentIndex = 129;
			nCurrentIndex = CalLeftCheckPoint(nCurrentIndex, pFacePoint);
			CalRightCheckPoint(nCurrentIndex, pFacePoint);
			nCurrentIndex = 227;
			CalChainPoint(nCurrentIndex, pFacePoint);
			nCurrentIndex = PauseIndex;
		}
			
		fRadio = 0.4f;
		Mid = (pFacePoint[72] + pFacePoint[73]) / 2.0f;
		for (int i = 0; i <= 32; i += 2)
		{
			pt1 = (1.0f + fRadio) * pFacePoint[i] - fRadio * Mid;
			pFacePoint[nCurrentIndex] = pt1;
			nCurrentIndex++;
		}

		return nCurrentIndex;
	}


	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	//眼瞳分层插值
	bool EyePupilInterlation::RunEyePupilInterlation(const float * pFacePoint, int numLandmarkPoint, int numIntelatPoint,float *pPuiplePoint)
	{
		if (pFacePoint == nullptr)
		{
			return false;
		}
		std::vector<Point2D> FacePoint(numLandmarkPoint);
		for (int i = 0; i < numLandmarkPoint; i++)
		{
			FacePoint[i].x = pFacePoint[2 * i];
			FacePoint[i].y = pFacePoint[2 * i + 1];
		}
		std::vector<Point2D> PuiplePoint(numIntelatPoint);
		int nCurrentIndex = 0;
		nCurrentIndex = EyePupilInterlate(FacePoint.data(), nCurrentIndex, PuiplePoint.data());
		if (nCurrentIndex == numIntelatPoint)
		{
			for (int i = 0; i < numIntelatPoint; i++)
			{
				pPuiplePoint[2 * i] = PuiplePoint[i].x;
				pPuiplePoint[2 * i + 1] = PuiplePoint[i].y;
			}
		}
		else
		{
			return false;
		}    
		return true;
	}

	int EyePupilInterlation::EyePupilInterlate(const Point2D * pFacePoint, int nCurrentIndex,Point2D* pPuiplePoint)
	{
		float AxisLength[2] = {0.0f};
		int EyePuipleEdgeIndex[4] = { 114,115,116,117 };
		for (int i = 0; i < 2; i++)
		{
			Point2D TemPoint = pFacePoint[EyePuipleEdgeIndex[2 * i]] - pFacePoint[EyePuipleEdgeIndex[2 * i + 1]];
			AxisLength[i] = TemPoint.length();
		}
		int PupilIndex[] = { 59,69 };
		for (int i = 0; i < 2; i++)
		{
			
			float ALength = AxisLength[i]/2.0f;
			float BLength = ALength * 0.91f;
			Point2D TemPoint = pFacePoint[PupilIndex[i]];
			pPuiplePoint[nCurrentIndex] = TemPoint;
			nCurrentIndex++;
			float  OvalShape[] = { -1.0f,-0.9f,-0.7f,-0.35f,0.0f,0.35f,0.7f,0.9f,1.0f };
			for (int j = 0; j < 9; j++)
			{
				Point2D PointValue=Point2D(0.0f,0.0f);
				PointValue.x = OvalShape[j] * ALength;
				float TemValue = PointValue.x*PointValue.x;
				TemValue /= ALength * ALength;
				PointValue.y = (1.0f - TemValue)*BLength*BLength;
				PointValue.y = sqrtf(PointValue.y);
				Point2D ResultPoint = PointValue;
				ResultPoint.x = TemPoint.x + ResultPoint.x;
				ResultPoint.y = TemPoint.y - ResultPoint.y;
				pPuiplePoint[nCurrentIndex] = ResultPoint;
				nCurrentIndex++;
				if (PointValue.y)
				{
					ResultPoint.y = TemPoint.y + PointValue.y;
					pPuiplePoint[nCurrentIndex] = ResultPoint;
					nCurrentIndex++;
				}
				
			}
		
		}
		return nCurrentIndex;
	}

}//mt3dface






