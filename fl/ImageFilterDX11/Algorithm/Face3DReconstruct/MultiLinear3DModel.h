/*****************************************************************
* MultiLinear3DModel ,3DMM类:相关模型的读写操作
* Copyright (c) 2017年 MEITU. All rights reserved.
*
* @version: 1.0
*
* @author:  lym
*
* @date: 2017-09-08
*
* @note:
*
* @usage：
*
******************************************************************/
#ifndef MULTI_LINEAR_MODEL_lymlym2312434
#define MULTI_LINEAR_MODEL_lymlym2312434


#define  MLM_FACE_3D_VERTEX 2820
#define  MLM_FACE_3D_UV_VERTEX 2835
#define  MLM_IDENTITY_DIMS 35
#define  MLM_IDENTITY_PRIOR_DIMS 150
#define  MLM_EXPRESS_DIMS 25
#define  MLM_EXPRESS_PRIOR_DIMS 47
#define  MLM_TRIANGLE_NUM 5569
#define  MLM_TRIANGLE_WITH_LIPS 5602
#define  MLM_FACE_TRIANGLE_NUM 4266
#define  MLM_FACE_TRIANGLE_WITH_LIPS 4299

#define  MLM_LANMARK_INDEX 86
#define  MLM_CONTOUR_INDEX 731
#define  MLM_CONTOUR_LINE 75
#define short3D unsigned short

#define MLM_FLOAT_TO_SHORT 5000
#define MLM_SHORT_TO_FLOAT 0.0002f

#include "Common3D.hpp"


namespace mt3dface {
    
class  MultiLinear3DModel
{
public:
	MultiLinear3DModel();
	~MultiLinear3DModel();

//获取数据
public:
	//根据脸型系数与表情系数生成3D模型
	float *Get3DFace(float *pIdentityParam, float *pExpressParam);

	//获取初始3D点的索引
	short3D* Get3DIndex();

	//获取初始2D点的索引
	short3D* Get2DIndex();

	//获取头部三角形
	short3D* GetTriangleIndex();

	//获取头部UV
	float* GetTextureCoordinates();

	//获取脸部三角形
	short3D* GetFaceTriangleIndex();

	//获取脸部UV
	float* GetFaceTextureCoordinates();

	//获取SFM UV
	float* GetSFMTextureCoordinates();

	//获取人脸法线
	float* GetFaceNormal();

//输出
public:
	//输出obj
	void ObjOut(float *pModel3D, float *pTextPos, int nTextWidth, int nTextHeight, char *pImageName, bool bNormal = false);

public:
	bool LoadModel(char *pModelName, char *pContourName, char *pLanmarkName, char *pTempfile, char *pMatFile, bool bUseBlendShape = false);

	//支持直接读取数据流
	bool LoadModelFromData(unsigned char *pModelData, unsigned char *pContourData, unsigned char *pLanmarkData, unsigned char *pTempData, unsigned char *pMatDta);
public:
	float *GetCoreCore();
	short *GetCore();
	float *GetAndUpdateIdentityDim(short3D *pIndex, float *pExpressParam);
	float *GetAndUpdateExpressDim(short3D *pIndex, float *pIdentityParam);
	float *GetLanmarkVertex(short3D *pIndex, float *pExpressParam, float *pIdentityParam);
	short3D *GetContouIndex();
	short3D *GetContourLine();
	float *GetAndUpdateContour(float *pIdentityParam, float *pExpressParam);
	short3D* GetUVMap();
	float *GetExpress25To47Mat();
	float *GetExpressInitParam();
	float *GetIdentityInitParam();
private:
	void ClearData();
	void InitData();
	void LoadContourLine(char *pFileName);
	void LoadLanmarkIndex(char *pFileName);
	void Load3DObj(char*pFileName);
	void LoadExpressMat(char*pFileName);
	short *m_pCore;
	float *m_pCoreCore;

	float *m_p3DFaceVertex;

	float *m_pContourVertex;
	short3D *m_pContourIndex;
	short3D m_pContourLine[MLM_CONTOUR_LINE];
	float *m_pIdentityVertex;   //选取的脸型3d点
	float *m_pIdentityBasis;    //选取的3d点对应的shape基
	float *m_pExpressVertex;    //选取的表情3d点
	float *m_pExpressBasis;      //选取的3d点对应的Express基
	float *m_pLanmarkVertex;    //选取的3d点

	short3D m_p3Dindex[MLM_LANMARK_INDEX];
	short3D m_p2Dindex[MLM_LANMARK_INDEX];

	float *m_pTextureCoordinates;
	short3D *m_pTriangleIndex;
	float *m_pFaceTextureCoordinates;
	short3D *m_pFaceTriangleIndex;
	float *m_pFaceNormal;

	float *m_pSFMTextureCoordinates;
	short3D m_pUVMap[MLM_FACE_3D_UV_VERTEX - MLM_FACE_3D_VERTEX];

	float m_pExpress25To47Mat[MLM_EXPRESS_DIMS * MLM_EXPRESS_PRIOR_DIMS];
	float m_pExpressInitParam[MLM_EXPRESS_DIMS];
	float m_pIdentityInitParam[MLM_IDENTITY_DIMS];


	float *m_pBlendShape;
	bool m_bUseBlendShape;
};

} //namespace mt3dface
#endif  //MULTI_LINEAR_MODEL_lymlym2312434
