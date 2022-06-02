#include "MathUtils.h"
#include <algorithm>

#ifndef DBL_EPSILON
#define DBL_EPSILON 2.2204460492503131e-016 
#define LDBL_EPSILON DBL_EPSILON
#endif

#undef SS
#define SS(x,y) (x*x + y*y)


void CMathUtils::LagrangePolyfit(Vector2* polyfitIn,int InNum,Vector2* polyfitOut,int OutNum )
{
	Vector2 firstPt = polyfitIn[0];
	Vector2 lastPt = polyfitIn[InNum - 1];
	Vector2 vectorL2R;//方向向量
	vectorL2R.x = lastPt.x - firstPt.x;
	vectorL2R.y = lastPt.y - firstPt.y;

	float ss = SS(vectorL2R.x,vectorL2R.y);
	float normValue = sqrt(ss);
	if(fabs(normValue) < 0.001f)
	{
		normValue = 0.001f;
	}
	//顺时针旋转
	float sinVal = -vectorL2R.y /normValue;
	float cosVal = vectorL2R.x /normValue;

	//保存旋转过好的临时坐标集
	Vector2* tmpInVec = new Vector2[InNum];
	Vector2 tmpPt;

	for (int i =0 ; i<InNum; i++)
	{
		polyfitIn[i].x -= firstPt.x;
		polyfitIn[i].y -= firstPt.y;
		tmpPt.x = cosVal*polyfitIn[i].x - sinVal*polyfitIn[i].y;
		tmpPt.y = cosVal*polyfitIn[i].y + sinVal*polyfitIn[i].x;
		tmpInVec[i] = tmpPt;
	}

	// 	//保存临时的第一次旋转过后插值结果集

	//计算插值间距
	float gap = (tmpInVec[InNum -1].x - tmpInVec[0].x)/(OutNum - 1);
	//设定拟合的横坐标值
	for (int i =0; i<OutNum;i++)
	{
		tmpPt.x = tmpInVec[0].x + gap*i;
		tmpPt.y = -1;
		polyfitOut[i] = tmpPt;
	}

	//算出每一个拟合值
	for (int i =0; i<OutNum;i++)
	{
		float tmp_y = 0.0f;
		for (int j = 0;j<InNum;j++)
		{
			float Ltmp = 1.0f;
			for (int k = 0;k<InNum;k++)
			{
				if (k == j)
				{
					continue;
				}
				float fDen = (tmpInVec[j].x - tmpInVec[k].x);
				if(fabs(fDen) < 0.0001f)
				{
					fDen = 0.0001f;
				}
				Ltmp = Ltmp*(polyfitOut[i].x - tmpInVec[k].x)/fDen;
				//Ltmp = Ltmp*(polyfitOut[i].x - tmpInVec[k].x)/(tmpInVec[j].x - tmpInVec[k].x);
			}
			Ltmp *= tmpInVec[j].y;
			tmp_y += Ltmp;
		}	
		polyfitOut[i].y = tmp_y;
	}

	//逆时针旋转
	sinVal = -sinVal;
	for (int i =0 ; i<OutNum;i++)
	{
		tmpPt.x = cosVal*polyfitOut[i].x - sinVal*polyfitOut[i].y;
		tmpPt.y = cosVal*polyfitOut[i].y + sinVal*polyfitOut[i].x;
		tmpPt.x += firstPt.x;
		tmpPt.y += firstPt.y;
		polyfitOut[i] = tmpPt;
	}

	SAFE_DELETE_ARRAY(tmpInVec);
}

