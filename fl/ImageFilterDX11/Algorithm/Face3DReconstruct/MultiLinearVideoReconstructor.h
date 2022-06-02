/*****************************************************************
* MultiLinearVideoReconstructor ,实时3D重建,需要外部做人脸检测
Copyright (c) 2017骞?MEITU. All rights reserved.
*
* @version: 1.0
*
* @author:  lym
*
* @date: 2017-10-01
*
* @note:
*
* @usage*
******************************************************************/

#ifndef MULTI_LINEAR_VIDEO_RECONSTRUCTOR_lymlymdshakj
#define  MULTI_LINEAR_VIDEO_RECONSTRUCTOR_lymlymdshakj

#include "Common3D.hpp"
#include "MultiLinear3DModel.h"
#include "OrthographicEstimateCamera.h"
#include "PerspectiveEstimeCamera.h"
#include <vector>
#include <string>
#include "BaseDefine/Vectors.h"
namespace mt3dface {
    
using std::vector;

#define MLVR_Smooth3DFace  5
#define MLVR_REPRENTATIVE_FRAME 28
#define MLVR_THROW_INIT_FRAME  5
#define MLVR_MAX_IDENTITY_REPRENTATIVE_FRAME 10

#define MLVR_USE_PERPECTIVE_PROJECT 0

struct RepresentativeFrame
{
	float pImagePoint2D[MLM_LANMARK_INDEX * 2];
	float pProjectMat[16];
	float Rotation_Express_vec[MLVR_REPRENTATIVE_FRAME];
	short3D pIndex[MLM_LANMARK_INDEX];
	void initVec(float *Rxyz, float *pExpressParam)
	{
		Rotation_Express_vec[0] = Rxyz[0] * 3.1416f / 180.f;
		Rotation_Express_vec[1] = Rxyz[1] * 3.1416f / 180.f;
		Rotation_Express_vec[2] = Rxyz[2] * 3.1416f / 180.f;
		memcpy(Rotation_Express_vec + 3, pExpressParam, (MLVR_REPRENTATIVE_FRAME - 3) * sizeof(float));
	}
};

class   MultiLinearVideoReconstructor
{
public:
	MultiLinearVideoReconstructor();
	~MultiLinearVideoReconstructor();


	//加载模型，LoadModel和InitModel只能调用一个
public:

	///-------------------------------------------------------------------------------------------------
	/// @fn	bool LoadModel(const char *configureFolder, const char *userFolder = NULL);
	///
	/// @brief	Loads 3D model.
	///
	/// @author	Alop
	/// @date	18/6/19
	///
	/// @param	configureFolder ： 3Dmodel文件夹
	/// @return	True if it succeeds, false if it fails.
	///-------------------------------------------------------------------------------------------------
	bool LoadModel(const char *configureFloder);

	///用外部已加载好的模型初始化当前模型
	bool InitModel(MultiLinear3DModel *pExternModel);

//fitting
public:

	///-------------------------------------------------------------------------------------------------
	/// @fn	bool Run(float *pImagePoint106, int nWidth, int nHeight, int nFaceWidth, int nFaceHeight, int nMaxIter = 2, int nSmoothModel = 3, bool isUse47Express = false, bool isFirstFrame = false);
	///
	/// @brief	fitting
	///
	/// @author	Alop
	/// @date	18/6/19
	///
	/// @param [in]	    pImagePoint106	the image FA point 106.
	/// @param 		   	nWidth		  	The width.
	/// @param 		   	nHeight		  	The height.
	/// @param 		   	nFaceWidth	  	Width of the face.
	/// @param 		   	nFaceHeight   	Height of the face.
	/// @param 		   	nMaxIter	  	(Optional) The maximum iterator.
	/// @param 		   	nSmoothModel  	(Optional) The smooth model frame.
	/// @param 		   	isUse47Express	(Optional) True if this object is use 47 express.
	/// @param 		   	isFirstFrame    每个人脸第一次调用时必须为true
	/// @return	True if it succeeds, false if it fails.
	///-------------------------------------------------------------------------------------------------

	bool Run(float *pImagePoint106, int nWidth, int nHeight, int nFaceWidth, int nFaceHeight, bool isFirstFrame = false,int nMaxIter = 2,
		int nSmoothModel = 3, bool isUse47Express = false);

	///将正交投影矩阵转换成透视投影矩阵（每一帧调用）
	void OrthoToPerspectMVP(float FovAngle = 15.f,int SmoothFrame = 1);

