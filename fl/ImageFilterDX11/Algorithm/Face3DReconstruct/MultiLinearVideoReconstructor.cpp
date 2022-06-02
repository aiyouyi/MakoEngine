#include "MultiLinearVideoReconstructor.h"

#include "Eigen/Geometry"
#include "Eigen/Dense"
#include "glm/gtc/matrix_transform.hpp"
#include <map>
#include "BaseDefine/Define.h"
#ifdef PLATFORM_WINDOWS
#include <io.h>
#define access _access
#else
#include <unistd.h>
#endif

namespace mt3dface
{
using namespace Eigen;

void CalMeanRepresentativeFrame(vector<RepresentativeFrame>&v)
{
	int nFrame = v.size();
	for (int i = 0; i < MLVR_REPRENTATIVE_FRAME; i++)
	{
		float meanVec = 0.f;
		for (int j = 1; j < nFrame; j++)
		{
			meanVec += v[j].Rotation_Express_vec[i];
		}
		meanVec /= (nFrame - 1);
		v[0].Rotation_Express_vec[i] = meanVec;
	}

}

float CalFrameProjectErr(float *pMMt, float *pMeanV, float *Rotation_Express_vec)
{
	float pTmp1[MLVR_REPRENTATIVE_FRAME], pTmp2[MLVR_REPRENTATIVE_FRAME];
	for (int i = 0; i < MLVR_REPRENTATIVE_FRAME; i++)
	{
		pTmp1[i] = Rotation_Express_vec[i] - pMeanV[i];
	}
	for (int i = 0; i < MLVR_REPRENTATIVE_FRAME; i++)
	{
		float value = 0.f;
		for (int j = 0; j < MLVR_REPRENTATIVE_FRAME; j++)
		{
			value += *(pMMt++)*pTmp1[j];
		}
		pTmp2[i] = value;
	}
	for (int i = 0; i < MLVR_REPRENTATIVE_FRAME; i++)
	{
		pTmp1[i] = Rotation_Express_vec[i] - (pTmp2[i] + pMeanV[i]);
	}
	float Value = 0.f;
	for (int i = 0; i < MLVR_REPRENTATIVE_FRAME; i++)
	{
		Value += pTmp1[i] * pTmp1[i];
	}
	//	printf("project err Value = %f\n", Value);
	return Value;

}

void CalEigenVectorMat(vector<RepresentativeFrame>&v, float*pMMt)
{
	int nFrame = v.size() - 1;
	MatrixXf A = MatrixXf::Zero(MLVR_REPRENTATIVE_FRAME, nFrame);
	float *pA = A.data();
	float pVtmp[MLVR_REPRENTATIVE_FRAME];
	for (int i = 0; i < nFrame; i++)
	{
		for (int j = 0; j < MLVR_REPRENTATIVE_FRAME; j++)
		{
			pVtmp[j] = v[i + 1].Rotation_Express_vec[j] - v[0].Rotation_Express_vec[j];
		}
		memcpy(pA + i*MLVR_REPRENTATIVE_FRAME, pVtmp, MLVR_REPRENTATIVE_FRAME*sizeof(float));
	}
	//PCA ,keep 95%
	MatrixXf CovMat = A*A.transpose();
	SelfAdjointEigenSolver<MatrixXf> eig(CovMat);
	MatrixXf EigenVector = eig.eigenvectors();
	MatrixXf EigenVlaue = eig.eigenvalues();
	float sumValue = 0.f;
	float sumEigenValue = EigenVlaue.sum();
	int dim = 0;
	if (nFrame == 1)
	{
		dim = 1;
	}
	else
	{
		for (int i = MLVR_REPRENTATIVE_FRAME - 1; i >= 0; i--)
		{
			sumValue += EigenVlaue(i, 0);
			if (sumValue / sumEigenValue > 0.95f)
			{
				dim = EigenVlaue.rows() - i;
				break;
			}
		}
	}

	MatrixXf pca = EigenVector.rightCols(dim);

	MatrixXf MtM = pca.transpose()*pca;
	float *pMtM = MtM.data();
	MatrixXf MMt = pca*pca.transpose();
	//printf("pca dim = %d\n", dim);
	memcpy(pMMt, MMt.data(), MLVR_REPRENTATIVE_FRAME*MLVR_REPRENTATIVE_FRAME*sizeof(float));

}

MultiLinearVideoReconstructor::MultiLinearVideoReconstructor()
{
	m_pLanmarkVertex = NULL;
	m_pContourVertex = NULL;
	m_pAdjustModel3D = NULL;
	m_pExpressDim = NULL;
	m_pLanmarkExpressBasis = NULL;;
	m_nFrame = 1;
	m_nGetPersepectCount = 1;
	m_bFixID = false;
	memset(m_fModelView, 0, sizeof(float)*16);
	memset(m_fModelViewPerspect, 0, sizeof(float)*16);
	m_pMMt = NULL;//new float[MLVR_REPRENTATIVE_FRAME*MLVR_REPRENTATIVE_FRAME];
    m_pLanmarkVertex = NULL;// new float[3 * MLM_LANMARK_INDEX];
    m_pContourVertex = NULL;// new float[3 * MLM_CONTOUR_INDEX];
    m_pAdjustModel3D = NULL;// new float[3 * MLM_FACE_3D_UV_VERTEX];
    m_pExpressDim = NULL;// new float[3 * MLM_FACE_3D_VERTEX*MLM_EXPRESS_DIMS];
    m_pLanmarkExpressBasis =  NULL;//new float[3 * MLM_LANMARK_INDEX*MLM_EXPRESS_DIMS];
    
    model = NULL;
    m_bHasLoadModel = false;

	for(int i=0;i<MLVR_Smooth3DFace;i++)
	{
		m_pAdjustExpress3DN[i] =  NULL;//new float[MLM_FACE_3D_UV_VERTEX*3];
	}

	m_pNeuFace3D = NULL;

}


MultiLinearVideoReconstructor::~MultiLinearVideoReconstructor()
{
	ClearData();
    if (m_bHasLoadModel)
    {
        SAFE_DELETE(model);
    }
}

void MultiLinearVideoReconstructor::InitData()
{
	m_pMMt = new float[MLVR_REPRENTATIVE_FRAME*MLVR_REPRENTATIVE_FRAME];
    m_pLanmarkVertex = new float[3 * MLM_LANMARK_INDEX];
    m_pContourVertex = new float[3 * MLM_CONTOUR_INDEX];
    m_pAdjustModel3D = new float[3 * MLM_FACE_3D_UV_VERTEX];
    m_pExpressDim = new float[3 * MLM_FACE_3D_VERTEX*MLM_EXPRESS_DIMS];
    m_pLanmarkExpressBasis = new float[3 * MLM_LANMARK_INDEX*MLM_EXPRESS_DIMS];
	for(int i=0;i<MLVR_Smooth3DFace;i++)
	{
		m_pAdjustExpress3DN[i] = new float[MLM_FACE_3D_UV_VERTEX*3];
	}
}

void MultiLinearVideoReconstructor::ClearData()
{
	SAFE_DELETE_ARRAY(m_pExpressDim);
	SAFE_DELETE_ARRAY(m_pLanmarkExpressBasis);
	SAFE_DELETE_ARRAY(m_pLanmarkVertex);
	SAFE_DELETE_ARRAY(m_pContourVertex);
	SAFE_DELETE_ARRAY(m_pAdjustModel3D);
	SAFE_DELETE_ARRAY(m_pMMt);
	for(int i=0;i<MLVR_Smooth3DFace;i++)
	{
		SAFE_DELETE_ARRAY(m_pAdjustExpress3DN[i]);
	}
	SAFE_DELETE_ARRAY(m_pNeuFace3D);
}

bool MultiLinearVideoReconstructor::LoadModel(const char *configureFloder)
{
    //涓轰簡闃叉璺緞澶暱
    char* pAllPath = new char[1024*5];
    char* ModelFile = pAllPath;
    char* ContourFile = ModelFile + 1024;
    char* LanmarkFile = ContourFile + 1024;
    char* TempFile = LanmarkFile + 1024;
    char* MatFile = TempFile + 1024;
    
	sprintf(ModelFile, "%s/ModelCore.bin", configureFloder);
	sprintf(ContourFile, "%s/ContourVertex.bin", configureFloder);
	sprintf(LanmarkFile, "%s/Lanmark.bin", configureFloder);
	sprintf(TempFile, "%s/UVmap_3DObj.bin", configureFloder);
	sprintf(MatFile, "%s/ExpressMat_InitParam.bin", configureFloder);
	if (access(ModelFile, 0) != 0 || access(ContourFile, 0) != 0 || access(LanmarkFile, 0) != 0 || access(TempFile, 0) != 0 || access(MatFile, 0) != 0)
	{
        SAFE_DELETE_ARRAY(pAllPath);
		return false;
	}
    model = new MultiLinear3DModel();
    model->LoadModel(ModelFile, ContourFile, LanmarkFile, TempFile, MatFile);
    m_bHasLoadModel = true;
    SAFE_DELETE_ARRAY(pAllPath);
	return true;
}

bool MultiLinearVideoReconstructor::InitModel(MultiLinear3DModel *pExternModel)
{
	model = pExternModel;
	m_bHasLoadModel = false;
	return model != NULL;
}

bool MultiLinearVideoReconstructor::Run(float *pImagePoint106, int nWidth, int nHeight, int nFaceWidth, int nFaceHeight, bool isFirstFrame /* = false */, int nMaxIter,
	int nSmoothModel /* = 3 */, bool isUse47Express /* = false */)
{
	if (pImagePoint106 == NULL || nFaceWidth < 1 || nFaceHeight < 1)
	{
		return false;
	}
	if(m_pAdjustModel3D == NULL)
	{
		InitData();
	}
	m_nHeight = nHeight;
	m_nWidth = nWidth;
	m_nFaceWidth = nFaceWidth;
	m_nFaceHeight = nFaceHeight;
	m_bUse47Express = isUse47Express;
	m_nMaxIter = nMaxIter;

	if (m_nMaxIter<2)
	{
		m_nMaxIter = 2;
	}

	if (isFirstFrame)
	{
		m_nFrame = 1;
		m_bFixID = true;
		m_RepresentativeFrame.clear();
		RepresentativeFrame meanRF;
		m_RepresentativeFrame.push_back(meanRF);
		memcpy(m_pFitCoeffsID, model->GetIdentityInitParam(), sizeof(float)*MLM_IDENTITY_DIMS);
		UpdateExpressDim();
		m_bFitIdentity = true;
		m_nGetPersepectCount = 1;
		//OutObjIdentityNeutral(m_pFitCoeffsID);

		m_maxEyeAngle = sin(60 * 3.1415926 / 180.f);
		m_minEyeAngle = sin(5 * 3.1415926 / 180.f);
		m_UserMaxEyeAngle = sin(30 * 3.1415926 / 180.f);
		m_UserMinEyeAngle = sin(15 * 3.1415926 / 180.f);

	}
	else
	{
		m_nFrame++;
	}
	memcpy(m_pFitCoeffsExpress, model->GetExpressInitParam(), sizeof(float)*MLM_EXPRESS_DIMS);

	for (int i = 0; i < MLM_LANMARK_INDEX; i++)
	{
		m_pWeightLanMark[i] = 1.0f;
	}
	memcpy(m_pIndex3D, model->Get3DIndex(), sizeof(short3D)*MLM_LANMARK_INDEX);
	short3D *pIndex2D = model->Get2DIndex();
	for (int i = 0; i < MLM_LANMARK_INDEX; i++)
	{
		int i2 = i * 2;
		int index2 = pIndex2D[i] * 2;
		m_pImagePoint2D[i2] = pImagePoint106[index2];
#if MLVR_USE_PERPECTIVE_PROJECT
		m_pImagePoint2D[i2 + 1] = nHeight - pImagePoint106[index2 + 1];
#else
		m_pImagePoint2D[i2 + 1] = pImagePoint106[index2 + 1];
#endif
	}

	Vector2 *FacePoint = (Vector2*)pImagePoint106;
	m_LeftEyePoint[0] = FacePoint[53];
	m_LeftEyePoint[1] = FacePoint[55];
	m_LeftEyePoint[2] = FacePoint[57];

	m_RightEyePoint[0] = FacePoint[63];
	m_RightEyePoint[1] = FacePoint[61];
	m_RightEyePoint[2] = FacePoint[67];


	RunShape_Express();
	GenerateModel();
	CalGL_MVP();
	SmoothFace(nSmoothModel);
    
    float maxId = -100.f;
    for (int i = 0; i < MLM_IDENTITY_DIMS; i++)
    {
        float bin_val = m_pFitCoeffsID[i];
        maxId = (std::max)(maxId, bin_val);
    }
    if (maxId >0.4f)
    {
        m_nFrame = 1;
        m_bFixID = true;
        m_RepresentativeFrame.clear();
        RepresentativeFrame meanRF;
        m_RepresentativeFrame.push_back(meanRF);
        memcpy(m_pFitCoeffsID, model->GetIdentityInitParam(), sizeof(float)*MLM_IDENTITY_DIMS);
        UpdateExpressDim();
    }
	return true;
}

void MultiLinearVideoReconstructor::OrthoToPerspectMVP(float FovAngle,int SmoothFrame )
{
	int nSmooth = SmoothFrame;
	if (nSmooth > MLVR_Smooth3DFace)
	{
		nSmooth = 5;
	}

	short3D *pIndex3D = m_pIndex3D;
	float *pFace3D = m_pAdjustModel3D;
	float pModelPoint[MLM_LANMARK_INDEX * 3];
	float pImage2D[MLM_LANMARK_INDEX * 2];
	float pWeight[MLM_LANMARK_INDEX];
	memcpy(pImage2D, m_pImagePoint2D, sizeof(float)*MLM_LANMARK_INDEX * 2);
	for (int i = 0; i < MLM_LANMARK_INDEX; i++)
	{
		pImage2D[i * 2 + 1] = m_nHeight - pImage2D[i * 2 + 1];

		pModelPoint[i * 3] = pFace3D[pIndex3D[i] * 3];
		pModelPoint[i * 3 + 1] = pFace3D[pIndex3D[i] * 3 + 1];
		pModelPoint[i * 3 + 2] = pFace3D[pIndex3D[i] * 3 + 2];

		pWeight[i] = 1.0f;
	}

	bool isInit = true;

	if(m_nGetPersepectCount == 1 )
	{
		float faceRate = m_nWidth *0.5f / m_nFaceWidth;
	    memset(m_pCameraParamPerspect[MLVR_Smooth3DFace], 0, 6 * sizeof(float));

		float fovy = FovAngle*3.14159f / 180.f;
	    m_pCameraParamPerspect[MLVR_Smooth3DFace][5] = -1.0f *faceRate/ tan(fovy * 0.5);
	}


	m_PEC.EstimateProMat(pModelPoint, pImage2D, MLM_LANMARK_INDEX, m_nWidth, m_nHeight, m_fModelViewPerspect,
		m_pWeightLanMark, m_pCameraParamPerspect[MLVR_Smooth3DFace], isInit,FovAngle);


	if (fabs(m_pCameraParamPerspect[MLVR_Smooth3DFace][0]) > 90|| fabs(m_pCameraParamPerspect[MLVR_Smooth3DFace][1]) > 90|| fabs(m_pCameraParamPerspect[MLVR_Smooth3DFace][2]) > 90)
	{
		float faceRate = m_nWidth * 0.5f / m_nFaceWidth;
		memset(m_pCameraParamPerspect[MLVR_Smooth3DFace], 0, 6 * sizeof(float));

		float fovy = FovAngle * 3.14159f / 180.f;
		m_pCameraParamPerspect[MLVR_Smooth3DFace][5] = -1.0f *faceRate / tan(fovy * 0.5);
	}

	for (int i = MLVR_Smooth3DFace-1; i > 0; i--)
	{
		memcpy(m_pCameraParamPerspect[i], m_pCameraParamPerspect[i - 1], 6 * sizeof(float));
	}
	memcpy(m_pCameraParamPerspect[0], m_pCameraParamPerspect[MLVR_Smooth3DFace], 6 * sizeof(float));

	int MinSmooth = nSmooth < m_nGetPersepectCount ? nSmooth : m_nGetPersepectCount;
	m_nGetPersepectCount ++;
    float pCameraParamSmooth[6] = { 0 };
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < MinSmooth; j++)
		{
			pCameraParamSmooth[i] += m_pCameraParamPerspect[j][i];
		}
		pCameraParamSmooth[i] /= MinSmooth;
	}
	memcpy( m_pCameraParamPerspect[MLVR_Smooth3DFace], pCameraParamSmooth, 6 * sizeof(float));
	glm::mat4 project = glm::perspective(FovAngle *3.14159f / 180.f, m_nWidth*1.f / m_nHeight, -pCameraParamSmooth[5]-3 , -pCameraParamSmooth[5]+2);
	auto rot_mtx_x = glm::rotate(glm::mat4(1.0f), pCameraParamSmooth[0]*3.14159f / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));
	auto rot_mtx_y = glm::rotate(glm::mat4(1.0f), pCameraParamSmooth[1]*3.14159f / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
	auto rot_mtx_z = glm::rotate(glm::mat4(1.0f), pCameraParamSmooth[2]*3.14159f / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));
	auto t_mtx = glm::translate(glm::mat4(1.0f), glm::vec3(pCameraParamSmooth[3], pCameraParamSmooth[4], pCameraParamSmooth[5]));
	auto projectView = project*t_mtx*rot_mtx_y*rot_mtx_x*rot_mtx_z;
	projectView = glm::transpose(projectView);
	float *full_projection_4x4 = &projectView[0][0];
	memcpy(m_fModelViewPerspect, full_projection_4x4, 16 * sizeof(float));
	memcpy(m_fModelViewPerspect+16,pCameraParamSmooth,6*sizeof(float));

