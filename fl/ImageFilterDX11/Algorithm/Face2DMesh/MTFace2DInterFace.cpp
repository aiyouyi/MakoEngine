#include "MTFace2DInterFace.h"
#include "SimpleFaceMesh.h"
#include "SimpleFaceBackMesh.h"
#include "MultiSimpleFaceBackMesh.h"


namespace mt3dface {


	MTFace2DInterFace::MTFace2DInterFace()
	{
		m_pFace2DMesh = NULL;
		m_p2DFaceReconstruct = NULL;

		m_p2DFaceBackReconstruct = NULL;
		m_p2DMultiFaceBackReconstruct = NULL;
		// 标准脸UV
		std::vector<float> g_pStandPoint={
           0.180591f,0.352581f,0.182117f,0.37894f,0.185771f,0.405241f,0.19279f,0.431432f,0.202267f,0.457172f,0.213422f,0.48246f,0.225277f,0.507714f,0.239108f,0.532907f,0.25664f,0.557213f,0.278975f,0.579447f,0.304777f,0.59969f,0.333094f,0.618827f,0.363049f,0.636694f,0.394268f,0.653326f,0.427431f,0.667875f,0.465155f,0.677975f,0.506845f,0.681396f,0.548604f,0.67799f,0.586168f,0.667823f,0.618782f,0.652715f,0.648554f,0.635034f,0.676153f,0.615686f,0.701234f,0.594832f,0.723343f,0.572732f,0.742354f,0.549226f,0.757874f,0.524237f,0.770734f,0.498994f,0.782624f,0.473665f,0.793999f,0.447939f,0.803461f,0.421858f,0.810005f,0.395184f,0.812672f,0.368673f,0.812348f,0.342259f,0.229422f,0.319736f,0.268908f,0.298774f,0.319077f,0.295833f,0.369549f,0.300119f,0.416689f,0.309176f,0.411439f,0.325462f,0.366406f,0.319434f,0.318702f,0.315448f,0.27139f,0.316069f,0.583718f,0.306111f,0.630051f,0.295724f,0.679678f,0.289784f,0.729568f,0.291399f,0.770194f,0.310335f,0.728727f,0.308686f,0.681931f,0.309432f,0.6346f,0.314977f,0.590204f,0.322282f,0.292466f,0.365252f,0.321857f,0.351073f,0.359532f,0.348052f,0.395929f,0.356709f,0.419756f,0.376742f,0.385781f,0.379987f,0.351205f,0.381802f,0.318565f,0.376316f,0.357285f,0.361351f,0.355413f,0.365331f,0.593919f,0.372723f,0.61574f,0.353818f,0.649971f,0.34439f,0.686796f,0.345336f,0.716619f,0.356866f,0.693408f,0.368934f,0.66279f,0.375389f,0.627985f,0.374761f,0.655093f,0.356507f,0.656632f,0.360263f,0.503272f,0.357453f,0.504374f,0.398275f,0.505832f,0.439024f,0.506779f,0.47884f,0.45667f,0.368367f,0.45063f,0.458778f,0.424194f,0.489935f,0.447584f,0.507488f,0.46672f,0.500445f,0.506572f,0.512437f,0.546887f,0.498947f,0.56596f,0.506109f,0.587889f,0.487906f,0.561613f,0.457604f,0.552091f,0.366943f,0.402811f,0.559528f,0.438168f,0.553551f,0.478387f,0.548394f,0.506428f,0.552677f,0.534842f,0.548266f,0.573915f,0.55355f,0.608783f,0.560054f,0.584131f,0.578621f,0.550018f,0.591217f,0.506594f,0.594961f,0.463051f,0.590709f,0.429071f,0.577699f,0.417408f,0.561486f,0.445694f,0.563785f,0.50677f,0.567773f,0.56769f,0.564066f,0.594896f,0.562235f,0.568025f,0.565364f,0.506543f,0.569561f,0.445542f,0.565036f,0.498321f,0.124978f,0.357892f,0.144034f,0.258128f,0.18832f,0.199027f,0.257836f,0.63625f,0.141129f,0.734564f,0.182727f,0.793263f,0.24977f,0.199712f,0.30884f,0.216498f,0.26807f,0.263518f,0.243547f,0.361739f,0.195542f,0.365586f,0.24705f,0.471383f,0.185763f,0.444444f,0.246549f,0.526502f,0.184752f,0.554683f,0.244526f,0.634204f,0.192146f,0.632158f,0.243162f,0.732066f,0.237063f,0.761416f,0.270584f,0.794653f,0.299594f,0.236529f,0.358916f,0.764483f,0.349562f,0.477502f,0.428527f,0.448629f,0.402634f,0.384015f,0.407204f,0.416826f,0.432606f,0.335937f,0.406397f,0.379407f,0.447543f,0.232877f,0.388001f,0.283638f,0.397062f,0.254373f,0.436748f,0.315957f,0.442065f,0.257749f,0.465363f,0.313231f,0.473553f,0.368713f,0.481744f,0.297085f,0.51524f,0.368892f,0.522766f,0.4407f,0.530292f,0.379804f,0.579097f,0.356798f,0.598666f,0.32468f,0.543801f,0.410252f,0.554694f,0.318222f,0.568155f,0.532994f,0.427939f,0.563457f,0.400331f,0.629402f,0.40252f,0.596013f,0.42965f,0.674138f,0.400109f,0.631657f,0.443353f,0.766956f,0.379047f,0.72124f,0.389421f,0.746766f,0.428951f,0.69007f,0.436045f,0.742471f,0.457931f,0.690944f,0.467923f,0.639416f,0.477914f,0.704471f,0.50917f,0.638208f,0.519346f,0.571946f,0.529521f,0.631015f,0.578412f,0.653247f,0.596771f,0.679677f,0.539102f,0.60148f,0.553967f,0.686685f,0.563819f,0.301789f,0.359169f,0.311853f,0.354373f,0.334208f,0.34864f,0.346499f,0.347703f,0.372992f,0.349641f,0.385083f,0.352507f,0.405158f,0.36207f,0.413141f,0.368768f,0.600101f,0.365222f,0.607547f,0.358833f,0.62623f,0.349388f,0.637468f,0.346334f,0.66237f,0.343403f,0.674253f,0.343642f,0.696469f,0.347801f,0.706802f,0.35172f,0.415272f,0.557265f,0.427786f,0.555164f,0.452978f,0.551453f,0.465656f,0.549842f,0.487808f,0.550788f,0.497225f,0.552215f,0.516043f,0.552173f,0.525445f,0.550702f,0.547344f,0.549733f,0.559774f,0.551399f,0.584421f,0.555328f,0.596638f,0.557591f,0.601221f,0.567118f,0.592766f,0.573434f,0.573177f,0.583822f,0.562044f,0.587893f,0.535494f,0.593353f,0.520982f,0.594601f,0.491993f,0.594432f,0.477516f,0.593014f,0.451162f,0.587099f,0.440013f,0.582855f,0.419933f,0.572462f,0.411002f,0.566312f,0.432271f,0.56273f,0.462033f,0.564983f,0.476933f,0.565991f,0.491845f,0.566921f,0.521468f,0.566928f,0.536163f,0.566052f,0.550853f,0.565144f,0.580219f,0.563236f,0.580279f,0.564036f,0.550914f,0.566942f,0.536167f,0.568047f,0.521377f,0.56892f,0.491559f,0.568845f,0.476626f,0.567877f,0.461745f,0.566657f,0.432136f,0.563461f,0.473607f,0.529412f,0.506515f,0.528533f,0.53923f,0.529027f,0.401531f,0.609775f,0.452474f,0.630445f,0.506719f,0.638178f,0.560831f,0.630588f,0.610866f,0.609428f        
        };
        m_pStandPoint = new float[g_pStandPoint.size()];
		memcpy(m_pStandPoint,g_pStandPoint.data(),g_pStandPoint.size()*sizeof(float));
	}