	void ClearData();

	void InitData();
/// 相关输出
public:
	//输出脸型系数
	void saveIdentityCoeffs(std::string filePath);
	//输出当前脸型对应的中性表情obj
	void OutObjIdentityNeutral(float *pIdentityParam);
	//输出当前脸型对应的blendShape
	void OutBlendShape(float *pIdentityParam);
	
	float *GetNeuFace3D();

public:
	//3DModel类
	MultiLinear3DModel *model;

	///最终输出后的3D模型
	float *m_pAdjustModel3D;

	//传给GL的姿态预估矩阵,按行存储
	float m_fModelView[16];

	float m_fModelViewPerspect[16+6];

	//姿态预估矩阵，fitting用，最终将模型投影到图像空间	
	float m_pProjectionMat[16]; 

	///姿态预估的6个参数[Rx,Ry,Rz,Tx,Ty,S],其中R为角度（正交投影）
	float m_pCameraParam[6];

	///姿态预估的6个参数[Rx,Ry,Rz,Tx,Ty,Tz],其中R为角度（透视投影）
	float m_pCameraParamPerspect[MLVR_Smooth3DFace+1][6];
	//fitting选取的3D点索引
	short3D m_pIndex3D[MLM_LANMARK_INDEX];  

	//fitting选取的2D点坐标
	float m_pImagePoint2D[MLM_LANMARK_INDEX * 2];

	//fitting得到的脸型系数
	float m_pFitCoeffsID[MLM_IDENTITY_DIMS];  

	//fitting得到的表情系数
	float m_pFitCoeffsExpress[MLM_EXPRESS_DIMS];

	//fitting得到的原始空间的表情系数
	float m_pFitCoeffsExpress47[47];

	//是否fit了脸型
	bool m_bFitIdentity;

private:
	void RunShape_Express();
	void RunExpress(float*pImagePoint2D, float Lamda);
	void RunExpress47(float*pImagePoint2D, float Lamda);
	void RunEstimateProjectMat(float*pModelPoint, float*pImagePoint, int nIter);
	void Fitting(float *pModelPoint, float *pNormBasis, float *pImagePoint, int nPoints, int nDims, float*pProjectionMat, float *pOutCoeffs, float lamda = 3.0f, bool bFirst = true);
	void RePosFitBlock(short3D *p3Dindex, float *pImagePoints, int nIter);

	void GenerateModel();

	void CalGL_MVP();

	void SmoothFace(int nSmoothFrame);

	void UpdateExpressDim();
	float *GetLanmarkVertex(float *pExpressParam);
	float *GetAndUpdateContour(float *pExpressParam);
	float *GetAndUpdateExpressDim();

	void ExpressParam47to25(float *Param47, float *Param25);
	void ExpressParam25to47(float *Param25, float *Param47);
	void ExpressDim25to47(float *pExpressDim25, float *pExpressDim47);

	bool AddNewFrameToSet();
	void RunMultiFrameShape(float Lamda);
	void FittingMultiIdentity(float Lamda);

	float m_pWeightLanMark[MLM_LANMARK_INDEX]; 

	float *m_pAdjustExpress3DN[MLVR_Smooth3DFace];
	float m_pFitCoeffsExpress47N[MLVR_Smooth3DFace][47];

	int m_nWidth, m_nHeight;
	int m_nFaceWidth, m_nFaceHeight;

	int m_nFrame;

	float *m_pExpressDim;
	float *m_pLanmarkVertex;
	float *m_pContourVertex;
	float *m_pLanmarkExpressBasis;     
	bool m_bFixID;             

	bool m_bUse47Express;
	int m_nMaxIter;

	vector<RepresentativeFrame> m_RepresentativeFrame;
	float *m_pMMt;
    
    bool m_bHasLoadModel;

	float *m_pNeuFace3D;
	int m_nGetPersepectCount;

	//for expressFit trick
	Vector2 m_LeftEyePoint[3];
	Vector2 m_RightEyePoint[3];
	float m_maxEyeAngle;
	float m_minEyeAngle;
	float m_UserMaxEyeAngle;
	float m_UserMinEyeAngle;

	PerspectiveEstimeCamera m_PEC;

};
} // namespace mt3dface
#endif //MULTI_LINEAR_VIDEO_RECONSTRUCTOR_lymlymdshakj