// 	m_fModelViewPerspect[4] *= -1.f;
// 	m_fModelViewPerspect[5] *= -1.f;
// 	m_fModelViewPerspect[6] *= -1.f;
// 	m_fModelViewPerspect[7] *= -1.f;
}


void MultiLinearVideoReconstructor::CalGL_MVP()
{
#if MLVR_USE_PERPECTIVE_PROJECT
	memcpy(m_fModelView, m_pProjectionMat, 16 * sizeof(float));
	m_fModelView[4] *= -1.f;
	m_fModelView[5] *= -1.f;
	m_fModelView[6] *= -1.f;
	m_fModelView[7] *= -1.f;
#else
	memcpy(m_fModelView, m_pProjectionMat, 12 * sizeof(float));
	float aspect = m_nWidth*1.0f / m_nHeight;
	m_fModelView[3] -= (m_nWidth / 2);
	m_fModelView[7] -= (m_nHeight / 2);
	m_fModelView[0] /= aspect;
	m_fModelView[1] /= aspect;
	m_fModelView[2] /= aspect;
	m_fModelView[3] /= aspect;
	for (int i=0;i<8;i++)
	{
		m_fModelView[i] /= (m_nHeight / 2);
	}
	for(int i=8;i<12;i++)
	{
		m_fModelView[i]*=0.33f;
	}

	m_fModelView[15] = 1.0f;
#endif


}

