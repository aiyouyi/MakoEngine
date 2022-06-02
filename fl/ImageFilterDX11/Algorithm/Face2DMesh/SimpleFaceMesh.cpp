#include <cstdio>
#include <cassert>
#include <algorithm>
#include "SimpleFaceMesh.h"
#include "FaceInterlation.h"

namespace mt3dface
{	
	///////////////////////////////////////////////////////////////

    void SimpleFaceMesh::setMemberInit()
    {
		m_numLandmark = 106;
        m_numOfSimpleFaceMeshV = 235;
		m_nNumOfFacePt = 235;

        // simpleMesh
		// 1260个index对应420个三角形，即235个顶点连接成420个三角形？
        std::vector<short3D> simpleMeshIndex = {
			127,0,1,127,1,135,2,135,1,3,2,4,135,2,3,71,37,75,38,75,37,162,83,165,82,165,83,139,4,5,26,163,25,139,5,6,228,82,80,142,6,7,215,100,222,104,222,100,147,148,143,78,144,227,189,227,144,232,233,203,94,203,233,11,10,12,146,10,11,231,15,232,16,232,15,146,11,12,190,213,191,214,191,213,230,12,13,231,13,14,231,14,15,142,143,140,141,140,143,232,16,17,233,17,18,177,38,54,176,54,38,233,18,19,129,76,73,74,73,76,234,19,20,54,176,56,167,20,21,22,21,23,167,21,22,177,54,56,39,38,36,37,36,38,40,35,41,34,41,35,35,39,36,160,26,27,156,158,29,160,27,28,58,133,57,131,57,133,156,29,30,171,41,51,156,30,31,43,50,49,181,49,50,128,31,32,117,116,115,108,115,116,185,64,66,184,66,64,109,114,115,34,115,114,119,37,42,71,42,37,41,34,33,114,33,34,107,108,116,118,119,120,121,120,119,115,34,35,116,118,107,106,107,118,117,35,36,119,36,37,48,43,49,130,55,131,56,131,55,175,38,39,40,39,35,174,39,40,172,40,41,48,64,47,185,47,64,119,118,116,50,43,42,121,42,43,118,120,106,110,106,120,44,47,45,46,45,47,48,44,43,123,43,44,123,124,122,111,122,124,35,117,115,124,44,45,123,44,124,125,45,46,48,47,44,183,48,49,181,50,62,180,62,50,71,50,42,84,150,73,173,40,52,172,52,40,175,39,53,174,53,39,172,41,171,173,174,40,132,130,131,176,38,175,68,151,152,153,152,151,38,177,75,164,162,165,178,56,55,131,134,132,131,56,57,134,133,138,136,138,133,172,58,52,173,52,58,136,135,137,3,137,135,171,51,58,151,150,153,84,153,150,75,72,71,85,71,72,85,180,50,183,49,63,182,63,49,61,179,85,180,85,179,186,65,47,31,128,156,186,185,66,186,66,65,66,67,154,152,154,67,68,181,180,62,180,181,157,154,159,155,159,154,152,67,68,73,74,84,151,68,61,179,61,68,85,50,71,151,85,72,130,72,75,129,72,130,61,85,151,143,144,77,78,77,144,166,199,234,200,234,199,231,96,230,207,230,96,204,95,232,205,232,95,206,96,231,130,75,55,133,134,131,188,211,99,226,99,211,228,227,191,88,191,227,164,168,163,166,92,199,102,199,92,153,155,152,154,152,155,26,160,163,161,163,160,155,84,83,145,149,146,98,187,86,144,86,187,208,97,230,145,230,97,88,190,191,189,144,87,188,87,144,88,227,190,189,190,227,193,215,194,216,194,215,192,89,228,193,228,89,94,233,202,201,202,233,80,227,228,229,82,228,233,19,234,194,228,193,195,229,90,228,90,229,166,167,170,197,229,91,196,91,229,166,170,169,168,169,170,165,229,197,168,164,169,220,217,221,216,221,217,234,200,201,93,201,200,101,220,103,93,103,220,204,232,203,90,194,216,222,104,203,204,203,104,231,232,205,192,191,214,205,95,223,104,223,95,230,13,231,188,187,211,225,105,207,208,207,105,146,12,230,210,98,86,99,226,105,97,105,226,213,224,214,223,214,224,188,99,87,189,87,99,99,212,189,190,189,212,206,223,224,214,100,89,193,89,100,221,215,222,201,93,220,217,101,91,197,91,101,197,101,218,103,218,101,197,218,198,200,103,93,223,104,100,203,94,222,221,222,94,204,104,95,212,99,105,96,225,207,208,105,97,120,122,110,111,110,122,108,109,115,113,33,114,113,109,0,45,125,124,112,124,125,46,126,125,112,125,126,126,32,112,124,112,111,114,109,113,117,36,119,116,117,119,123,121,43,121,119,42,122,120,121,123,122,121,132,76,129,132,129,130,137,138,136,141,77,134,139,6,142,140,139,142,141,143,77,147,149,148,145,148,149,147,143,142,209,145,97,230,145,146,151,72,150,84,155,153,162,155,83,158,156,157,159,158,157,164,161,162,163,161,164,229,165,82,166,234,167,20,167,234,169,92,166,171,58,172,178,177,56,178,55,177,180,179,68,181,182,49,186,47,185,188,144,187,212,213,190,192,228,191,192,214,89,215,193,100,228,194,90,195,90,216,196,229,195,196,217,91,216,196,195,198,165,197,219,103,200,219,200,199,94,202,221,220,221,202,233,234,201,220,202,201,232,17,233,213,212,225,105,225,212,206,231,205,225,96,206,223,206,205,208,230,207,226,209,97,210,145,209,226,210,209,224,213,225,214,223,100,216,215,221,216,217,196,217,220,101,219,218,103,225,206,224,133,58,51,136,133,51,66,154,65,154,157,65,77,74,76,77,78,74,74,83,84,74,82,83,74,80,82,78,80,74,78,227,80,148,145,86,86,145,210,198,92,165,198,102,92,218,102,198,199,102,219,219,102,218,98,211,187,210,226,98,226,211,98,173,58,57,173,57,174,174,57,53,53,57,175,175,57,56,175,56,176,181,68,182,182,68,63,63,68,67,63,67,183,183,67,184,184,67,66,177,178,75,178,55,75,7,8,147,142,7,147,163,168,25,25,168,24,168,170,24,170,23,24,170,167,23,167,22,23,128,65,157,128,157,156,51,127,136,127,135,136,162,159,155,159,162,161,159,161,158,158,161,160,9,146,149,9,10,146,147,8,149,8,9,149,72,129,73,72,73,150,132,134,76,134,77,76,3,4,139,137,3,139,137,139,140,137,140,138,138,140,141,138,141,134,158,160,29,29,160,28,48,183,184,48,184,64,113,0,33,33,0,127,46,128,32,126,46,32,33,127,51,33,51,41,47,65,46,65,128,46,143,148,86,143,86,144,164,165,92,164,92,169
        };
        m_numOfSimpleFaceMeshTri = (short3D)simpleMeshIndex.size() / 3;
		if(m_pSimpleFaceMeshV== nullptr || m_pSimpleFaceMeshVt== nullptr || m_pSimpleFaceMeshTriIndex== nullptr )
		{
			// V代表顶点（坐标），Vt代表纹理（纹理展开图坐标）
			m_pSimpleFaceMeshV = new float[m_numOfSimpleFaceMeshV * 3];
			m_pSimpleFaceMeshVt = new float[m_numOfSimpleFaceMeshV * 2];
			m_pSimpleFaceMeshTriIndex = new short3D[m_numOfSimpleFaceMeshTri * 3];
		}
        memcpy(m_pSimpleFaceMeshTriIndex, simpleMeshIndex.data(), sizeof(short3D)*m_numOfSimpleFaceMeshTri * 3);
    }