void CMathUtils::LagrangePolyfit( vector<Vector2> polyfitIn, vector<Vector2>& polyfitOut, int ptnum )
{
	polyfitOut.clear();
	Vector2 firstPt = polyfitIn.at(0);
	Vector2 lastPt = polyfitIn.at(polyfitIn.size() - 1);
	Vector2 vectorL2R;//方向向量
	vectorL2R.x = lastPt.x - firstPt.x;
	vectorL2R.y = lastPt.y - firstPt.y;
	float ss = SS(vectorL2R.x,vectorL2R.y);
	float normValue = sqrt(ss);
	if(fabs(normValue) < 0.001f)
	{
		normValue = 0.001f;
	}
	//顺时针旋转
	float sinVal = -vectorL2R.y /normValue;
	float cosVal = vectorL2R.x /normValue;

	int Innum = polyfitIn.size();
	vector<Vector2> tmpInVec;
	Vector2 tmpPt;

	for (int i =0 ; i<Innum;i++)
	{
		polyfitIn[i].x -= firstPt.x;
		polyfitIn[i].y -= firstPt.y;
		tmpPt.x = cosVal*polyfitIn[i].x - sinVal*polyfitIn[i].y;
		tmpPt.y = cosVal*polyfitIn[i].y + sinVal*polyfitIn[i].x;
		tmpInVec.push_back(tmpPt);
	}

	vector<Vector2> tmpOutVec; 

	float gap = (tmpInVec.back().x - tmpInVec[0].x)/(ptnum - 1);
	//设定拟合的横坐标值
	for (int i =0; i<ptnum;i++)
	{
		tmpPt.x = tmpInVec[0].x + gap*i;
		tmpPt.y = -1;
		tmpOutVec.push_back(tmpPt);
	}
	//算出每一个拟合值
	for (int i =0; i<ptnum;i++)
	{
		float tmp_y = 0.0f;
		for (int j = 0;j<Innum;j++)
		{
			float Ltmp = 1.0f;
			for (int k = 0;k<Innum;k++)
			{
				if (k == j)
				{
					continue;
				}
				float fDen = (tmpInVec[j].x - tmpInVec[k].x);
				if(fabs(fDen) < 0.0001f)
				{
					fDen = 0.0001f;
				}
				Ltmp = Ltmp*(tmpOutVec[i].x - tmpInVec[k].x)/fDen;
				//Ltmp = Ltmp*(tmpOutVec[i].x - tmpInVec[k].x)/(tmpInVec[j].x - tmpInVec[k].x);
			}
			Ltmp *= tmpInVec[j].y;
			tmp_y += Ltmp;
		}	
		tmpOutVec[i].y = tmp_y;
	}

	//逆时针旋转
	sinVal = -sinVal;
	for (unsigned int i =0 ; i<tmpOutVec.size();i++)
	{
		tmpPt.x = cosVal*tmpOutVec[i].x - sinVal*tmpOutVec[i].y;
		tmpPt.y = cosVal*tmpOutVec[i].y + sinVal*tmpOutVec[i].x;
		tmpPt.x += firstPt.x;
		tmpPt.y += firstPt.y;
		polyfitOut.push_back(tmpPt);
	}
}


void CMathUtils::CalcUnitVector( Vector2 &startPt, Vector2 &endPt ,Vector2 &UnitVec )
{
	UnitVec.x = endPt.x - startPt.x;
	UnitVec.y = endPt.y - startPt.y;
	float UnitVetMod = SS(UnitVec.x,UnitVec.y);
	UnitVetMod = sqrt(UnitVetMod);
	UnitVec.x /= UnitVetMod;
	UnitVec.y /= UnitVetMod;
}



template<typename _Tp>  int
	CMathUtils::LUImpl(_Tp* A, size_t astep, int m, _Tp* b, size_t bstep, int n)
{
	int i, j, k, p = 1;
	astep /= sizeof(A[0]);
	bstep /= sizeof(b[0]);

	for( i = 0; i < m; i++ )
	{
		k = i;

		for( j = i+1; j < m; j++ )
			if( std::abs(A[j*astep + i]) > std::abs(A[k*astep + i]) )
				k = j;

		if( std::abs(A[k*astep + i]) < LDBL_EPSILON  )
			return 0;

		if( k != i )
		{
			for( j = i; j < m; j++ )
				std::swap(A[i*astep + j], A[k*astep + j]);
			if( b )
				for( j = 0; j < n; j++ )
					std::swap(b[i*bstep + j], b[k*bstep + j]);
			p = -p;
		}

		_Tp d = -1/A[i*astep + i];

		for( j = i+1; j < m; j++ )
		{
			_Tp alpha = A[j*astep + i]*d;

			for( k = i+1; k < m; k++ )
				A[j*astep + k] += alpha*A[i*astep + k];

			if( b )
				for( k = 0; k < n; k++ )
					b[j*bstep + k] += alpha*b[i*bstep + k];
		}

		A[i*astep + i] = -d;
	}

	if( b )
	{
		for( i = m-1; i >= 0; i-- )
			for( j = 0; j < n; j++ )
			{
				_Tp s = b[i*bstep + j];
				for( k = i+1; k < m; k++ )
					s -= A[i*astep + k]*b[k*bstep + j];
				b[i*bstep + j] = s*A[i*astep + i];
			}
	}

	return p;/**/
}

bool CMathUtils::SolveMat(double _src[36],double _src2arg[6],double _dst[6])
{

	double a[36];
	int m = 6, m_ = m, n = 6, nb = 1;
	for(int i = 0 ; i < 6;i++)
	{
		for(int j = 0 ; j < 6;j++)
		{
			a[i*6 + j] = _src[i*6 + j];
		}
	}



	for(int i = 0 ; i < 6;i++)
	{
		_dst[i] = _src2arg[i];
	}



	LUImpl(a, 6*sizeof(double), n, _dst, sizeof(double), nb);



	return true;
}

Matrix3 CMathUtils::getAffineTransform(Vector2 src[],Vector2 dst[])
{
	Matrix3 res;


	double a[6*6], b[6],x[6];


	for( int i = 0; i < 3; i++ )
	{
		int j = i*12;
		int k = i*12+6;
		a[j] = a[k+3] = src[i].x;
		a[j+1] = a[k+4] = src[i].y;
		a[j+2] = a[k+5] = 1;
		a[j+3] = a[j+4] = a[j+5] = 0;
		a[k] = a[k+1] = a[k+2] = 0;
		b[i*2] = dst[i].x;
		b[i*2+1] = dst[i].y;
	}

	SolveMat( a, b, x );
	float fx[9];
	memset(fx, 0, sizeof(float) * 9);
	for(int i=0;i<6;i++)
		fx[i] = static_cast<float>(x[i]);
	res.set(fx);

	return res;
}