void MultiLinearVideoReconstructor::SmoothFace(int nSmoothFrame)
{
	//Smooth the model
	if (nSmoothFrame > 5)
	{
		nSmoothFrame = 5;
	}
	if (nSmoothFrame < 1)
	{
		nSmoothFrame = 1;
	}
	for (int i = nSmoothFrame - 1; i > 0; i--)
	{
		if (m_nFrame - 1 > i)
		{
			memcpy(m_pAdjustExpress3DN[i], m_pAdjustExpress3DN[i - 1], sizeof(float) * MLM_FACE_3D_UV_VERTEX * 3);
			memcpy(m_pFitCoeffsExpress47N[i], m_pFitCoeffsExpress47N[i - 1], sizeof(float) * 47);
		}
	}
	memcpy(m_pAdjustExpress3DN[0], m_pAdjustModel3D, sizeof(float) * MLM_FACE_3D_UV_VERTEX * 3);
	memcpy(m_pFitCoeffsExpress47N[0], m_pFitCoeffsExpress47, sizeof(float) * 47);
	if (m_nFrame > nSmoothFrame)
	{
		for (int i = 0; i < MLM_FACE_3D_UV_VERTEX * 3; i++)
		{
			float sum = 0.0f;
			for (int j = 0; j < nSmoothFrame; j++)
			{
				sum += m_pAdjustExpress3DN[j][i];
			}
			m_pAdjustModel3D[i] = sum / nSmoothFrame;
		}
		for (int i = 0; i < 47; i++)
		{
			float sum = 0.0f;
			for (int j = 0; j < nSmoothFrame; j++)
			{
				sum += m_pFitCoeffsExpress47N[j][i];
			}
			m_pFitCoeffsExpress47[i] = sum / nSmoothFrame;
		}
		Vector2 L1 = (m_LeftEyePoint[0] - m_LeftEyePoint[1]).normalize();
		Vector2 L2 = (m_LeftEyePoint[2] - m_LeftEyePoint[1]).normalize();
		float cosa = L1.dot(L2);
		float sina = sqrt(1 - cosa * cosa);

		Vector2 R1 = (m_RightEyePoint[0] - m_RightEyePoint[1]).normalize();
		Vector2 R2 = (m_RightEyePoint[2] - m_RightEyePoint[1]).normalize();
		float cosb = R1.dot(R2);
		float sinb = sqrt(1 - cosb * cosb);

		m_UserMaxEyeAngle = std::max(sina, m_UserMaxEyeAngle);
		m_UserMaxEyeAngle = std::max(sinb, m_UserMaxEyeAngle);
		m_UserMaxEyeAngle = std::min(m_maxEyeAngle, m_UserMaxEyeAngle);

		m_UserMinEyeAngle = std::min(sina, m_UserMinEyeAngle);
		m_UserMinEyeAngle = std::min(sinb, m_UserMinEyeAngle);
		m_UserMinEyeAngle = std::max(m_minEyeAngle, m_UserMinEyeAngle);


		if (sina <m_UserMinEyeAngle+0.1)
		{
			m_pFitCoeffsExpress47[2] = 1.0;
		}
		else if(sina>m_UserMaxEyeAngle-0.1)
		{
			m_pFitCoeffsExpress47[2] = 0.0;
		}
		else
		{
			m_pFitCoeffsExpress47[2] =  (m_UserMaxEyeAngle - sina -0.2 ) / (m_UserMaxEyeAngle - m_UserMinEyeAngle -0.2);
		}

		if (sinb < m_UserMinEyeAngle + 0.1)
		{
			m_pFitCoeffsExpress47[1] = 1.0;
		}
		else if (sinb > m_UserMaxEyeAngle - 0.1)
		{
			m_pFitCoeffsExpress47[1] = 0.0;
		}
		else
		{
			m_pFitCoeffsExpress47[1] =(m_UserMaxEyeAngle - sinb-0.2) / (m_UserMaxEyeAngle - m_UserMinEyeAngle-0.2);
		}

		//add trick
		for (int i=1;i<3;i++)
		{
			if (m_pFitCoeffsExpress47[i] < 0.3)
			{
				m_pFitCoeffsExpress47[i] = 0;
			}
			else if (m_pFitCoeffsExpress47[i] > 0.6)
			{
				m_pFitCoeffsExpress47[i] = 1;
			}
			else
			{
				m_pFitCoeffsExpress47[i] = (m_pFitCoeffsExpress47[i] - 0.3) * 3.33;
			}
		}



		m_pFitCoeffsExpress47[0] = 1.0;
		for (int j = 0; j < 46; j++)
		{
			m_pFitCoeffsExpress47[j + 1] = std::max(-0.2f, m_pFitCoeffsExpress47[j + 1]);
			m_pFitCoeffsExpress47[0] -= m_pFitCoeffsExpress47[j+1];
		}

	}
}


