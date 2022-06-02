#include "FaceDeformation.h"
#include "Common/CC3DUtils.h"

FaceDeformation::FaceDeformation()
{
	m_pShiftRGBA = NULL;
	m_pOffestTexture = NULL;
	m_StackBlur = NULL;

	Init();
}

FaceDeformation::~FaceDeformation()
{
	SAFE_DELETE_ARRAY(m_pShiftRGBA);
	SAFE_DELETE_ARRAY(m_pOffestTexture);
	SAFE_DELETE(m_StackBlur);
	SAFE_DELETE_ARRAY(m_pAlpha);
}


int FaceDeformation::Init()
{
	int size = m_StandFacePoint.size() / 2;
	if (m_SrcMeshPoint.size() != size)
	{
		m_SrcMeshPoint.resize(size);
		m_DstMeshPoint.resize(size);
	}

	for (int i = 0; i < size; i++)
	{
		m_SrcMeshPoint[i].point.x = m_StandFacePoint[2 * i];
		m_SrcMeshPoint[i].point.y = m_StandFacePoint[2 * i + 1];

		if (std::find(m_ControlIndex.begin(), m_ControlIndex.end(), (unsigned short)i) != m_ControlIndex.end())
		{
			m_SrcMeshPoint[i].isControl = true;
		}
		else
		{
			m_SrcMeshPoint[i].isControl = false;
		}

	}
	m_DstMeshPoint = m_SrcMeshPoint;

	m_FaceMesh.pTriangleIndex = m_StandIndex.data();
	m_FaceMesh.nTriangle = m_StandIndex.size() / 3;


//  	SetControlIndex(0, 1);
//  	SetControlPoint(0, m_SrcMeshPoint[0].point + Vector2(-0.05,0 ),1);
	return 1;
}


CCFace2DMesh& FaceDeformation::GetFaceMesh()
{
	m_FaceMesh.nVertex = m_DstMeshPoint.size();
	//for (int i = 0; i < m_DstMeshPoint.size(); i++)
	//{
	//	if (!m_DstMeshPoint[i].isControl)
	//	{
	//		m_DstMeshPoint[i] = m_SrcMeshPoint[i];
	//	}
	//}
	m_FaceMesh.pDstVertexs = (PointInfo*)m_DstMeshPoint.data();

	return m_FaceMesh;
}

int FaceDeformation::GetSymmetryIndex(int index)
{
	if (index < 98 || index > 111)
	{
		if (index % 2 == 0)
		{
			return index + 1;
		}
		return index - 1;
	}
	else
	{
		return index;
	}

}

void FaceDeformation::SetControlIndex(unsigned short index, bool iSymmetry)
{
	if (index >= m_SrcMeshPoint.size())
	{
		return;
	}

	m_DstMeshPoint[index].isControl = true;

// 	m_DstMeshPoint[index] = m_SrcMeshPoint[index];
// 	m_SrcMeshPoint[index].point = Vector2(m_StandFacePoint[index*2], m_StandFacePoint[index * 2+1]);
	if (iSymmetry)
	{
		if (index < 98 || index > 111)
		{
			int idx = GetSymmetryIndex(index);
			m_DstMeshPoint[idx].isControl = true;

// 			m_DstMeshPoint[idx] = m_SrcMeshPoint[idx];
// 			m_SrcMeshPoint[idx].point = Vector2(m_StandFacePoint[idx * 2], m_StandFacePoint[idx * 2 + 1]);
		}
	}
}

void FaceDeformation::SetFreeIndex(unsigned short index, bool iSymmetry)
{
	if (index >= m_SrcMeshPoint.size())
	{
		return;
	}
	m_SrcMeshPoint[index].isControl = false;
	m_DstMeshPoint[index].isControl = false;

	m_SrcMeshPoint[index].isMultiControl = false;
	m_DstMeshPoint[index].isMultiControl = false;

	if (iSymmetry)
	{
		if (index < 98 || index > 111)
		{
			int idx = GetSymmetryIndex(index);
			m_SrcMeshPoint[idx].isControl = false;
			m_DstMeshPoint[idx].isControl = false;

			m_SrcMeshPoint[idx].isMultiControl = false;
			m_DstMeshPoint[idx].isMultiControl = false;

		}
	}

}