template<typename T, typename WT>  void
	CMathUtils::transform_( const T* src, T* dst, const WT* m, int len, int cn, int dcn )
{
	int x;

	for( x = 0; x < len; x++, src += cn, dst += dcn )
	{
		const WT* _m = m;
		int j, k;
		for( j = 0; j < dcn; j++, _m += cn + 1 )
		{
			WT s = _m[cn];
			for( k = 0; k < cn; k++ )
				s += _m[k]*src[k];
			dst[j] = static_cast<T>(s);
		}
	}
}

void CMathUtils::transform(vector<Vector2> &srcArr,vector<Vector2> &dstArr,Matrix3 &mat)
{
	const float *values = mat.get();

	float *val1 = new float[dstArr.size()*2];
	float *val2 = new float[dstArr.size()*2];
	for(unsigned int i = 0;i<dstArr.size();i++)
	{
		val1[i*2] = srcArr[i].x;
		val1[i*2+1] = srcArr[i].y;
	}
	transform_(val1,val2,values,dstArr.size(),2,2);
	for(unsigned int i = 0;i<dstArr.size();i++)
	{
		dstArr[i].x = val2[i*2];
		dstArr[i].y = val2[i*2+1];
	}
	delete [] val1;
	delete [] val2;
}

void CMathUtils::transform(Vector2 * srcArr, Vector2 * dstArr, int nPoints, Matrix3 & mat)
{
	transform_((float*)srcArr, (float*)dstArr, (float*)mat.get(), nPoints, 2, 2);
}

// 比较向量中哪个与x轴向量(1, 0)的夹角更大  
static bool CompareVector(const Vector2 &pt1, const Vector2 &pt2) {  
	//求向量的模  
	float m1 = sqrt((float)(pt1.x * pt1.x + pt1.y * pt1.y));  
	float m2 = sqrt((float)(pt2.x * pt2.x + pt2.y * pt2.y));  
	//两个向量分别与(1, 0)求内积  
	float v1 = pt1.x / m1, v2 = pt2.x / m2;  
	//如果向量夹角相等，则返回离基点较近的一个，保证有序  
	return (v1 > v2 || v1 == v2 && m1 < m2);  
} 
void CMathUtils::CalcConvexHull(vector<Vector2> &vecSrc)
{
	//点集中至少应有3个点，才能构成多边形  
	if (vecSrc.size() < 3) {  
		return;  
	}  
	//查找基点  
	Vector2 ptBase = vecSrc.front(); //将第1个点预设为最小点  
	for (unsigned int i = 0 + 1; i < vecSrc.size(); ++i) {  
		//如果当前点的y值小于最小点，或y值相等，x值较小  
		if (vecSrc[i].y < ptBase.y || (vecSrc[i].y == ptBase.y && vecSrc[i].x > ptBase.x)) {  
			//将当前点作为最小点  
			ptBase = vecSrc[i];  
		}  
	}  
	//计算出各点与基点构成的向量  
	for (vector<Vector2>::iterator i = vecSrc.begin(); i != vecSrc.end();) {  
		//排除与基点相同的点，避免后面的排序计算中出现除0错误  
		if (*i == ptBase) {  
			i = vecSrc.erase(i);  
		}  
		else {  
			//方向由基点到目标点  
			i->x -= ptBase.x, i->y -= ptBase.y;  
			++i;  
		}  
	}  
	//按各向量与横坐标之间的夹角排序  
	sort(vecSrc.begin(), vecSrc.end(),&CompareVector);  
	//删除相同的向量  
	vecSrc.erase(unique(vecSrc.begin(), vecSrc.end()), vecSrc.end());  
	//计算得到首尾依次相联的向量  
	for (vector<Vector2>::reverse_iterator ri = vecSrc.rbegin();  
		ri != vecSrc.rend() - 1; ++ri) {  
			vector<Vector2>::reverse_iterator riNext = ri + 1;  
			//向量三角形计算公式  
			ri->x -= riNext->x, ri->y -= riNext->y;  
	}  
	//依次删除不在凸包上的向量  
	for (vector<Vector2>::iterator i = vecSrc.begin() + 1; i != vecSrc.end(); ++i) {  
		//回溯删除旋转方向相反的向量，使用外积判断旋转方向  
		for (vector<Vector2>::iterator iLast = i - 1; iLast != vecSrc.begin();) {  
			int v1 = (int)(i->x * iLast->y), v2 = (int)(i->y * iLast->x);  
			//如果叉积小于0，则无没有逆向旋转  
			//如果叉积等于0，还需判断方向是否相逆  
			if (v1 < v2 || (v1 == v2 && i->x * iLast->x > 0 &&  
				i->y * iLast->y > 0)) {  
					break;  
			}  
			//删除前一个向量后，需更新当前向量，与前面的向量首尾相连  
			//向量三角形计算公式  
			i->x += iLast->x, i->y += iLast->y;  
			iLast = (i = vecSrc.erase(iLast)) - 1;  
		}  
	}  
	//将所有首尾相连的向量依次累加，换算成坐标  
	vecSrc.front().x += ptBase.x, vecSrc.front().y += ptBase.y;  
	for (vector<Vector2>::iterator i = vecSrc.begin() + 1; i != vecSrc.end(); ++i) {  
		i->x += (i - 1)->x, i->y += (i - 1)->y;  
	}  
	//添加基点，全部的凸包计算完成  
	vecSrc.push_back(ptBase);  
}


