#ifndef FaceDetectorInterface_h
#define FaceDetectorInterface_h

#include "FaceDetectorDef.h"
#define CC_MAX_FACE_COUNT 10
typedef struct ccFDRect_t
{
	float left;
	float top;
	float right;
	float bottom;
}ccFDRect,FaceRect_List[CC_MAX_FACE_COUNT];

typedef struct ccFDPoint_t
{
	float x;
	float y;
}ccFDPoint;

//人脸表情属性
typedef struct ccFDFaceExpression_t
{
	bool blinkLeft = false;		  //眨眼 -- left
	bool blinkRight = false;	  //眨眼 -- right
	bool openMouse = false;   //张嘴
	bool shakeHead = false;   //摇头
	bool nodHead = false;     //点头
	bool raiseEyeBrow = false;//挑眉
	bool pout = false;        //嘟嘴
}ccFDFaceExpression;

typedef struct ccFDShape68_t
{
	ccFDRect faceRect;      //人脸包围框
	ccFDPoint arrShape[68]; //人脸68点，从130人脸点导出以兼容旧接口
	ccFDPoint arrShapeDense[NUMKEYPOINTS];//人脸130点位置
	
	//ccFDPoint arrShapeProj[68];
	//人脸姿态和位置
	float yaw;
	float pitch;
	float roll;
	float x, y, z;
	float arrWMatrix[16];
	// 人脸姿态估计框位置
	int gesture_box_left, gesture_box_right, gesture_box_top, gesture_box_bottom;
	// 人脸表情
	ccFDFaceExpression faceExpression;
	ccFDPoint extraEye[38];							  //稠密眼瞳点
	bool own_extraEye_left, own_extraEye_right;      //当前是否存在稠密眼瞳点
	bool cover_mouth;								 // 是否覆盖嘴部
}ccFDShape68;


typedef struct ccFDFaceRes_t
{
	ccFDShape68 arrFace[CC_MAX_FACE_COUNT];//各个人脸的具体信息
	int numFace;//检测出的人脸数量
	float matPerspective[16];
}ccFDFaceRes;

typedef void * cc_handle_t;
FACEDETECTOR_EXPORTS_API cc_handle_t ccFDInit(const char *szLandMark);//模型初始化接口
FACEDETECTOR_EXPORTS_API void ccFDUnInit(cc_handle_t handle);
FACEDETECTOR_EXPORTS_API void ccFDReset(cc_handle_t handle);                    //重置，清除跟踪人脸信息。
FACEDETECTOR_EXPORTS_API void ccFDSetMaxDetectedFace(cc_handle_t handle, int num);//设置最大可检测人脸数量
FACEDETECTOR_EXPORTS_API int  ccFDGetMaxDetectedFace(cc_handle_t handle);
FACEDETECTOR_EXPORTS_API void ccFDSetMinFaceDetected(cc_handle_t handle, float fMinFace);//设置最小可检测人脸数量
FACEDETECTOR_EXPORTS_API float ccFDGetMinFaceDetected(cc_handle_t handle);
// stage two mouse
FACEDETECTOR_EXPORTS_API bool ccActivateStageTwo(cc_handle_t handle, const char *szLandMark);//开启二阶段模型
FACEDETECTOR_EXPORTS_API void ccDeactivateStageTwo(cc_handle_t handle);                    //关闭二阶段
// stage two eye
FACEDETECTOR_EXPORTS_API bool ccActivateStageTwoEye(cc_handle_t handle, const char *szLandMark);//开启眼部二阶段模型
FACEDETECTOR_EXPORTS_API void ccDeactivateStageTwoEye(cc_handle_t handle);                    //关闭二阶段
//
FACEDETECTOR_EXPORTS_API void ccFDSetLandmarkFilter(cc_handle_t handle, float fValue);//设置滤波系数。0~1.0，越大稳定性越高滞后越严重
FACEDETECTOR_EXPORTS_API float ccFDGetLandmarkFilter(cc_handle_t handle);
FACEDETECTOR_EXPORTS_API void ccFDSetImgFilter(cc_handle_t handle, bool bValue);//视频滤波开关。默认关。与点滤波合用后关键点会产生轻微滞后但更稳，需要进一步减少点抖动并对滞后不敏感的应用可以打开此选项。
FACEDETECTOR_EXPORTS_API void ccFDActivateEyeExpression(cc_handle_t handle);    //打开表情检测
FACEDETECTOR_EXPORTS_API void ccFDDeactivateEyeExpression(cc_handle_t handle);  //关闭表情检测
#if defined(_MSC_VER)
FACEDETECTOR_EXPORTS_API void ccFDDetect(cc_handle_t handle, unsigned char *pBuffer, int width, int height, int rowBytes, int ctType);//视频检测
#else
FACEDETECTOR_EXPORTS_API void ccFDDetect(cc_handle_t handle, unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, bool bPoseEst);
#endif
FACEDETECTOR_EXPORTS_API int ccPicFD(cc_handle_t handle, unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, FaceRect_List faceRects);//图片人脸检测接口
FACEDETECTOR_EXPORTS_API int ccPicFA(cc_handle_t handle, unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, float* landmark);//人脸图片人脸关键点检测
FACEDETECTOR_EXPORTS_API void ccFDGetFaceRes(cc_handle_t handle, ccFDFaceRes *res);//获取视频检测结果
FACEDETECTOR_EXPORTS_API void ccFDGetFaceResFlipHorizonal(cc_handle_t handle, ccFDFaceRes *res, int width, int height);//获取镜像人脸点结果

FACEDETECTOR_EXPORTS_API int ccFDNumFacedetected(cc_handle_t handle);//获取人脸数量
    
FACEDETECTOR_EXPORTS_API bool ccFDGetFaceDetected(cc_handle_t handle, float *arrFaceRect, int index);
FACEDETECTOR_EXPORTS_API bool ccFDGetFaceDetectedLast(cc_handle_t handle, float *arrFaceRect, int index);

FACEDETECTOR_EXPORTS_API int ccFDGetFaceShape2D(cc_handle_t handle, float *arrShape, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetHeaderRotate(cc_handle_t handle, float *arrRotate, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetHeaderTrans(cc_handle_t handle, float *arrTrans, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetHeaderMatrix(cc_handle_t handle, float *arrMatrix, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetFilterHeaderRotate(cc_handle_t handle, float *arrRotate, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetFilterHeaderTrans(cc_handle_t handle, float *arrTrans, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetFilterHeaderMatrix(cc_handle_t handle, float *arrMatrix, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetPerspectiveMatrix(cc_handle_t handle, float *arrMatrix);
#endif