void FaceDeformation::SetMultiControlIndex(unsigned short index, bool iSymmetry)
{
	if (index >= m_SrcMeshPoint.size())
	{
		return;
	}

	m_DstMeshPoint[index].isMultiControl = true;

	// 	m_DstMeshPoint[index] = m_SrcMeshPoint[index];
	// 	m_SrcMeshPoint[index].point = Vector2(m_StandFacePoint[index*2], m_StandFacePoint[index * 2+1]);
	if (iSymmetry)
	{
		if (index < 98 || index > 111)
		{
			int idx = GetSymmetryIndex(index);
			m_DstMeshPoint[idx].isMultiControl = true;

			// 			m_DstMeshPoint[idx] = m_SrcMeshPoint[idx];
			// 			m_SrcMeshPoint[idx].point = Vector2(m_StandFacePoint[idx * 2], m_StandFacePoint[idx * 2 + 1]);
		}
	}
}

void FaceDeformation::SetFreeMultiControlIndex(unsigned short index, bool iSymmetry)
{
	if (index >= m_SrcMeshPoint.size())
	{
		return;
	}
	m_SrcMeshPoint[index].isMultiControl = false;
	m_DstMeshPoint[index].isMultiControl = false;

	if (iSymmetry)
	{
		if (index < 98 || index > 111)
		{
			int idx = GetSymmetryIndex(index);
			m_SrcMeshPoint[idx].isMultiControl = false;
			m_DstMeshPoint[idx].isMultiControl = false;

		}
	}

}

void FaceDeformation::SetControlPoint(unsigned short indexDst, Vector2 &posDst, bool iSymmetry, bool isXlock, bool isYLock)
{
	if (indexDst >= m_DstMeshPoint.size())
	{
		return;
	}

	if (isYLock && isXlock)
	{
		return;
	}

	//m_DstMeshPoint[indexDst].point = posDst;

	Vector2 offset = posDst - m_DstMeshPoint[indexDst].point;
	int idx = GetSymmetryIndex(indexDst);
	
	if (idx == indexDst && iSymmetry)//如果是中间点，并设置对称时没有x方向偏移量
	{
		offset.x = 0;
	}

	if (isXlock && !isYLock)//锁定x轴
	{	
		offset.x = 0;
	}
	else if (isYLock && !isXlock)//锁定Y轴
	{
		offset.y = 0;
	}

	m_DstMeshPoint[indexDst].point = m_DstMeshPoint[indexDst].point + offset;

	 
	if (iSymmetry)
	{
		
		//int idx = GetSymmetryIndex(indexDst);
		if (idx == indexDst)//如果是中间点，对称时没有x方向偏移量
		{
			return;
		}
		offset.x = -offset.x;
		if (isXlock && !isYLock)//对称时锁定x轴
		{
			offset.x = 0;
		}
		else if (isYLock && !isXlock)//对称时锁定Y轴
		{
			offset.y = 0;
		}

		m_DstMeshPoint[idx].point = m_DstMeshPoint[idx].point + offset;
	}

	//updateOffest();

}