bool CMathUtils::FillRect5(byte* pMask,int nWidth,int nHeight,vector<Vector2>& points,byte fillColor)
{
	if(pMask == NULL || points.size() == 0)
	{
		return false;
	}
	int pointCount = points.size();

	Vector2* circlePoints = new Vector2[pointCount + 1];
	for(int i = 0 ; i < pointCount;i++)
	{
		circlePoints[i] = points[i];
	}
	//memcpy(circlePoints,points,sizeof(Vector2)*pointCount);
	circlePoints[pointCount].x = circlePoints[0].x;
	circlePoints[pointCount].y = circlePoints[0].y;
	///////求外接矩形///////////////////////////////////////
	float maxX = circlePoints[0].x;
	float minX = circlePoints[0].x;
	float maxY = circlePoints[0].y;
	float minY = circlePoints[0].y;
	for(int i = pointCount; i > 0 ; i--)
	{
		if(circlePoints[i].x < minX ) minX = circlePoints[i].x;
		else if(circlePoints[i].x > maxX) maxX = circlePoints[i].x;

		if(circlePoints[i].y < minY) minY = circlePoints[i].y;
		else if(circlePoints[i].y > maxY) maxY = circlePoints[i].y;
	}
	int minXi = max(0,(int)minX);
	int maxXi = min((int)maxX + 1, nWidth - 1);
	int minYi = max(0,(int)minY);
	int maxYi = min((int)maxY + 1,nHeight - 1);
	//LOGD("min max %d,%d,%d,%d",minXi,maxXi,minYi,maxYi);
	byte* pTmp = pMask;
	for(int i = minYi; i <= maxYi;i++)
	{
		Vector2 p;
		p.y = (float)(i);

		int left = minXi,right = maxXi;
		while(left <= right)
		{
			p.x = (float)(left);
			if(InsidePolygon(circlePoints,pointCount,p))
			{
				break;
			}
			left ++;
		}
		if(right >= left)
		{
			int l = left,r = right ,m = (left + right)>>1;
			int key = right;
			while(l <= r)
			{
				m = (l + r)>>1;
				p.x = (float)(m);
				if(InsidePolygon(circlePoints,pointCount,p))
				{
					l = m + 1;
					key = m;
				}
				else
				{
					r = m - 1;
				}
			}
			right = key;
			pTmp = pMask + i*nWidth + left;
			for(int j = left ; j <= right ; j ++)
			{
				*pTmp++ = fillColor;
			}
		}

	}
	SAFE_DELETE_ARRAY(circlePoints);
	return true;
}

bool CMathUtils::FillRect6(byte* pMask,int nWidth,int nHeight,vector<Vector2>& points,byte fillColor)
{
	if(pMask == NULL || points.size() == 0)
	{
		return false;
	}
	int pointCount = points.size();

	byte* pTmpMask = new byte[nWidth*nHeight];
	memcpy(pTmpMask,pMask,nWidth*nHeight);
	memset(pMask,fillColor,nWidth*nHeight);

	Vector2* circlePoints = new Vector2[pointCount + 1];
	for(int i = 0 ; i < pointCount;i++)
	{
		circlePoints[i] = points[i];
	}
	//memcpy(circlePoints,points,sizeof(Vector2)*pointCount);
	circlePoints[pointCount].x = circlePoints[0].x;
	circlePoints[pointCount].y = circlePoints[0].y;
	///////求外接矩形///////////////////////////////////////
	float maxX = circlePoints[0].x;
	float minX = circlePoints[0].x;
	float maxY = circlePoints[0].y;
	float minY = circlePoints[0].y;
	for(int i = pointCount; i > 0 ; i--)
	{
		if(circlePoints[i].x < minX ) minX = circlePoints[i].x;
		else if(circlePoints[i].x > maxX) maxX = circlePoints[i].x;

		if(circlePoints[i].y < minY) minY = circlePoints[i].y;
		else if(circlePoints[i].y > maxY) maxY = circlePoints[i].y;
	}
	int minXi = max(0,(int)minX);
	int maxXi = min((int)maxX + 1, nWidth - 1);
	int minYi = max(0,(int)minY);
	int maxYi = min((int)maxY + 1,nHeight - 1);
	byte* pTmp = pMask;
	for(int i = minYi; i <= maxYi;i++)
	{
		Vector2 p;
		p.y = (float)(i);

		int left = minXi,right = maxXi;
		while(left <= right)
		{
			p.x = (float)(left);
			if(InsidePolygon(circlePoints,pointCount,p))
			{
				break;
			}
			left ++;
		}
		if(right >= left)
		{
			int l = left,r = right ,m = (left + right)>>1;
			int key = right;
			while(l <= r)
			{
				m = (l + r)>>1;
				p.x = (float)(m);
				if(InsidePolygon(circlePoints,pointCount,p))
				{
					l = m + 1;
					key = m;
				}
				else
				{
					r = m - 1;
				}
			}
			right = key;
			pTmp = pMask + i*nWidth + left;
			byte* pSrc = pTmpMask + i*nWidth + left;
			for(int j = left ; j <= right ; j ++)
			{
				*pTmp++ = *pSrc++;
			}
		}

	}
	SAFE_DELETE_ARRAY(pTmpMask);
	SAFE_DELETE_ARRAY(circlePoints);
	return true;
}