	bool SimpleFaceMesh::Run(const float *pImagePoint, const float fWidthImage, const float fHeightImage,
		 const float fFacePitchAngle)
	{
		if (pImagePoint == nullptr || fWidthImage <= 0 || fHeightImage <= 0)
		{
			return false;
		}
        setMemberInit();
		m_widthImage = fWidthImage;
		m_heightImage = fHeightImage;		
		m_facePitchAngle = fFacePitchAngle;
		float *pNewModifyLandmark = new float[(m_numLandmark+12) * 2];
		CalFaceSize(pImagePoint);
		ModifyLandmark(pImagePoint, pNewModifyLandmark);
		UpdateSimpleFaceMesh(pNewModifyLandmark);
		delete[] pNewModifyLandmark;
 		return true;
	}



	void SimpleFaceMesh::LocalVtUpdate(std::vector<Point2D>& VtCoord, std::vector<Point2D>& ImagePointVtCoord, float *pSimpleFaceMeshVt)
	{
		short3D LandmarkIndex[] = { 51,0,95,16,65,32,75,55,85,61 };
		float multiple[] = { 0.6f ,0.55f,0.6f,0.6f,0.6f};
		
		Point2D ImagePointVtBegin, ImagePointVtEnd, Direction, VtBegin, VtEnd; 
		for (int i = 0; i < 5; i++)
		{
			ImagePointVtBegin = ImagePointVtCoord[LandmarkIndex[2 * i]];
			ImagePointVtEnd = ImagePointVtCoord[LandmarkIndex[2 * i + 1]];
			Direction = ImagePointVtEnd - ImagePointVtBegin;
			VtBegin = VtCoord[LandmarkIndex[2 * i]];
			VtEnd = VtBegin + multiple[i] * Direction;
			pSimpleFaceMeshVt[2 * LandmarkIndex[2 * i + 1]] = VtEnd.x ;
			pSimpleFaceMeshVt[2 * LandmarkIndex[2 * i + 1] + 1] = VtEnd.y;
		}
		short3D EyeIndex[] = { 55,54,54,53,53,52,52,51,55,56,56,57,57,58,61,62,62,63,63,64,64,65,61,68,68,67,67,66 };
		for (int i = 0; i < 14; i++)
		{
			ImagePointVtBegin = VtCoord[EyeIndex[2 * i]];
			ImagePointVtEnd = VtCoord[EyeIndex[2 * i + 1]];
			Direction = ImagePointVtEnd - ImagePointVtBegin;
			pSimpleFaceMeshVt[2 * EyeIndex[2 * i + 1]] = pSimpleFaceMeshVt[2 * EyeIndex[2 * i]] + Direction.x;
			pSimpleFaceMeshVt[2 * EyeIndex[2 * i + 1] + 1] = pSimpleFaceMeshVt[2 * EyeIndex[2 * i] + 1] + Direction.y;
		}
	}
	