void MultiLinearVideoReconstructor::RunShape_Express()
{
	float *pModelPoint = GetLanmarkVertex(m_pFitCoeffsExpress);
	float *pImagePoint = m_pImagePoint2D;
	RunEstimateProjectMat(pModelPoint, pImagePoint, 1);
	int MAXwh = (std::max)(m_nFaceWidth, m_nFaceHeight);
	float fLamda = MAXwh*1.2f / 200;

	for (int i = 0; i < m_nMaxIter; i++)
	{
		RePosFitBlock(m_pIndex3D, pImagePoint, i);
		//閲嶆柊濮挎€侀浼?		
		pModelPoint = GetLanmarkVertex(m_pFitCoeffsExpress);
		RunEstimateProjectMat(pModelPoint, pImagePoint, i + 2);
		if (m_bUse47Express)
		{
			RunExpress47(m_pImagePoint2D, fLamda);
		}
		else
		{
			RunExpress(m_pImagePoint2D, fLamda);
		}
	}
	if (AddNewFrameToSet())
	{
		RunMultiFrameShape(fLamda);
		m_bFitIdentity = true;
		//OutObjIdentityNeutral(m_pFitCoeffsID);
		//	OutBlendShape(m_pFitCoeffsID);
	}
	else if(m_nFrame>1)
	{
		m_bFitIdentity = false;
	}
}

void MultiLinearVideoReconstructor::RunExpress(float*pImagePoint2D, float Lamda)
{
	//Fit琛ㄦ儏
	float *m_pExpParam = model->GetExpressInitParam();
	float *pModelPoint = GetLanmarkVertex(m_pExpParam);
	float *pExpressBasis = GetAndUpdateExpressDim();
	Fitting(pModelPoint, pExpressBasis, pImagePoint2D, MLM_LANMARK_INDEX, MLM_EXPRESS_DIMS, m_pProjectionMat, m_pFitCoeffsExpress, Lamda * 20);
	for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
	{
		m_pFitCoeffsExpress[j] += m_pExpParam[j];
	}
}

void MultiLinearVideoReconstructor::RunExpress47(float*pImagePoint2D, float Lamda)
{
	float *m_pExpParam = model->GetExpressInitParam();
	float *pModelPoint = GetLanmarkVertex(m_pExpParam);
	float *pExpressBasis = GetAndUpdateExpressDim();

	float *pExpress47Dim = new float[47 * MLM_LANMARK_INDEX * 3];
	float pExpress47[47];
	ExpressDim25to47(pExpressBasis, pExpress47Dim);
	for (int j = 0; j < 47; j++)
	{
		m_pFitCoeffsExpress47[j] = 0.f;
	}
	m_pFitCoeffsExpress47[0] = 1.f;

	Fitting(pModelPoint, pExpress47Dim, pImagePoint2D, MLM_LANMARK_INDEX, 46, m_pProjectionMat, pExpress47, Lamda * 20);

	int nParam = 0;
	for (int j = 0; j < 46; j++)
	{
		m_pFitCoeffsExpress47[0] -= pExpress47[nParam];
		m_pFitCoeffsExpress47[j+1] += pExpress47[nParam++];
	}
	ExpressParam47to25(m_pFitCoeffsExpress47, m_pFitCoeffsExpress);
	SAFE_DELETE_ARRAY(pExpress47Dim);
}

void MultiLinearVideoReconstructor::RunEstimateProjectMat(float*pModelPoint, float*pImagePoint, int nIter)
{
	for (int i = 0; i < 17; i++)
	{
		if (i != 8)
		{
			m_pWeightLanMark[i] = 0.3*nIter;
		}

	}
	for (int i = 67; i < 86; i++)
	{
		if (i != 72)
		{
			m_pWeightLanMark[i] = 0.3*nIter;
		}
	}
	bool isInit = m_nFrame == 1 ? false : true;

#if MLVR_USE_PERPECTIVE_PROJECT
	PerspectiveEstimeCamera m_PEC;
	m_PEC.EstimateProMat(pModelPoint, pImagePoint, MLM_LANMARK_INDEX, m_nWidth, m_nHeight, m_pProjectionMat,
		m_pWeightLanMark, m_pCameraParam, isInit);
#else
	OrthographicEstimateCamera EC;
	if(m_nFrame == 1)
	{
		float faceRate = m_nWidth *1.0f / m_nFaceWidth;
	    memset(m_pCameraParam, 0, 6 * sizeof(float));
	    m_pCameraParam[5] = 1.5f *faceRate / 3.f;
	}
	EC.EstimateProMat(pModelPoint, pImagePoint, MLM_LANMARK_INDEX, m_nWidth, m_nHeight, m_pProjectionMat,
		m_pWeightLanMark, m_pCameraParam, true);

#endif
}