bool CMathUtils::InsidePolygon(Vector2 *polygon,int N,Vector2 p)
{
	int counter = 0;
	for (int i=1;i<=N;i++) 
	{
		Vector2 p1 = polygon[i-1];
		Vector2 p2;
		p2 = polygon[i];
		if (p.y > min(p1.y,p2.y)) {
			if (p.y <= max(p1.y,p2.y)) {
				if (p.x <= max(p1.x,p2.x)) {
					if (p1.y != p2.y) {
						float xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
						if (p1.x == p2.x || p.x <= xinters)
							counter++;
					}
				}
			}
		}
	}

	if (counter % 2 == 0)
		return false;
	else
		return true;
}

BYTE* CMathUtils::DrawLine( int nWidth,int nHeight,int pointCount,Vector2* points )
{
	int nPixelCount = nWidth*nHeight;
	byte* pTmpMask = new byte[nPixelCount];
	memset(pTmpMask, 0, nPixelCount);
	// 连直线
	float step, sumPos;
	float xDist, yDist;
	float absXDist, absYDist;
	int startValue, endValue;
	int i = 0,t = 0;
	for (i=1; i<pointCount; ++i)
	{
		xDist = points[i].x - points[i - 1].x;
		yDist = points[i].y - points[i - 1].y;

		absXDist = abs(xDist);
		absYDist = abs(yDist);

		if (absXDist <= absYDist)
		{
			step = xDist / absYDist;
			sumPos = points[i - 1].x;
			startValue = int(points[i - 1].y + 0.5f);
			endValue = int(points[i].y + 0.5f);

			if (startValue > endValue)
			{
				sumPos = points[i].x;
				startValue = int(points[i].y + 0.5f);
				endValue = int(points[i - 1].y + 0.5f);
				step = -step;
			}

			int sumYOffset = startValue * nWidth;
			for (t=startValue; t<=endValue; ++t)
			{
				int x = (int)(sumPos + 0.5f);
				int pos = sumYOffset + x;
				if(pos >=0 && pos < nPixelCount)
				{
					pTmpMask[pos] = 255;
				}

				sumPos += step;
				sumYOffset += nWidth;
			}
		}
		else
		{
			step = yDist / absXDist;
			sumPos = points[i - 1].y;
			startValue = int(points[i - 1].x + 0.5f);
			endValue = int(points[i].x + 0.5f);

			if (startValue > endValue)
			{
				sumPos = points[i].y;
				startValue = int(points[i].x + 0.5f);
				endValue = int(points[i - 1].x + 0.5f);
				step = -step;
			}
			for (t=startValue; t<=endValue; ++t)
			{
				int y = (int)(sumPos + 0.5f);
				int pos = y * nWidth + t;
				if(pos >=0 && pos < nPixelCount)
				{
					pTmpMask[pos] = 255;
				}
				sumPos += step;
			}
		}
	}
	return pTmpMask;
}

void CMathUtils::GetOutSideRect( vector<Vector2> &srcArr,int& left,int& top,int& right,int& bottom )
{
	if(srcArr.size() == 0)
	{
		return ;
	}
	left = right = (int)(srcArr[0].x);
	top = bottom = (int)(srcArr[0].y);
	int nCount  = srcArr.size();
	for(int i = 0 ; i < nCount; i ++)
	{
		if(srcArr[i].x < left)
		{
			left = (int)(srcArr[i].x);
		}
		else if(srcArr[i].x > right)
		{
			right = (int)(srcArr[i].x);
		}

		if(srcArr[i].y < top)
		{
			top = (int)(srcArr[i].y);
		}
		else if(srcArr[i].y > bottom)
		{
			bottom = (int)(srcArr[i].y);
		}
	}
}