void FaceDeformation::updateOffest()
{
	if (m_pShiftRGBA == NULL)
	{
		m_pShiftRGBA = new Vector2[m_SmallTextureHeight * m_SmallTextureWidth];
	}

	int nControlCount = 0;

	std::vector<Vector2>srcControlPoint;
	std::vector<Vector2>dstControlPoint;

	for (int i = 0; i < m_DstMeshPoint.size(); i++)
	{
		if (m_DstMeshPoint[i].isControl)
		{
			nControlCount = nControlCount + 1;

			srcControlPoint.push_back(m_SrcMeshPoint[i].point * Vector2(m_SmallTextureWidth,m_SmallTextureHeight));
			dstControlPoint.push_back(m_DstMeshPoint[i].point* Vector2(m_SmallTextureWidth, m_SmallTextureHeight));
		}

	}
	//MLS get free point offset
	MLSControlPoint(srcControlPoint.data(), dstControlPoint.data(), nControlCount);
	//Create Offset
	CreateOffset();

	for (int i = 0; i < m_SmallTextureHeight; ++i) {
		for (int j = 0; j < m_SmallTextureWidth; ++j) {
			float ox = m_pOffestTexture[(i*m_SmallTextureWidth + j) * 4] * 1.f + m_pOffestTexture[(i*m_SmallTextureWidth + j) * 4 + 1] / 255.f;
			float oy = m_pOffestTexture[(i*m_SmallTextureWidth + j) * 4 + 2] * 1.f + m_pOffestTexture[(i*m_SmallTextureWidth + j) * 4 + 3] / 255.f;
			ox = (ox - 127.f)*0.25f;
			oy = (oy - 127.f)*0.25f;
			ox = ox * m_SmallTextureWidth / 255.f;
			oy = oy * m_SmallTextureHeight / 255.f;
			m_pShiftRGBA[i*m_SmallTextureWidth + j] = Vector2(j + ox, i + oy);
		}
	}
	int nSize = m_SmallTextureHeight * m_SmallTextureWidth;
	Vector2 *pStandFacePoint = (Vector2*)m_StandFacePoint.data();
	for (int n = 0; n < m_DstMeshPoint.size(); ++n) {
		if (m_DstMeshPoint[n].isControl==false)
		{
			Vector2 pos = pStandFacePoint[n] * Vector2(m_SmallTextureWidth, m_SmallTextureHeight);
			float minDist = 10000.0f;
			int minIndex = 0;
			for (int i = 0; i < nSize; ++i) {
				float dist = m_pShiftRGBA[i].distance(pos);
				if (dist < minDist)
				{
					minDist = dist;
					minIndex = i;
				}
			}

			int y = minIndex / m_SmallTextureWidth;
			int x = minIndex - y * m_SmallTextureWidth;

			float xx = pos.x, yy = pos.y;
			if ((m_pShiftRGBA[minIndex - 1].x-pos.x)*(m_pShiftRGBA[minIndex].x - pos.x)<0)
			{
				float len1 = fabs(m_pShiftRGBA[minIndex - 1].x - pos.x);
				float len2 = fabs(m_pShiftRGBA[minIndex].x - m_pShiftRGBA[minIndex - 1].x);
				if (len2>0.00001)
				{
					float alpha = len1 / len2;
					xx = (x-1)*(1 - alpha) + x*alpha;
				}
			}
			else if ((m_pShiftRGBA[minIndex + 1].x - pos.x)*(m_pShiftRGBA[minIndex].x - pos.x) < 0)
			{
				float len1 = fabs(m_pShiftRGBA[minIndex + 1].x - pos.x);
				float len2 = fabs(m_pShiftRGBA[minIndex].x - m_pShiftRGBA[minIndex + 1].x);
				if (len2 > 0.00001)
				{
					float alpha = len1 / len2;
					xx = (x + 1)*(1 - alpha) + x * alpha;
				}
			}

			if ((m_pShiftRGBA[minIndex - m_SmallTextureWidth].y - pos.y)*(m_pShiftRGBA[minIndex].y - pos.y) < 0)
			{
				float len1 = fabs(m_pShiftRGBA[minIndex - m_SmallTextureWidth].y- pos.y);
				float len2 = fabs(m_pShiftRGBA[minIndex].y - m_pShiftRGBA[minIndex - m_SmallTextureWidth].y);
				if (len2 > 0.00001)
				{
					float alpha = len1 / len2;
					yy = (y - 1)*(1 - alpha) + y * alpha;
				}
			}
			else if ((m_pShiftRGBA[minIndex + m_SmallTextureWidth].y - pos.y)*(m_pShiftRGBA[minIndex].y - pos.y) < 0)
			{
				float len1 = fabs(m_pShiftRGBA[minIndex + m_SmallTextureWidth].y - pos.y);
				float len2 = fabs(m_pShiftRGBA[minIndex].y - m_pShiftRGBA[minIndex + m_SmallTextureWidth].y);
				if (len2 > 0.00001)
				{
					float alpha = len1 / len2;
					yy = (y + 1)*(1 - alpha) + y * alpha;
				}
			}


			m_DstMeshPoint[n].point = Vector2(xx, yy)*Vector2(1.f/m_SmallTextureWidth,1.f/m_SmallTextureHeight);
		}


	}
}