void MultiLinearVideoReconstructor::Fitting(float *pModelPoint, float *pNormBasis, float *pImagePoint, int nPoints, int nDims, float*pProjectionMat, float *pOutCoeffs, float lamda, bool bFirst)
{
	using namespace Eigen;
	MatrixXf AShape;
	MatrixXf BShape;
	LLT<MatrixXf> lltShape;
	AShape = MatrixXf::Zero(2 * nPoints, nDims);
	float *pA = AShape.data();
	int nStep = 2 * nPoints;
	float *pV_hat_h = pNormBasis;
	for (int i = 0; i < nPoints; i++)
	{
		float *ptrA = pA + i * 2;
		for (int j = 0; j < nDims; j++)
		{
#if MLVR_USE_PERPECTIVE_PROJECT
			ProjectPoint(pV_hat_h, pProjectionMat, ptrA, m_nWidth, m_nHeight);
            ptrA[0] -= m_nWidth*0.5f;
            ptrA[1] -= m_nHeight*0.5f;
#else
			float x = pV_hat_h[0];
			float y = pV_hat_h[1];
			float z = pV_hat_h[2];
			ptrA[0] = pProjectionMat[0] * x + pProjectionMat[1] * y + pProjectionMat[2] * z;
			ptrA[1] = pProjectionMat[4] * x + pProjectionMat[5] * y + pProjectionMat[6] * z;
#endif
			ptrA += nStep;
			pV_hat_h += 3;
		}
	}

	MatrixXf Omega = MatrixXf::Identity(nStep, nStep);
	MatrixXf Alpha = MatrixXf::Identity(nDims, nDims);

	if (nDims != MLM_IDENTITY_DIMS)
	{
		// code by zhanghuaye 20180906
		// 默认参数
		std::map<std::string, std::pair<float, float> > weightAll;
		weightAll["contourUp"] = std::make_pair(0.5f, 0.5f);
		weightAll["contourDown"] = std::make_pair(0.5f, 0.5f);
		weightAll["eyeBrow"] = std::make_pair(3.0f, 3.0f);
		weightAll["eye"] = std::make_pair(5.0f, 5.0f);
		weightAll["mouth"] = std::make_pair(8.0f, 8.0f);

		bool largeRotateX = std::abs(m_pCameraParam[0]) > 15.0f;
		bool largeRotateY = std::abs(m_pCameraParam[1]) > 20.0f;


		// 大角度时，适当扩大轮廓点权重，分成侧面轮廓和底部轮廓，实测对五官基本没有影响
		if (largeRotateX || largeRotateY)
		{
			weightAll["eye"] = std::make_pair(5.0f, 5.0f);
			weightAll["contourUp"] = std::make_pair(3.5f, 3.5f);
			weightAll["contourDown"] = std::make_pair(3.5f, 3.5f);

			if (largeRotateX)
			{
				weightAll["contourDown"].second *= 3.0f;
				weightAll["eye"].second *= 1.5f;
			}
			if (largeRotateY)
			{
				weightAll["contourUp"].first *= 2.0f;
			}
		}

		//***********************
		//	set weigth
		//***********************
		for (int i = 0; i < 6; i++)
		{
			Omega(i * 2, i * 2) = weightAll["contourUp"].first;
			Omega(i * 2 + 1, i * 2 + 1) = weightAll["contourUp"].second;
		}
		for (int i = 11; i < 17; i++)
		{
			Omega(i * 2, i * 2) = weightAll["contourUp"].first;
			Omega(i * 2 + 1, i * 2 + 1) = weightAll["contourUp"].second;
		}
		for (int i = 6; i < 11; i++)
		{
			Omega(i * 2, i * 2) = weightAll["contourDown"].first;
			Omega(i * 2 + 1, i * 2 + 1) = weightAll["contourDown"].second;
		}
		// eyebrow
		for (int i = 17; i < 35; i++)
		{
			Omega(i * 2, i * 2) = weightAll["eyeBrow"].first;
			Omega(i * 2 + 1, i * 2 + 1) = weightAll["eyeBrow"].second;
		}
		// eye
		for (int i = 35; i < 51; i++)
		{
			Omega(i * 2, i * 2) = weightAll["eye"].first;
			Omega(i * 2 + 1, i * 2 + 1) = weightAll["eye"].second;
		}
		// mouth
		for (int i = 65; i < 86; i++)
		{
			Omega(i * 2, i * 2) = weightAll["mouth"].first;
			Omega(i * 2 + 1, i * 2 + 1) = weightAll["mouth"].second;
		}


	}
	MatrixXf AT_A_Lanmda = AShape.transpose()*Omega*AShape + lamda*lamda * Alpha;
	lltShape.compute(AT_A_Lanmda);

	BShape = MatrixXf::Zero(2 * nPoints, 1);
	float *pB = (float*)BShape.data();
	float *pVBar = pModelPoint;
	float *py = pImagePoint;
	for (int i = 0; i < nPoints; i++)
	{
#if MLVR_USE_PERPECTIVE_PROJECT
		ProjectPoint(pVBar, pProjectionMat, pB, m_nWidth, m_nHeight);
#else
		float x = pVBar[0];
		float y = pVBar[1];
		float z = pVBar[2];
		pB[0] = pProjectionMat[0] * x + pProjectionMat[1] * y + pProjectionMat[2] * z + pProjectionMat[3];
		pB[1] = pProjectionMat[4] * x + pProjectionMat[5] * y + pProjectionMat[6] * z + pProjectionMat[7];
#endif
		pB[0] -= py[0];
		pB[1] -= py[1];
		py += 2;
		pB += 2;
		pVBar += 3;
	}
	MatrixXf X = lltShape.solve(-AShape.transpose()*Omega*BShape);
	memcpy(pOutCoeffs, X.data(), nDims*sizeof(float));

}

void MultiLinearVideoReconstructor::RePosFitBlock(short3D *p3Dindex, float *pImagePoints, int nIter)
{
	short3D *pContourIndex = model->GetContouIndex();
	short3D *pContourLine = model->GetContourLine();
	float *pContourVertex = GetAndUpdateContour(m_pFitCoeffsExpress);
	short3D pProjContour[MLM_CONTOUR_LINE];
	//0:34 left
	//35:39 center
	//40:74 right
	//Left
	int nBlock = 0;
	for (int i = 0; i < 35; i++)
	{
		int nBlockClass = pContourLine[i];
		float MinX = 10000.f;
		int index3 = 0;
		for (int j = 0; j < nBlockClass; j++)
		{
			float *pModel3dxyz = pContourVertex + nBlock * 3;
#if MLVR_USE_PERPECTIVE_PROJECT
			float out[2];
			ProjectPoint(pModel3dxyz, m_pProjectionMat, out, m_nWidth, m_nHeight);
			float x = out[0];
#else
			float x = m_pProjectionMat[0] * pModel3dxyz[0] + m_pProjectionMat[1] * pModel3dxyz[1] + m_pProjectionMat[2] * pModel3dxyz[2];

#endif
			if (x < MinX)
			{
				MinX = x;
				index3 = nBlock;
			}
			nBlock++;
		}
		pProjContour[i] = index3;
	}

	//center
	for (int i = 35; i < 40; i++)
	{
		int nBlockClass = pContourLine[i];
		float MaxY = -10000.f;
		int index3 = 0;
		for (int j = 0; j < nBlockClass; j++)
		{
			float *pModel3dxyz = pContourVertex + nBlock * 3;

#if MLVR_USE_PERPECTIVE_PROJECT
			float out[2];
			ProjectPoint(pModel3dxyz, m_pProjectionMat, out, m_nWidth, m_nHeight);
			float y = m_nHeight - out[1];
#else
			float y = m_pProjectionMat[4] * pModel3dxyz[0] + m_pProjectionMat[5] * pModel3dxyz[1] + m_pProjectionMat[6] * pModel3dxyz[2];
#endif
			if (y > MaxY)
			{
				MaxY = y;
				index3 = nBlock;
			}
			nBlock++;
		}
		pProjContour[i] = index3;
	}

	//Right
	for (int i = 40; i < 75; i++)
	{
		int nBlockClass = pContourLine[i];
		float MaxX = -10000.f;
		int index3 = 0;
		for (int j = 0; j < nBlockClass; j++)
		{
			float *pModel3dxyz = pContourVertex + nBlock * 3;
#if MLVR_USE_PERPECTIVE_PROJECT
			float out[2];
			ProjectPoint(pModel3dxyz, m_pProjectionMat, out, m_nWidth, m_nHeight);
			float x = out[0];
#else
			float x = m_pProjectionMat[0] * pModel3dxyz[0] + m_pProjectionMat[1] * pModel3dxyz[1] + m_pProjectionMat[2] * pModel3dxyz[2];
#endif
			if (x > MaxX)
			{
				MaxX = x;
				index3 = nBlock;
			}
			nBlock++;
		}
		pProjContour[i] = index3;
	}

	//Find nearest
	for (int i = 0; i < 17; i++)
	{
		float MinDist = 100000.f;
		int indexTmp = 0;
		for (int j = 0; j < MLM_CONTOUR_LINE; j++)
		{
			float *pModel3dxyz = pContourVertex + pProjContour[j] * 3;
#if MLVR_USE_PERPECTIVE_PROJECT
			float out[2];
			ProjectPoint(pModel3dxyz, m_pProjectionMat, out, m_nWidth, m_nHeight);
			float x = out[0];
			float y = out[1];
#else
			float x = m_pProjectionMat[0] * pModel3dxyz[0] + m_pProjectionMat[1] * pModel3dxyz[1] + m_pProjectionMat[2] * pModel3dxyz[2] + m_pProjectionMat[3];
			float y = m_pProjectionMat[4] * pModel3dxyz[0] + m_pProjectionMat[5] * pModel3dxyz[1] + m_pProjectionMat[6] * pModel3dxyz[2] + m_pProjectionMat[7];
#endif
			float pointdst = sqrt((pImagePoints[i * 2] - x)*(pImagePoints[i * 2] - x) + (pImagePoints[i * 2 + 1] - y)*(pImagePoints[i * 2 + 1] - y));
			if (pointdst < MinDist)
			{
				MinDist = pointdst;
				indexTmp = pProjContour[j];
			}
		}
		p3Dindex[i] = pContourIndex[indexTmp];
	}
}