void CMathUtils::GetOutSideRect(Vector2 * srcArr, int nCount, int & left, int & top, int & right, int & bottom)
{
	if (nCount == 0)
	{
		return;
	}
	left = right = (int)(srcArr[0].x);
	top = bottom = (int)(srcArr[0].y);
	for (int i = 0; i < nCount; i++)
	{
		if (srcArr[i].x < left)
		{
			left = (int)(srcArr[i].x);
		}
		else if (srcArr[i].x > right)
		{
			right = (int)(srcArr[i].x);
		}

		if (srcArr[i].y < top)
		{
			top = (int)(srcArr[i].y);
		}
		else if (srcArr[i].y > bottom)
		{
			bottom = (int)(srcArr[i].y);
		}
	}

}


// 点连线
void CMathUtils::ConnectPoint(Vector2** ppPoints, int nCount, BYTE* pBuffer, int nWidth, int nHeight)
{
	// 连直线
	float step, sumPos;
	float xDist, yDist;
	float absXDist, absYDist;
	int startValue, endValue;
	int i = 0,t = 0;

	for (i=1; i<nCount; ++i)
	{
		xDist = ppPoints[i]->x - ppPoints[i - 1]->x;
		yDist = ppPoints[i]->y - ppPoints[i - 1]->y;

		absXDist = fabs(xDist);
		absYDist = fabs(yDist);

		if (absXDist <= absYDist)
		{
			step = xDist * 1.0f / absYDist;
			sumPos = ppPoints[i - 1]->x;
			startValue = int(ppPoints[i - 1]->y + 0.5f);
			endValue = int(ppPoints[i]->y + 0.5f);

			if (startValue > endValue)
			{
				sumPos = ppPoints[i]->x;
				startValue = int(ppPoints[i]->y + 0.5f);
				endValue = int(ppPoints[i - 1]->y + 0.5f);
				step = -step;
			}

			int sumYOffset = startValue * nWidth;
			for (t=startValue; t<=endValue; ++t)
			{
				int x = (int)(sumPos);
				int pos = sumYOffset + x;
				pBuffer[pos] = 255;

				sumPos += step;
				sumYOffset += nWidth;
			}
		}
		else
		{
			step = yDist * 1.0f / absXDist;
			sumPos = ppPoints[i - 1]->y;
			startValue = int(ppPoints[i - 1]->x + 0.5f);
			endValue = int(ppPoints[i]->x + 0.5f);

			if (startValue > endValue)
			{
				sumPos = ppPoints[i]->y;
				startValue = int(ppPoints[i]->x + 0.5f);
				endValue = int(ppPoints[i - 1]->x + 0.5f);
				step = -step;
			}

			for (t=startValue; t<=endValue; ++t)
			{
				int y = (int)(sumPos);
				int pos = y * nWidth + t;
				pBuffer[pos] = 255;

				sumPos += step;
			}
		}
	}
}

// 种子填充
void CMathUtils::SeedFill(BYTE *pMask, int width, int height, int startX, int startY)
{
	int* queue = new int[width * height * 2];
	int head = 0, end = 0;
	int tx, ty;
	int tyStride;
	if(startX < 0)
	{
		startX = 0;
	}
	else if(startX >= width)
	{
		startX = width -1;
	}
	if(startY < 0)
	{
		startY = 0;
	}
	else if(startY >= height)
	{
		startY = height - 1;
	}
	/* Add node to the end of queue. */
	queue[end*2+0] = startX;
	queue[end*2+1] = startY;
	end++;

	while (head < end)
	{	
		tx = queue[(head<<1)+0];
		ty = queue[(head<<1)+1];
		tyStride = ty * width;

		if (pMask[tyStride + tx] == 0) 
		{
			int w = tx;
			int e = tx;

			while (w-1 >= 0 && pMask[tyStride + (w - 1)] == 0)
				w--;
			while (e+1 < width && pMask[tyStride + (e + 1)] == 0) 
				e++;

			for (int i=w; i<=e; i++) 
			{
				// change color
				pMask[tyStride + i] = 255;

				if (ty-1 >=0 && pMask[tyStride - width + i] == 0)
				{
					queue[(end<<1)+0] = i;
					queue[(end<<1)+1] = ty-1;
					end++;
				}

				if (ty+1 < height && pMask[tyStride + width + i] == 0) 
				{
					queue[(end<<1)+0] = i;
					queue[(end<<1)+1] = ty+1;
					end++;
				}
			}
		}
		/* Remove the first element from queue. */
		head++;
	}
	delete [] queue;
}