	void SimpleFaceMesh::LocalVtAdjust(std::vector<Point2D>& ImagePointVtCoord,float *pSimpleFaceMeshVt)
	{
		float LieRadio = 0.0f,ColRadio = 0.0f;
		float LeftLength = (ImagePointVtCoord[75] - ImagePointVtCoord[71]).length();
		float RightLength = (ImagePointVtCoord[85] - ImagePointVtCoord[71]).length();
		float LengtDivition = LeftLength - RightLength;
		//右转调节右眼的Vt值，否则调节左眼
		if (LengtDivition >= 0.0f)
		{
			for (int i = 61; i <= 68; i++)
			{
				pSimpleFaceMeshVt[2 * i] = pSimpleFaceMeshVt[2 * i] + 0.5f* LengtDivition;
			}
		}
		else
		{
			for (int i = 51; i <= 58; i++)
			{
				pSimpleFaceMeshVt[2 * i] = pSimpleFaceMeshVt[2 * i] + 0.5f*LengtDivition;
			}
		}

		//landmark0,16,32的Vt值调整
		if (m_facePitchAngle > 0.0f)//低头
		{
			LieRadio = 0.0003f;
			pSimpleFaceMeshVt[2 * 0 + 1] = pSimpleFaceMeshVt[2 * 0 + 1] - LieRadio*m_facePitchAngle;
			pSimpleFaceMeshVt[2 * 32 + 1] = pSimpleFaceMeshVt[2 * 32 + 1] - LieRadio*m_facePitchAngle;
			LieRadio = 0.00045f;
			pSimpleFaceMeshVt[2 * 16 + 1] = pSimpleFaceMeshVt[2 * 16 + 1] + LieRadio*m_facePitchAngle;
		}

		if (m_facePitchAngle < 0.0f)//抬头
		{

			float RadianAngle = -m_facePitchAngle*3.1415f / 180.f;
			LieRadio = 0.00017f*(1 / tan(RadianAngle));
			pSimpleFaceMeshVt[2 * 16 + 1] = pSimpleFaceMeshVt[2 * 16 + 1] - LieRadio*m_facePitchAngle;
		}

		LieRadio = 0.27f;
		ColRadio = 0.05f;
		if (m_mouthLength > 0.0f)//张嘴
		{
			pSimpleFaceMeshVt[2 * 0] = pSimpleFaceMeshVt[2 * 0] - ColRadio*m_mouthLength;
			pSimpleFaceMeshVt[2 * 32] = pSimpleFaceMeshVt[2 * 32] + ColRadio*m_mouthLength;
			pSimpleFaceMeshVt[2 * 16 + 1] = pSimpleFaceMeshVt[2 * 16 + 1] + LieRadio*m_mouthLength;
		}
		//判断landmark0的Vt是否在正确范围内
		if (pSimpleFaceMeshVt[2 * 0] < 0.0200f)
		{
			pSimpleFaceMeshVt[2 * 0] = 0.0200f;
		}
		if (pSimpleFaceMeshVt[2 * 0 + 1] < 0.3547f)
		{
			pSimpleFaceMeshVt[2 * 0 + 1] = 0.3547f;
		}
		else if (pSimpleFaceMeshVt[2 * 0 + 1] > 0.3983f)
		{
			pSimpleFaceMeshVt[2 * 0 + 1] = 0.3983f;
		}
		
		//判断landmark32是否在正确范围内
		if (pSimpleFaceMeshVt[2 * 32] > 0.9808f)
		{
			pSimpleFaceMeshVt[2 * 32] = 0.9808f;
		}
		if (pSimpleFaceMeshVt[2 * 32 + 1] < 0.3571f)
		{
			pSimpleFaceMeshVt[2 * 32 + 1] = 0.3571f;
		}
		else if (pSimpleFaceMeshVt[2 * 32 + 1] > 0.3983f)
		{
			pSimpleFaceMeshVt[2 * 32 + 1] = 0.3983f;
		}

		//判断landmark16的Vt值是否合理范围
		if (pSimpleFaceMeshVt[2 * 16 + 1] > 0.9934f)
		{
			pSimpleFaceMeshVt[2 * 16 + 1] = 0.9934f;
		}
	}
	bool SimpleFaceMesh::FaceContourLineVtUpdate(float *pSimpleFaceMeshVt, float *pImagePoint, short3D numVt)
	{
		if (pImagePoint == nullptr)
		{
			return false;
		}
		
		std::vector<Point2D> VtCoord(numVt);
		for (int i = 0; i < numVt; i++)
		{
			VtCoord[i].x = pSimpleFaceMeshVt[2 * i];
			VtCoord[i].y = pSimpleFaceMeshVt[2 * i + 1];
		}

		std::vector<Point2D> ImagePointVtCoord(numVt);
		for (int i = 0; i < numVt; i++)
		{
			ImagePointVtCoord[i].x = pImagePoint[2 * i];
			ImagePointVtCoord[i].y = pImagePoint[2 * i + 1];
		}
		//更新landmark对应Vt
		LocalVtUpdate(VtCoord, ImagePointVtCoord, pSimpleFaceMeshVt);
		//局部调整Vt值
		LocalVtAdjust(ImagePointVtCoord, pSimpleFaceMeshVt);
		return true;
	}