bool MultiLinearVideoReconstructor::AddNewFrameToSet()
{
	if (m_nFrame<MLVR_THROW_INIT_FRAME)
	{
		return false;
	}
	if (m_pCameraParam[0]>15.f || m_pCameraParam[0] < -25.f)
	{
		return false;
	}
	if (m_pCameraParam[1] > 25.f || m_pCameraParam[1] < -25.f)
	{
		return false;
	}
	if (m_RepresentativeFrame.size() > MLVR_MAX_IDENTITY_REPRENTATIVE_FRAME)
	{
		return false;
	}

	RepresentativeFrame RFrame;
	RFrame.initVec(m_pCameraParam, m_pFitCoeffsExpress);
	if (m_RepresentativeFrame.size() == 1)
	{
		memcpy(RFrame.pImagePoint2D, m_pImagePoint2D, sizeof(float)*MLM_LANMARK_INDEX * 2);
		memcpy(RFrame.pIndex, m_pIndex3D, sizeof(short3D)*MLM_LANMARK_INDEX);
		memcpy(RFrame.pProjectMat, m_pProjectionMat, 16 * sizeof(float));
		m_RepresentativeFrame.push_back(RFrame);
		return true;
	}

	float projectErr = CalFrameProjectErr(m_pMMt, m_RepresentativeFrame[0].Rotation_Express_vec, RFrame.Rotation_Express_vec);
	if (projectErr > 0.5f)
	{
		memcpy(RFrame.pImagePoint2D, m_pImagePoint2D, sizeof(float)*MLM_LANMARK_INDEX * 2);
		memcpy(RFrame.pIndex, m_pIndex3D, sizeof(short3D)*MLM_LANMARK_INDEX);
		memcpy(RFrame.pProjectMat, m_pProjectionMat, 16 * sizeof(float));
		m_RepresentativeFrame.push_back(RFrame);
		return true;
	}
	return false;
}

void MultiLinearVideoReconstructor::RunMultiFrameShape(float Lamda)
{
	FittingMultiIdentity(Lamda * 20);
// 	int nFrame = m_RepresentativeFrame.size();
// 	for (int i = 1; i < nFrame; i++)
// 	{
// 		memcpy(m_pFitCoeffsExpress, m_RepresentativeFrame[i].Rotation_Express_vec + 3, MLM_EXPRESS_DIMS*sizeof(float));
// 		memcpy(m_pProjectionMat, m_RepresentativeFrame[i].pProjectMat, 16 * sizeof(float));
// 		memcpy(m_pIndex3D, m_RepresentativeFrame[i].pIndex, 17 * sizeof(short3D));
// 		float *pModelPoint = GetLanmarkVertex(m_pFitCoeffsExpress);
// 
// 		RePosFitBlock(m_pIndex3D, m_RepresentativeFrame[i].pImagePoint2D, 2);
// 		RunEstimateProjectMat(pModelPoint, m_RepresentativeFrame[i].pImagePoint2D, 2);
// 		RunExpress(m_RepresentativeFrame[i].pImagePoint2D, Lamda);
// 
// 		m_RepresentativeFrame[i].initVec(m_pCameraParam, m_pFitCoeffsExpress);
// 		memcpy(m_RepresentativeFrame[i].pIndex, m_pIndex3D, sizeof(short3D)* 17);
// 		memcpy(m_RepresentativeFrame[i].pProjectMat, m_pProjectionMat, 16 * sizeof(float));
// 	}
// #if MLVR_USE_PERPECTIVE_PROJECT
// 	float *pModelPoint = GetLanmarkVertex(m_pFitCoeffsExpress);
// 	m_PEC.OptimizeForFocalLength(pModelPoint, m_pImagePoint2D, MLM_LANMARK_INDEX, m_pCameraParam);
// #else
// 
// #endif

	CalMeanRepresentativeFrame(m_RepresentativeFrame);
	CalEigenVectorMat(m_RepresentativeFrame, m_pMMt);
}

void MultiLinearVideoReconstructor::FittingMultiIdentity(float Lamda)
{
	int nFrame = m_RepresentativeFrame.size() - 1;
	MatrixXf AShape = MatrixXf::Zero(2 * MLM_LANMARK_INDEX*nFrame, MLM_IDENTITY_DIMS);
	float *pA = AShape.data();
	int nStep = 2 * MLM_LANMARK_INDEX*nFrame;
	for (int f = 0; f < nFrame; f++)
	{
		float *pProjectionMatN = m_RepresentativeFrame[f + 1].pProjectMat;
		float *pV_hat_h = model->GetAndUpdateIdentityDim(m_RepresentativeFrame[f + 1].pIndex, m_RepresentativeFrame[f + 1].Rotation_Express_vec + 3);// GetAndUpdateExpressDim();
		for (int i = 0; i < MLM_LANMARK_INDEX; i++)
		{
			float *ptrA = pA + i * 2;
			for (int j = 0; j < MLM_IDENTITY_DIMS; j++)
			{
#if MLVR_USE_PERPECTIVE_PROJECT
				ProjectPoint(pV_hat_h, pProjectionMatN, ptrA, m_nWidth, m_nHeight);
                ptrA[0] -= m_nWidth*0.5f;
                ptrA[1] -= m_nHeight*0.5f;
#else
				float x = pV_hat_h[0];
				float y = pV_hat_h[1];
				float z = pV_hat_h[2];
				ptrA[0] = pProjectionMatN[0] * x + pProjectionMatN[1] * y + pProjectionMatN[2] * z;
				ptrA[1] = pProjectionMatN[4] * x + pProjectionMatN[5] * y + pProjectionMatN[6] * z;
#endif
				ptrA += nStep;
				pV_hat_h += 3;
			}
		}
		pA += MLM_LANMARK_INDEX * 2;
	}

	MatrixXf AT_A_Lanmda = AShape.transpose()*AShape + nFrame*Lamda*Lamda * MatrixXf::Identity(MLM_IDENTITY_DIMS, MLM_IDENTITY_DIMS);
	LLT<MatrixXf> lltShape;
	lltShape.compute(AT_A_Lanmda);
	MatrixXf BShape = MatrixXf::Zero(nStep, 1);
	float *pB = (float*)BShape.data();
	for (int f = 0; f < nFrame; f++)
	{
		float *pProjectionMatN = m_RepresentativeFrame[f + 1].pProjectMat;
		float *pVBar = model->GetLanmarkVertex(m_RepresentativeFrame[f + 1].pIndex, m_RepresentativeFrame[f + 1].Rotation_Express_vec + 3, model->GetIdentityInitParam());// GetLanmarkVertex(m_RepresentativeFrame[f + 1].Rotation_Express_vec + 3);
		float *py = m_RepresentativeFrame[f + 1].pImagePoint2D;
		for (int i = 0; i < MLM_LANMARK_INDEX; i++)
		{
#if MLVR_USE_PERPECTIVE_PROJECT
			ProjectPoint(pVBar, pProjectionMatN, pB, m_nWidth, m_nHeight);
#else
			float x = pVBar[0];
			float y = pVBar[1];
			float z = pVBar[2];
			pB[0] = pProjectionMatN[0] * x + pProjectionMatN[1] * y + pProjectionMatN[2] * z + pProjectionMatN[3];
			pB[1] = pProjectionMatN[4] * x + pProjectionMatN[5] * y + pProjectionMatN[6] * z + pProjectionMatN[7];
#endif
			pB[0] -= py[0];
			pB[1] -= py[1];
			py += 2;
			pB += 2;
			pVBar += 3;
		}
	}
	MatrixXf X = lltShape.solve(-AShape.transpose()*BShape);
	memcpy(m_pFitCoeffsID, X.data(), MLM_IDENTITY_DIMS*sizeof(float));
	float *pIdentityParam = model->GetIdentityInitParam();
	for (int j = 0; j < MLM_IDENTITY_DIMS; j++)
	{
		m_pFitCoeffsID[j] += pIdentityParam[j];
	}
	UpdateExpressDim();
}