	MTFace2DInterFace::~MTFace2DInterFace()
	{
		SAFE_DELETE(m_p2DFaceReconstruct);
		SAFE_DELETE(m_pFace2DMesh);
		SAFE_DELETE_ARRAY(m_pStandPoint);

		SAFE_DELETE(m_p2DFaceBackReconstruct);
		SAFE_DELETE(m_p2DMultiFaceBackReconstruct)
	}


	MTFace2DMesh *MTFace2DInterFace::Get2DMesh(float *pFacePoint106, int nWidth, int nHeight,float pitchAngle,eReconstruct2DMode Reconstruct2DMode)
	{
		if (Reconstruct2DMode == MT_FACE_25D)
		{
			if (m_p2DFaceReconstruct == NULL)
			{
				SAFE_DELETE(m_pFace2DMesh);
				m_p2DFaceReconstruct = new SimpleFaceMesh();
				m_pFace2DMesh = new MTFace2DMesh();
			}

			if (pFacePoint106==NULL||nWidth<1||nHeight<1)
			{
				return NULL;
			}

			m_p2DFaceReconstruct->Run(pFacePoint106, nWidth, nHeight,pitchAngle);
			// 主播人脸的三角网格顶点
			m_pFace2DMesh->pVertexs = m_p2DFaceReconstruct->GetSimpleFaceMeshV();
			// 主播人脸的UV点
			m_pFace2DMesh->pTextureCoordinates = m_p2DFaceReconstruct->GetSimpleFaceMeshVt();
			// 主播人脸三角网格顶点个数
			m_pFace2DMesh->nVertex = m_p2DFaceReconstruct->GetNumOfSimpleFaceMeshV();
			m_pFace2DMesh->pTriangleIndex = m_p2DFaceReconstruct->GetSimpleFaceMeshTriIndex();
			m_pFace2DMesh->nTriangle = m_p2DFaceReconstruct->GetNumOfSimpleFaceMeshTri();
			return m_pFace2DMesh;
		}
		else if(Reconstruct2DMode == MT_FACE_2D_BACKGROUND)
		{
			if (m_p2DFaceBackReconstruct == NULL)
			{
				SAFE_DELETE(m_pFace2DMesh);
				m_p2DFaceBackReconstruct = new SimpleFaceBackMesh();
				m_pFace2DMesh = new MTFace2DMesh();
			}

			if (pFacePoint106==NULL||nWidth<1||nHeight<1)
			{
				return NULL;
			}

			m_p2DFaceBackReconstruct->Run(pFacePoint106, nWidth, nHeight,pitchAngle);
			m_pFace2DMesh->pVertexs = m_p2DFaceBackReconstruct->GetSimpleFaceMeshV();
			m_pFace2DMesh->pTextureCoordinates = m_p2DFaceBackReconstruct->GetSimpleFaceMeshVt();
			m_pFace2DMesh->nVertex = m_p2DFaceBackReconstruct->GetNumOfSimpleFaceMeshV();
			m_pFace2DMesh->pTriangleIndex = m_p2DFaceBackReconstruct->GetSimpleFaceMeshTriIndex();
			m_pFace2DMesh->nTriangle = m_p2DFaceBackReconstruct->GetNumOfSimpleFaceMeshTri();
			return m_pFace2DMesh;
		}
		else
		{
			if (m_p2DMultiFaceBackReconstruct == NULL)
			{
				SAFE_DELETE(m_pFace2DMesh);
				m_p2DMultiFaceBackReconstruct = new MultiSimpleFaceBackMesh();
				m_pFace2DMesh = new MTFace2DMesh();
			}

			if (pFacePoint106==NULL||nWidth<1||nHeight<1)
			{
				return NULL;
			}

			m_p2DMultiFaceBackReconstruct->Run(pFacePoint106, nWidth, nHeight,pitchAngle);
			m_pFace2DMesh->pVertexs = m_p2DMultiFaceBackReconstruct->GetSimpleFaceMeshV();
			m_pFace2DMesh->pTextureCoordinates = m_p2DMultiFaceBackReconstruct->GetSimpleFaceMeshVt();
			m_pFace2DMesh->nVertex = m_p2DMultiFaceBackReconstruct->GetNumOfSimpleFaceMeshV();
			m_pFace2DMesh->pTriangleIndex = m_p2DMultiFaceBackReconstruct->GetSimpleFaceMeshTriIndex();
			m_pFace2DMesh->nTriangle = m_p2DMultiFaceBackReconstruct->GetNumOfSimpleFaceMeshTri();
			return m_pFace2DMesh;
		}
		

	}
	float *MTFace2DInterFace::GetStandVerts()
	{
		return m_pStandPoint;
	}
}