int CMathUtils::SmoothLines(vector<Vector2>& pOutPts, vector<Vector2>& pInPts ,float tension ,int numOfSegments )
{
	int Num = pInPts.size();

	if(Num < 4)
	{
		return 0;
	}
	
	
	Vector2* _pts = new Vector2[Num +2];

	// clone array so we don't change the original
	//_pts = ptsa.concat();
	for(int i = 0 ; i < Num ; i++)
	{
		_pts[i+1] = pInPts[i];
	}
	/**
	 *	The algorithm require a previous and next point to the actual point array.
	 *	Check if we will draw closed or open curve.
	 *	If closed, copy end points to beginning and first points to end
	 *	If open, duplicate first points to befinning, end points to end
	*/

	//_pts.unshift(pts[1]);			//copy 1. point and insert at beginning
	//_pts.unshift(pts[0]);
	//_pts.push(pts[pts.length - 2]);	//copy last point and append
	//_pts.push(pts[pts.length - 1]);
	_pts[0] = pInPts[0];
	_pts[1+Num] = pInPts[Num-1];
	int l = Num;

	int resLen = (Num - 1)*(numOfSegments+1);
	// Calculations:
	pOutPts.clear();
	// 1. loop goes through point array
	// 2. loop goes through each segment between the two points
	int cnt = 0;
	for (int i=1; i < l; i++) 
	{
		for (int t=0; t <= numOfSegments; t++) 
		{

			// calc tension vectors
			float t1x = (_pts[i+1].x - _pts[i-1].x) * tension;
			float t2x = (_pts[i+2].x - _pts[i].x) * tension;
	
			float t1y = (_pts[i+1].y - _pts[i-1].y) * tension;
			float t2y = (_pts[i+2].y - _pts[i].y) * tension;

			// pre-calc step
			float st = (float)t / numOfSegments;
			float st2 = st * st;
			float st3 = st2 * st;
			float st23 = st3 * 2;
			float st32 = st2 * 3;

			// calc cardinals
			float c1 = st23 - st32 + 1; 
			float c2 = -(st23) + st32; 
			float c3 = st3 - 2 * st2 + st; 
			float c4 = st3 - st2;

			// calc x and y cords with common control vectors
			float x = c1 * _pts[i].x	+ c2 * _pts[i+1].x + c3 * t1x + c4 * t2x;
			float y = c1 * _pts[i].y	+ c2 * _pts[i+1].y + c3 * t1y + c4 * t2y;
		
			//store points in array
			pOutPts.push_back(Vector2(x,y));
			cnt++;
		}
		
	}
	delete [] _pts;
	return resLen;
}