void MultiLinearVideoReconstructor::UpdateExpressDim()
{
	memset(m_pExpressDim, 0, sizeof(float)*MLM_FACE_3D_VERTEX * 3 * MLM_EXPRESS_DIMS);
	int nFace = MLM_EXPRESS_DIMS*MLM_IDENTITY_DIMS;
	short *ModelCore = model->GetCore();
	float *ModelCoreCore = model->GetCoreCore();

	for (int k = 0; k < MLM_FACE_3D_VERTEX; k++)
	{
		float *pExpress = m_pExpressDim + k * 3 * MLM_EXPRESS_DIMS;
		short *pCore = ModelCore + k * nFace * 3;
		float *pCoreCore = ModelCoreCore + k * 3;
		for (int i = 0; i < MLM_IDENTITY_DIMS; i++)
		{
			float *pExpressi = pExpress;
			float idParam = m_pFitCoeffsID[i];
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				(*pExpressi++) += (*pCore++)*idParam;
				(*pExpressi++) += (*pCore++)*idParam;
				(*pExpressi++) += (*pCore++)*idParam;
			}
		}

		{
			m_pExpressDim[k * 3 * MLM_EXPRESS_DIMS] += m_pFitCoeffsID[0] * pCoreCore[0] * MLM_FLOAT_TO_SHORT;
			m_pExpressDim[k * 3 * MLM_EXPRESS_DIMS + 1] += m_pFitCoeffsID[0] * pCoreCore[1] * MLM_FLOAT_TO_SHORT;
			m_pExpressDim[k * 3 * MLM_EXPRESS_DIMS + 2] += m_pFitCoeffsID[0] * pCoreCore[2] * MLM_FLOAT_TO_SHORT;
		}
	}
}

float* MultiLinearVideoReconstructor::GetLanmarkVertex(float *pExpressParam)
{
	if (m_bFixID)
	{
		float *pLanmark = m_pLanmarkVertex;
		for (int i = 0; i < MLM_LANMARK_INDEX; i++)
		{
			float *pExpress = m_pExpressDim + m_pIndex3D[i] * 3 * MLM_EXPRESS_DIMS;
			float x = 0.0f, y = 0.0f, z = 0.0f;
			float *p = pExpressParam;
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				x += (*pExpress++)*(*p);
				y += (*pExpress++)*(*p);
				z += (*pExpress++)*(*p++);
			}
			*pLanmark++ = x*MLM_SHORT_TO_FLOAT;
			*pLanmark++ = y*MLM_SHORT_TO_FLOAT;
			*pLanmark++ = z*MLM_SHORT_TO_FLOAT;
		}
		return m_pLanmarkVertex;
	}
	else
	{
		return model->GetLanmarkVertex(m_pIndex3D, pExpressParam,m_pFitCoeffsID);
	}
}

float*MultiLinearVideoReconstructor::GetAndUpdateContour(float *pExpressParam)
{
	if (m_bFixID)
	{
		float *pContour = m_pContourVertex;
		short3D *pIndex = model->GetContouIndex();
		for (int i = 0; i < MLM_CONTOUR_INDEX; i++)
		{
			float *pExpress = m_pExpressDim + pIndex[i] * 3 * MLM_EXPRESS_DIMS;
			float x = 0.0f, y = 0.0f, z = 0.0f;
			float *p = pExpressParam;
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				x += (*pExpress++)*(*p);
				y += (*pExpress++)*(*p);
				z += (*pExpress++)*(*p++);
			}
			*pContour++ = x*MLM_SHORT_TO_FLOAT;
			*pContour++ = y*MLM_SHORT_TO_FLOAT;
			*pContour++ = z*MLM_SHORT_TO_FLOAT;
		}
		return m_pContourVertex;
	}
	else
	{
		return model->GetAndUpdateContour(pExpressParam, m_pFitCoeffsID);
	}
}

float *MultiLinearVideoReconstructor::GetAndUpdateExpressDim()
{
	if (m_bFixID)
	{
		for (int i = 0; i < MLM_LANMARK_INDEX;i++)
		{
			memcpy(m_pLanmarkExpressBasis + i*MLM_EXPRESS_DIMS * 3, m_pExpressDim + m_pIndex3D[i] * 3 * MLM_EXPRESS_DIMS, sizeof(float)*MLM_EXPRESS_DIMS * 3);
		}
		int nSize = MLM_LANMARK_INDEX*MLM_EXPRESS_DIMS * 3;
		for (int i = 0; i < nSize;i++)
		{
			m_pLanmarkExpressBasis[i] *= MLM_SHORT_TO_FLOAT;
		}
		return m_pLanmarkExpressBasis;
	}
	else
	{
		return model->GetAndUpdateExpressDim(m_pIndex3D,m_pFitCoeffsID);
	}
}
void MultiLinearVideoReconstructor::GenerateModel()
{

#if USE_PERPECTIVE_PROJECT

#else
	float p25[25];
    ExpressParam25to47(m_pFitCoeffsExpress, m_pFitCoeffsExpress47);
    
    float Sum = 0.f;
    for (int i=0; i<47; i++) {
        Sum+= m_pFitCoeffsExpress47[i];
    }
    float s = fabs(Sum);

    m_pProjectionMat[0] *= s;
    m_pProjectionMat[1] *= s;
    m_pProjectionMat[2] *= s;
    m_pProjectionMat[4] *= s;
    m_pProjectionMat[5] *= s;
    m_pProjectionMat[6] *= s;

	m_pFitCoeffsExpress47[0] = 1.0f;
    for (int i=1; i<47; i++) {
		m_pFitCoeffsExpress47[0] -= m_pFitCoeffsExpress47[i];
    }
    ExpressParam47to25(m_pFitCoeffsExpress47, p25);

#endif
    

	if (m_bFixID)
	{
		float *pModel = m_pAdjustModel3D;
		for (int i = 0; i < MLM_FACE_3D_VERTEX; i++)
		{
			float *pExpress = m_pExpressDim + i * 3 * MLM_EXPRESS_DIMS;
			float x = 0.0f, y = 0.0f, z = 0.0f;
			float *p = p25;
			for (int j = 0; j < MLM_EXPRESS_DIMS; j++)
			{
				x += (*pExpress++)*(*p);
				y += (*pExpress++)*(*p);
				z += (*pExpress++)*(*p++);
			}
			*pModel++ = x*MLM_SHORT_TO_FLOAT;
			*pModel++ = y*MLM_SHORT_TO_FLOAT;
			*pModel++ = z*MLM_SHORT_TO_FLOAT;
		}
		short3D* UVMap = model->GetUVMap();
		for (int i = MLM_FACE_3D_VERTEX; i < MLM_FACE_3D_UV_VERTEX; i++)
		{
			int index3 = UVMap[i - MLM_FACE_3D_VERTEX] * 3;
			int i3 = i * 3;
			m_pAdjustModel3D[i3] = m_pAdjustModel3D[index3];
			m_pAdjustModel3D[i3 + 1] = m_pAdjustModel3D[index3 + 1];
			m_pAdjustModel3D[i3 + 2] = m_pAdjustModel3D[index3 + 2];
		}
	}
	else
	{
		memcpy(m_pAdjustModel3D, model->Get3DFace(m_pFitCoeffsID, p25), sizeof(float) * 3 * MLM_FACE_3D_UV_VERTEX);
	}
	

}