void FaceDeformation::setAlphaPath(std::string path)
{
	m_AlphaPath = path;
}

void FaceDeformation::MLSControlPoint(Vector2* pSrc, Vector2* pDst, int nCount)
{

	float* pSrcCombine = new float[nCount * 4];
	float* pSrcX = pSrcCombine;
	float* pSrcY = pSrcX + nCount;
	float* pDstX = pSrcY + nCount;
	float* pDstY = pDstX + nCount;

	for (int i = 0; i < nCount; i++) {
		pSrcX[i] = pSrc[i].x;
		pSrcY[i] = pSrc[i].y;
		pDstX[i] = pDst[i].x;
		pDstY[i] = pDst[i].y;
	}

	MLSParam param;
	param.pSrcX = pSrcX;
	param.pSrcY = pSrcY;
	param.pDstX = pDstX;
	param.pDstY = pDstY;
	param.width = m_SmallTextureWidth;
	param.height = m_SmallTextureHeight;
	param.shiftHeight = m_SmallTextureHeight;
	param.shiftWidth = m_SmallTextureWidth;
	param.pointCount = nCount;
	//param.shiftRGBA = m_pShiftRGBA;
	RunMLSSP(param);

	SAFE_DELETE_ARRAY(pSrcCombine);

}

void FaceDeformation::RunMLSSP(MLSParam &param)
{
	int outputWidth = param.shiftWidth;
	int outputHeight = param.shiftHeight;
	int effectPointCount = param.pointCount;

	float xScale = param.width / (float)param.shiftWidth;
	float yScale = param.height / (float)param.shiftHeight;

	float* arr_r_s = new float[effectPointCount];

	for (int y = 0; y < outputHeight; y++)
	{
		float y_in_src = (y + 0.5f)*yScale;

		for (int x = 0; x < outputWidth; x++)
		{
			float x_in_src = (x + 0.5f)*xScale;

			float sumFactor = 0.0f;
			float sumofSrcX = 0.0f;
			float sumofSrcY = 0.0f;
			float sumofDstX = 0.0f;
			float sumofDstY = 0.0f;
			//������������Ƶ��Ȩ��
			for (int k = 0; k < effectPointCount; k++)
			{
				float src_x = param.pSrcX[k];
				float src_y = param.pSrcY[k];
				float dst_x = param.pDstX[k];
				float dst_y = param.pDstY[k];

				float a = x_in_src - dst_x;
				float b = y_in_src - dst_y;

				float s = a * a + b * b;
				s *= s;
				float r_s = 3.0f / (s*s);
				arr_r_s[k] = r_s;

				sumFactor += r_s;
				sumofDstX += dst_x * r_s;
				sumofDstY += dst_y * r_s;
				sumofSrcX += src_x * r_s;
				sumofSrcY += src_y * r_s;

			}

			//������4�������Ȩ��
			float x_calc_factor[4] = { 0.0f };
			x_calc_factor[0] = x_in_src;
			x_calc_factor[1] = 0.0f;
			x_calc_factor[2] = (float)param.width;
			x_calc_factor[3] = x_in_src;

			float y_calc_factor[4] = { 0.0f };
			y_calc_factor[0] = 0.0f;
			y_calc_factor[1] = y_in_src;
			y_calc_factor[2] = y_in_src;
			y_calc_factor[3] = (float)param.height;

			float float32x4_0010[4] = { 0.0f };

			for (int k = 0; k < 4; k++)
			{
				float floattar08 = x_in_src - x_calc_factor[k];
				float floattar09 = y_in_src - y_calc_factor[k];
				float temp = floattar08 * floattar08 + floattar09 * floattar09;
				temp *= temp;
				float32x4_0010[k] = 1.0f / (temp*temp);
				sumofDstX += float32x4_0010[k] * x_calc_factor[k];
				sumofSrcX += float32x4_0010[k] * x_calc_factor[k];
				sumofDstY += float32x4_0010[k] * y_calc_factor[k];
				sumofSrcY += float32x4_0010[k] * y_calc_factor[k];
				sumFactor += float32x4_0010[k];

			}

			//17
			float devFactorDstX = sumofDstX / sumFactor;
			//18
			float devFactorDstY = sumofDstY / sumFactor;
			//19
			float reDevFactorDstX = x_in_src - devFactorDstX;
			//20
			float reDevFactorDstY = y_in_src - devFactorDstY;

			float float21 = 0.0f;
			float float22 = 0.0f;


			for (int k = 0; k < effectPointCount; k++)
			{
				float src_x = param.pSrcX[k];
				float src_y = param.pSrcY[k];
				float dst_x = param.pDstX[k];
				float dst_y = param.pDstY[k];

				float offsetDevDstX = dst_x - devFactorDstX;
				float offsetDevDstY = dst_y - devFactorDstY;

				float float25 = (reDevFactorDstX * offsetDevDstX) + (reDevFactorDstY*offsetDevDstY);

				float float26 = (reDevFactorDstY* offsetDevDstX) - (reDevFactorDstX*offsetDevDstY);

				float offsetSrcDstX = src_x - devFactorDstX;
				float offsetSrcDstY = src_y - devFactorDstY;

				float float29 = arr_r_s[k];
				float21 += (float29 *(offsetSrcDstX*float25 - float26 * offsetSrcDstY));

				float22 += (float29*(offsetSrcDstX*float26 + float25 * offsetSrcDstY));

			}

			//////////////////////////////////////////////////////////////////////////
			float float32x4_0033[4];
			float float32x4_0034[4];
			float float07 = float22;
			float float06 = float21;
			for (int k = 0; k < 4; k++)
			{
				float float31 = (x_calc_factor[k] - devFactorDstX);
				float float32 = (y_calc_factor[k] - devFactorDstY);

				float32x4_0033[k] = (float32 * reDevFactorDstY) + (float31 * reDevFactorDstX);
				float32x4_0034[k] = (float31 * reDevFactorDstY) - (float32 * reDevFactorDstX);
				float07 += ((float32 * float32x4_0033[k]) + (float31 * float32x4_0034[k]))*float32x4_0010[k];
				float06 += ((float31 * float32x4_0033[k]) - (float32 * float32x4_0034[k]))*float32x4_0010[k];
			}


			float float008 = sqrtf((reDevFactorDstX*reDevFactorDstX + reDevFactorDstY * reDevFactorDstY) / (float07*float07 + float06 * float06));

			float float04 = sumofSrcX / sumFactor;
			float float05 = sumofSrcY / sumFactor;

			float float09 = (float04 + float06 * float008) - reDevFactorDstX - devFactorDstX;
			float float10 = (float05 + float07 * float008) - reDevFactorDstY - devFactorDstY;
			//if (float09 < 0.0001) float09 = 0;
			//if (float10 < 0.0001) float10 = 0;
			m_pShiftRGBA[(y*param.shiftWidth + x)].x = float09 * m_ShapeAlpha;// / m_SmallTextureWidth;
			m_pShiftRGBA[(y*param.shiftWidth + x)].y = float10 * m_ShapeAlpha; // / m_SmallTextureHeight;

		}
	}
	SAFE_DELETE_ARRAY(arr_r_s);

}