int CMathUtils::SmoothLines2(Vector2 * pOutPts, Vector2 * pInPts, int nPoints, float tension, int numOfSegments)
{
	int Num = nPoints;

	int resLen = (Num - 1)*(numOfSegments)+1;

	if (pOutPts == NULL || pInPts == NULL)
	{
		return resLen;
	}

	Vector2* _pts = new Vector2[Num + 2];

	// clone array so we don't change the original
	//_pts = ptsa.concat();
	memcpy(_pts + 1, pInPts, sizeof(Vector2)*Num);

	/**
	* The algorithm require a previous and next point to the actual point array.
	* Check if we will draw closed or open curve.
	* If closed, copy end points to beginning and first points to end
	* If open, duplicate first points to befinning, end points to end
	*/

	//_pts.unshift(pts[1]);   //copy 1. point and insert at beginning
	//_pts.unshift(pts[0]);
	//_pts.push(pts[pts.length - 2]); //copy last point and append
	//_pts.push(pts[pts.length - 1]);
	_pts[0] = pInPts[0];
	_pts[1 + Num] = pInPts[Num - 1];

	// Calculations:
	// 1. loop goes through point array
	// 2. loop goes through each segment between the two points
	int cnt = 0;
	pOutPts[cnt++] = _pts[0];
	int l = Num;
	for (int i = 1; i < l; i++)
	{
		for (int t = 1; t <= numOfSegments; t++)
		{

			// calc tension vectors
			float t1x = (_pts[i + 1].x - _pts[i - 1].x) * tension;
			float t2x = (_pts[i + 2].x - _pts[i].x) * tension;

			float t1y = (_pts[i + 1].y - _pts[i - 1].y) * tension;
			float t2y = (_pts[i + 2].y - _pts[i].y) * tension;

			// pre-calc step
			float st = (float)t / numOfSegments;
			float st2 = st * st;
			float st3 = st2 * st;
			float st23 = st3 * 2;
			float st32 = st2 * 3;

			// calc cardinals
			float c1 = st23 - st32 + 1;
			float c2 = -(st23)+st32;
			float c3 = st3 - 2 * st2 + st;
			float c4 = st3 - st2;

			// calc x and y cords with common control vectors
			float x = c1 * _pts[i].x + c2 * _pts[i + 1].x + c3 * t1x + c4 * t2x;
			float y = c1 * _pts[i].y + c2 * _pts[i + 1].y + c3 * t1y + c4 * t2y;

			//store points in array
			pOutPts[cnt].x = x;
			pOutPts[cnt].y = y;
			cnt++;
		}

	}
	delete[] _pts;
	return resLen;
}


 void CMathUtils::addMatrix(const float* m, float scalar, float* dst)
{
	dst[0] = m[0] + scalar;
	dst[1] = m[1] + scalar;
	dst[2] = m[2] + scalar;
	dst[3] = m[3] + scalar;
	dst[4] = m[4] + scalar;
	dst[5] = m[5] + scalar;
	dst[6] = m[6] + scalar;
	dst[7] = m[7] + scalar;
	dst[8] = m[8] + scalar;
	dst[9] = m[9] + scalar;
	dst[10] = m[10] + scalar;
	dst[11] = m[11] + scalar;
	dst[12] = m[12] + scalar;
	dst[13] = m[13] + scalar;
	dst[14] = m[14] + scalar;
	dst[15] = m[15] + scalar;
}

 void CMathUtils::addMatrix(const float* m1, const float* m2, float* dst)
{
	dst[0] = m1[0] + m2[0];
	dst[1] = m1[1] + m2[1];
	dst[2] = m1[2] + m2[2];
	dst[3] = m1[3] + m2[3];
	dst[4] = m1[4] + m2[4];
	dst[5] = m1[5] + m2[5];
	dst[6] = m1[6] + m2[6];
	dst[7] = m1[7] + m2[7];
	dst[8] = m1[8] + m2[8];
	dst[9] = m1[9] + m2[9];
	dst[10] = m1[10] + m2[10];
	dst[11] = m1[11] + m2[11];
	dst[12] = m1[12] + m2[12];
	dst[13] = m1[13] + m2[13];
	dst[14] = m1[14] + m2[14];
	dst[15] = m1[15] + m2[15];
}

 void CMathUtils::subtractMatrix(const float* m1, const float* m2, float* dst)
{
	dst[0] = m1[0] - m2[0];
	dst[1] = m1[1] - m2[1];
	dst[2] = m1[2] - m2[2];
	dst[3] = m1[3] - m2[3];
	dst[4] = m1[4] - m2[4];
	dst[5] = m1[5] - m2[5];
	dst[6] = m1[6] - m2[6];
	dst[7] = m1[7] - m2[7];
	dst[8] = m1[8] - m2[8];
	dst[9] = m1[9] - m2[9];
	dst[10] = m1[10] - m2[10];
	dst[11] = m1[11] - m2[11];
	dst[12] = m1[12] - m2[12];
	dst[13] = m1[13] - m2[13];
	dst[14] = m1[14] - m2[14];
	dst[15] = m1[15] - m2[15];
}

 void CMathUtils::multiplyMatrix(const float* m, float scalar, float* dst)
{
	dst[0] = m[0] * scalar;
	dst[1] = m[1] * scalar;
	dst[2] = m[2] * scalar;
	dst[3] = m[3] * scalar;
	dst[4] = m[4] * scalar;
	dst[5] = m[5] * scalar;
	dst[6] = m[6] * scalar;
	dst[7] = m[7] * scalar;
	dst[8] = m[8] * scalar;
	dst[9] = m[9] * scalar;
	dst[10] = m[10] * scalar;
	dst[11] = m[11] * scalar;
	dst[12] = m[12] * scalar;
	dst[13] = m[13] * scalar;
	dst[14] = m[14] * scalar;
	dst[15] = m[15] * scalar;
}

 void CMathUtils::multiplyMatrix(const float* m1, const float* m2, float* dst)
{
	// Support the case where m1 or m2 is the same array as dst.
	float product[16];

	product[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
	product[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
	product[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
	product[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];

	product[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
	product[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
	product[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
	product[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];

	product[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
	product[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
	product[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
	product[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];

	product[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
	product[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
	product[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
	product[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];

	memcpy(dst, product, MATRIX_SIZE);
}

 void CMathUtils::negateMatrix(const float* m, float* dst)
{
	dst[0] = -m[0];
	dst[1] = -m[1];
	dst[2] = -m[2];
	dst[3] = -m[3];
	dst[4] = -m[4];
	dst[5] = -m[5];
	dst[6] = -m[6];
	dst[7] = -m[7];
	dst[8] = -m[8];
	dst[9] = -m[9];
	dst[10] = -m[10];
	dst[11] = -m[11];
	dst[12] = -m[12];
	dst[13] = -m[13];
	dst[14] = -m[14];
	dst[15] = -m[15];
}

 void CMathUtils::transposeMatrix(const float* m, float* dst)
{
	float t[16] = {
		m[0], m[4], m[8], m[12],
		m[1], m[5], m[9], m[13],
		m[2], m[6], m[10], m[14],
		m[3], m[7], m[11], m[15]
	};
	memcpy(dst, t, MATRIX_SIZE);
}

 void CMathUtils::transformVector4(const float* m, float x, float y, float z, float w, float* dst)
{
	dst[0] = x * m[0] + y * m[4] + z * m[8] + w * m[12];
	dst[1] = x * m[1] + y * m[5] + z * m[9] + w * m[13];
	dst[2] = x * m[2] + y * m[6] + z * m[10] + w * m[14];
}

 void CMathUtils::transformVector4(const float* m, const float* v, float* dst)
{
	// Handle case where v == dst.
	float x = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
	float y = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
	float z = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
	float w = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];

	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}

 void CMathUtils::crossVector3(const float* v1, const float* v2, float* dst)
{
	float x = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	float y = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	float z = (v1[0] * v2[1]) - (v1[1] * v2[0]);

	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
}