void MultiLinearVideoReconstructor::ExpressParam25to47(float *Param25, float *Param47)
{
	float *pMat25to47 = model->GetExpress25To47Mat();
	for (int i = 0; i < 47; i++)
	{
		float sum = 0.f;
		for (int j = 0; j < 25; j++)
		{
			sum += pMat25to47[47 * j + i] * Param25[j];
		}
		Param47[i] = sum;
	}
}

void MultiLinearVideoReconstructor::ExpressParam47to25(float *Param47, float *Param25)
{
	float *pMat25to47 = model->GetExpress25To47Mat();
	for (int i = 0; i < 25; i++)
	{
		float sum = 0.f;
		for (int j = 0; j < 47; j++)
		{
			sum += (*pMat25to47++)*Param47[j];
		}
		Param25[i] = sum;
	}
}

void MultiLinearVideoReconstructor::ExpressDim25to47(float *pExpressDim25, float *pExpressDim47)
{
	float *pMat25to47 = model->GetExpress25To47Mat();
	for (int i = 0; i < MLM_LANMARK_INDEX; i++)
	{
		float x0 = 0.f, y0 = 0.f, z0 = 0.f;
		float *p25 = pExpressDim25 + i * 3 * MLM_EXPRESS_DIMS;
		for (int k = 0; k < 25; k++)
		{
			float p = pMat25to47[47 * k];
			x0 += p*(*p25++);
			y0 += p*(*p25++);
			z0 += p*(*p25++);
		}
		for (int j = 1; j < 47; j++)
		{
			int index = j;
			float x = 0.f, y = 0.f, z = 0.f;
			float *p25 = pExpressDim25 + i * 3 * MLM_EXPRESS_DIMS;
			for (int k = 0; k < 25; k++)
			{
				float p = pMat25to47[47 * k + index];
				x += p*(*p25++);
				y += p*(*p25++);
				z += p*(*p25++);
			}
			*pExpressDim47++ = x - x0;
			*pExpressDim47++ = y - y0;
			*pExpressDim47++ = z - z0;
		}
	}
}

void MultiLinearVideoReconstructor::OutObjIdentityNeutral(float *pIdentityParam)
{
	float pExpressParam47[47], pExpressParam25[25];
	memset(pExpressParam47, 0, 47 * sizeof(float));
	pExpressParam47[0] = 1.0f;
	ExpressParam47to25(pExpressParam47, pExpressParam25);
	float *pModel3D = model->Get3DFace(pIdentityParam, pExpressParam25);
	freopen("Neutral.obj", "w", stdout);
	//杈撳嚭椤剁偣銆佺汗鐞嗗潗鏍囥€佷笁瑙掑舰椤剁偣绱㈠紩
	printf("mtllib Neutral.mtl\n");
	for (int i = 0; i < MLM_FACE_3D_UV_VERTEX; i++)
	{
		printf("v %f %f %f\n", pModel3D[i * 3], pModel3D[i * 3 + 1], pModel3D[i * 3 + 2]);
	}
	float *pTextPos = model->GetTextureCoordinates();
	for (int i = 0; i < MLM_FACE_3D_UV_VERTEX; i++)
	{
		printf("vt %f %f\n", pTextPos[2 * i], pTextPos[i * 2 + 1]);
	}
	printf("usemtl FaceTexture\n");

	short3D *pT = model->GetTriangleIndex();
	int nTriangleNum = MLM_TRIANGLE_NUM;
	for (int i = 0; i < nTriangleNum; i++)
	{
		int i3 = i * 3;
		printf("f %d/%d %d/%d %d/%d\n", pT[i3] + 1, pT[i3] + 1, pT[i3 + 1] + 1, pT[i3 + 1] + 1, pT[i3 + 2] + 1, pT[i3 + 2] + 1);
	}
	fclose(stdout);
	freopen("Neutral.mtl", "w", stdout);
	printf("newmtl FaceTexture\n");
	printf("map_Kd %s\n", "Normal.png");
	fclose(stdout);
}

void MultiLinearVideoReconstructor::OutBlendShape(float *pIdentityParam)
{
	float pExpressParam47[47], pExpressParam25[25];
	memset(pExpressParam47, 0, 47 * sizeof(float));
	char pFileObj[] = "MT3DMaterial/BlenderShape/Express00.obj";
	char pFileMtl[] = "MT3DMaterial/BlenderShape/Express00.mtl";
	char pMtl[] = "Express00.mtl";
	for (int i = 1; i < 47; i++)
	{
		pExpressParam47[i] = 1.0f;
		pExpressParam47[i - 1] = 0.0f;
		ExpressParam47to25(pExpressParam47, pExpressParam25);
		float *pModel3D = model->Get3DFace(pIdentityParam, pExpressParam25);
		char a1 = (i % 10) + '0';
		char a2 = (i / 10) + '0';
		pFileObj[34] = a1;
		pFileObj[33] = a2;
		pFileMtl[34] = a1;
		pFileMtl[33] = a2;
		pMtl[8] = a1;
		pMtl[7] = a2;
		freopen(pFileObj, "w", stdout);
		//杈撳嚭椤剁偣銆佺汗鐞嗗潗鏍囥€佷笁瑙掑舰椤剁偣绱㈠紩
		printf("mtllib %s\n", pMtl);
		for (int i = 0; i < MLM_FACE_3D_UV_VERTEX; i++)
		{
			printf("v %f %f %f\n", pModel3D[i * 3], pModel3D[i * 3 + 1], pModel3D[i * 3 + 2]);
		}
		float *pTextPos = model->GetTextureCoordinates();
		for (int i = 0; i < MLM_FACE_3D_UV_VERTEX; i++)
		{
			printf("vt %f %f\n", pTextPos[2 * i], pTextPos[i * 2 + 1]);
		}
		printf("usemtl FaceTexture\n");

		short3D *pT = model->GetTriangleIndex();
		int nTriangleNum = MLM_TRIANGLE_NUM;
		for (int i = 0; i < nTriangleNum; i++)
		{
			int i3 = i * 3;
			printf("f %d/%d %d/%d %d/%d\n", pT[i3] + 1, pT[i3] + 1, pT[i3 + 1] + 1, pT[i3 + 1] + 1, pT[i3 + 2] + 1, pT[i3 + 2] + 1);
		}
		fclose(stdout);
		freopen(pFileMtl, "w", stdout);
		printf("newmtl FaceTexture\n");
		printf("map_Kd %s\n", "Normal.png");
		fclose(stdout);
	}
}
float * MultiLinearVideoReconstructor::GetNeuFace3D()
{
	if(m_pNeuFace3D==NULL)
	{
		m_pNeuFace3D = new float[MLM_FACE_3D_UV_VERTEX * 3];
		memcpy(m_pNeuFace3D, model->Get3DFace(m_pFitCoeffsID, model->GetExpressInitParam()), MLM_FACE_3D_UV_VERTEX * 3 * sizeof(float));
	}
	if(m_bFitIdentity)
	{
    	memcpy(m_pNeuFace3D, model->Get3DFace(m_pFitCoeffsID, model->GetExpressInitParam()), MLM_FACE_3D_UV_VERTEX * 3 * sizeof(float));
	}
	return m_pNeuFace3D;
}

} 