void FaceDeformation::CreateOffset()
{
	if (m_pOffestTexture == NULL)
	{
		m_pOffestTexture = new BYTE[m_SmallTextureHeight * m_SmallTextureWidth * 4];

	}
	if (m_pAlpha == NULL)
	{
		int w, h, n;
		m_pAlpha = ccLoadImage(m_AlphaPath.c_str(), &w, &h, &n, 4);
	}

	if (m_StackBlur == NULL)
	{
		m_StackBlur = new CStackBlur();
	}
	int nSize = m_SmallTextureWidth * m_SmallTextureHeight;

	m_StackBlur->OneChannelAvg(m_pShiftRGBA, m_SmallTextureWidth, m_SmallTextureHeight, m_SmoothRadius);
	if (m_pAlpha != NULL)
	{
		for (int y = 0; y < m_SmallTextureHeight; y++)
		{
			for (int x = 0; x < m_SmallTextureWidth; x++)
			{
				BYTE alpha = m_pAlpha[(y*m_SmallTextureWidth + x) * 4];
				float xc = m_pShiftRGBA[(y*m_SmallTextureWidth + x)].x*alpha * 4 / m_SmallTextureWidth + 127;
				float yc = m_pShiftRGBA[(y*m_SmallTextureWidth + x)].y*alpha * 4 / m_SmallTextureHeight + 127;
				xc = (std::max)(0.f, (std::min)(xc, 255.f));
				yc = (std::max)(0.f, (std::min)(yc, 255.f));
				int xi = xc;
				int yi = yc;

				int g = (xc - xi) * 255;
				int a = (yc - yi) * 255;

				m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4] = xi;
				m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 1] = g;
				m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 2] = yi;
				m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 3] = a;
			}
		}
	}
	else
	{
		for (int y = 0; y < m_SmallTextureHeight; y++)
		{
			for (int x = 0; x < m_SmallTextureWidth; x++)
			{
				BYTE alpha = 255;
				float xc = m_pShiftRGBA[(y*m_SmallTextureWidth + x)].x*alpha * 4 / m_SmallTextureWidth + 127;
				float yc = m_pShiftRGBA[(y*m_SmallTextureWidth + x)].y*alpha * 4 / m_SmallTextureHeight + 127;
				xc = (std::max)(0.f, (std::min)(xc, 255.f));
				yc = (std::max)(0.f, (std::min)(yc, 255.f));
				int xi = xc;
				int yi = yc;

				int g = (xc - xi) * 255;
				int a = (yc - yi) * 255;

				m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4] = xi;
				m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 1] = g;
				m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 2] = yi;
				m_pOffestTexture[(y*m_SmallTextureWidth + x) * 4 + 3] = a;
			}
		}
	}

}