	bool SimpleFaceMesh::UpdateSimpleFaceMesh(const float* pImagePoint)
	{
		if (pImagePoint == nullptr)
		{
			return false;
		}
		//FA点插值，Face Align
		float* imagePoint = new float[m_numOfSimpleFaceMeshV * 2];
		FaceInterlation::RunFacePointInterpolation(pImagePoint, m_numLandmark, m_numOfSimpleFaceMeshV, m_IsNonLine,1.5f,0.73f, false,imagePoint);

		//构建新的V，VtImage， 从图像坐标的0～1变换到GL空间坐标-1～1，深度默认给0.5, V就是顶点，Vt是纹理
		for (int indexPoint = 0; indexPoint < m_numOfSimpleFaceMeshV; indexPoint++)
		{
			m_pSimpleFaceMeshV[indexPoint * 3 + 0] = imagePoint[indexPoint * 2 + 0] / m_widthImage * 2.0f - 1.0f;
			m_pSimpleFaceMeshV[indexPoint * 3 + 1] = imagePoint[indexPoint * 2 + 1] / m_heightImage * 2.0f - 1.0f;
			m_pSimpleFaceMeshV[indexPoint * 3 + 2] = 0.5f;
		}
		//更新五官点的深度值
		// 眉毛眼睛
		for (int i = 33; i < 70; i++)
		{
			m_pSimpleFaceMeshV[i * 3 + 2] = 0.5f;
		}
		for (int i = 171; i <= 186; i++)
		{
			m_pSimpleFaceMeshV[i * 3 + 2] = 0.5f;
		}
		// 鼻子和嘴巴
		for (int i = 71; i <= 105; i++)
		{
			m_pSimpleFaceMeshV[i * 3 + 2] = 0.1f;
		}
		for (int i = 187; i <= 226; i++)
		{
			m_pSimpleFaceMeshV[i * 3 + 2] = 0.1f;
		}

		//构建新的Vt
		std::vector<float> simpleFaceMeshVt = {
				0.026296f,0.624402f,0.0f,0.0f,0.037503f,0.544739f,0.0f,0.0f,0.061355f,0.447088f,0.0f,0.0f,0.104443f,0.336108f,0.0f,0.0f,0.162586f,0.226257f,0.0f,0.0f,0.218651f,0.182073f,0.0f,0.0f,0.31824f,0.133046f,0.0f,0.0f,0.412766f,0.135248f,0.0f,0.0f,0.508027f,0.129027f,0.0f,0.0f,0.596496f,0.131182f,0.0f,0.0f,0.688898f,0.131672f,0.0f,0.0f,0.815772f,0.204535f,0.0f,0.0f,0.883522f,0.254583f,0.0f,0.0f,0.930873f,0.340264f,0.0f,0.0f,0.956175f,0.439221f,0.0f,0.0f,0.969158f,0.53784f,0.0f,0.0f,0.97394f,0.621737f,0.237311f,0.690377f,0.292988f,0.726592f,0.338437f,0.737576f,0.386773f,0.729342f,0.421092f,0.720747f,0.412135f,0.703428f,0.379467f,0.713846f,0.3349f,0.709084f,0.300893f,0.704073f,0.578661f,0.722044f,0.608049f,0.731031f,0.650019f,0.73707f,0.693572f,0.725663f,0.757134f,0.689439f,0.686084f,0.708471f,0.654043f,0.7114f,0.615237f,0.715792f,0.586586f,0.706719f,0.28825f, 0.626159f,0.319438f, 0.657878f,0.360554f, 0.667801f,0.391171f, 0.651298f,0.411827f, 0.628533f,0.383871f, 0.614515f,0.349719f, 0.608252f,0.322647f, 0.611345f,0.0f, 0.0f, 0.0f, 0.0f,0.584194f, 0.62853f, 0.604985f, 0.659592f, 0.640207f, 0.669972f,0.681344f, 0.659842f,0.712515f, 0.628129f, 0.678489f, 0.614223f, 0.647444f, 0.609352f, 0.617262f, 0.614192f, 0.0f, 0.0f, 0.0f, 0.0f,0.499237f,0.659684f,0.498229f,0.613436f,0.498386f,0.554912f,0.49864f,0.495625f,0.448989f,0.642701f,0.43709f,0.524603f,0.402402f,0.463353f,0.43349f,0.427831f,0.470534f,0.473675f,0.499977f,0.424341f,0.527971f,0.474478f,0.568054f,0.426753f,0.595453f,0.462449f,0.559516f,0.524378f,0.54671f,0.644377f,0.391598f,0.317339f,0.437202f,0.36044f,0.479037f,0.375858f,0.499921f,0.373572f,0.529084f,0.375283f,0.574913f,0.362017f,0.61869f,0.324207f,0.581209f,0.2896f,0.538731f,0.273926f,0.502536f,0.272604f,0.469048f,0.273874f,0.431159f,0.287237f,0.407338f,0.317971f,0.447178f,0.336204f,0.507417f,0.338848f,0.564473f,0.336438f,0.603749f,0.323885f,0.565167f,0.31241f,0.506464f,0.309704f,0.446178f,0.311211f
		};
		memcpy(m_pSimpleFaceMeshVt, simpleFaceMeshVt.data(), sizeof(float)*m_numLandmark * 2);

		std::vector<float> BrownVt = {
				0.238973f,0.71232f,0.278014f,0.733009f,0.327089f,0.7343f,0.375781f,0.729447f,0.421092f,0.720747f,0.416061f,0.703526f,0.372757f,0.708554f,0.325727f,0.712101f,0.279771f,0.712271f,0.578661f,0.722044f,0.624587f,0.731762f,0.674362f,0.737747f,0.724506f,0.737169f,0.765875f,0.707349f,0.722896f,0.717147f,0.67634f,0.715817f,0.629355f,0.710994f,0.585378f,0.705016f
				/*0.238973f,0.70232f,0.278014f,0.733009f,0.327089f,0.7343f,0.375781f,0.729447f,0.421092f,0.720747f,0.416061f,0.703526f,0.372757f,0.708554f,0.325727f,0.712101f,0.279771f,0.712271f,0.578661f,0.722044f,0.624587f,0.731762f,0.674362f,0.733747f,0.724506f,0.733169f,0.761875f,0.69149f,0.722896f,0.708147f,0.67634f,0.715817f,0.629355f,0.710994f,0.585378f,0.705016f*/
		};
		memcpy(m_pSimpleFaceMeshVt+(33*2), BrownVt.data(), sizeof(float)*18 * 2);
		for (int indexPoint = 0; indexPoint < m_numLandmark; indexPoint++)
		{
			if (m_pSimpleFaceMeshVt[indexPoint * 2 + 1])
			{
				m_pSimpleFaceMeshVt[indexPoint * 2 + 1] = 1.0f - m_pSimpleFaceMeshVt[indexPoint * 2 + 1];
			}
		}
		//利用现有的FA点映射出landmark0,16,32的Vt值
		//FA点坐标归一化
		float* ImagePointVtCoord = new float[2 * m_numOfSimpleFaceMeshV];
		FacePointCoorTransform(imagePoint,ImagePointVtCoord);
		//轮廓线Vt更新和插值
		FaceContourLineVtUpdate(m_pSimpleFaceMeshVt, ImagePointVtCoord, m_numLandmark);
		FaceCourtonLineVtInterpolation(m_pSimpleFaceMeshVt,m_numLandmark);
		m_pSimpleFaceMeshVt[2 * 16] = (m_pSimpleFaceMeshVt[2 * 15] + m_pSimpleFaceMeshVt[2 * 17]) / 2.0f;
		m_pSimpleFaceMeshVt[2 * 16 + 1] = (m_pSimpleFaceMeshVt[2 * 15 + 1] + m_pSimpleFaceMeshVt[2 * 17 + 1]) / 2.0f;
		//Vt插值
		FaceInterlation::RunFacePointInterpolation(m_pSimpleFaceMeshVt, m_numLandmark, m_numOfSimpleFaceMeshV, m_IsNonLine, 1.15f, 0.75f,true, m_pSimpleFaceMeshVt);
		delete[] imagePoint;
		delete[] ImagePointVtCoord;
		return true;
	}


}