BYTE * FaceDeformation::GetOffsetTexture()
{
	if (!m_pOffestTexture)
	{
		return NULL;
	}
	return m_pOffestTexture;
}

void FaceDeformation::SetSmoothRadius(float &radius)
{
	m_SmoothRadius = radius;
}

void FaceDeformation::ReadConfig(XMLNode &childNode)
{	
	XMLNode nodeDrawable = childNode.getChildNode("dstpoint", 0);
	const char *szSmoothRadis = nodeDrawable.getAttribute("SmoothRadis");
	if (szSmoothRadis != NULL)
	{
		sscanf(szSmoothRadis, "%f", &m_SmoothRadius);
	}
	int j = -1;
	XMLNode nodePoint = nodeDrawable.getChildNode("point", ++j);
	while (!nodePoint.isEmpty())
	{
		const char *szDstPoint = nodePoint.getAttribute("pos");

		if (szDstPoint != NULL)
		{
			float x = 0.f;
			float y = 0.f;
			int index = 0;
			sscanf(szDstPoint, "%f,%f,%d", &x, &y, &index);
			m_DstMeshPoint[index].point.x = x;
			m_DstMeshPoint[index].point.y = y;
			m_DstMeshPoint[index].isControl = true;
		}

		nodePoint = nodeDrawable.getChildNode("point", ++j);
	}

	updateOffest();

}

void FaceDeformation::WriteConfig(XMLNode &nodeEffect)
{
	XMLNode nodeDrwable = nodeEffect.addChild("dstpoint");
	char param[128];
	sprintf(param, "%.4f", m_SmoothRadius);
	nodeDrwable.addAttribute("SmoothRadis", param);
	for (int i = 0; i < m_DstMeshPoint.size(); i++)
	{
		if (m_DstMeshPoint[i].isControl)
		{
			XMLNode nodePoint = nodeDrwable.addChild("point");
			char dstpoint[256];
			sprintf(dstpoint, "%.6f,%.6f,%d", m_DstMeshPoint[i].point.x, m_DstMeshPoint[i].point.y, i);
			nodePoint.addAttribute("pos", dstpoint);
		}

	}

}


