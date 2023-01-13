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

//������������
typedef struct ccFDFaceExpression_t
{
	bool blinkLeft = false;		  //գ�� -- left
	bool blinkRight = false;	  //գ�� -- right
	bool openMouse = false;   //����
	bool shakeHead = false;   //ҡͷ
	bool nodHead = false;     //��ͷ
	bool raiseEyeBrow = false;//��ü
	bool pout = false;        //���
}ccFDFaceExpression;

typedef struct ccFDShape68_t
{
	ccFDRect faceRect;      //������Χ��
	ccFDPoint arrShape[68]; //����68�㣬��130�����㵼���Լ��ݾɽӿ�
	ccFDPoint arrShapeDense[NUMKEYPOINTS];//����130��λ��
	
	//ccFDPoint arrShapeProj[68];
	//������̬��λ��
	float yaw;
	float pitch;
	float roll;
	float x, y, z;
	float arrWMatrix[16];
	// ������̬���ƿ�λ��
	int gesture_box_left, gesture_box_right, gesture_box_top, gesture_box_bottom;
	// ��������
	ccFDFaceExpression faceExpression;
	ccFDPoint extraEye[38];							  //������ͫ��
	bool own_extraEye_left, own_extraEye_right;      //��ǰ�Ƿ���ڳ�����ͫ��
	bool cover_mouth;								 // �Ƿ񸲸��첿
}ccFDShape68;


typedef struct ccFDFaceRes_t
{
	ccFDShape68 arrFace[CC_MAX_FACE_COUNT];//���������ľ�����Ϣ
	int numFace;//��������������
	float matPerspective[16];
}ccFDFaceRes;

typedef void * cc_handle_t;
FACEDETECTOR_EXPORTS_API cc_handle_t ccFDInit(const char *szLandMark);//ģ�ͳ�ʼ���ӿ�
FACEDETECTOR_EXPORTS_API void ccFDUnInit(cc_handle_t handle);
FACEDETECTOR_EXPORTS_API void ccFDReset(cc_handle_t handle);                    //���ã��������������Ϣ��
FACEDETECTOR_EXPORTS_API void ccFDSetMaxDetectedFace(cc_handle_t handle, int num);//�������ɼ����������
FACEDETECTOR_EXPORTS_API int  ccFDGetMaxDetectedFace(cc_handle_t handle);
FACEDETECTOR_EXPORTS_API void ccFDSetMinFaceDetected(cc_handle_t handle, float fMinFace);//������С�ɼ����������
FACEDETECTOR_EXPORTS_API float ccFDGetMinFaceDetected(cc_handle_t handle);
// stage two mouse
FACEDETECTOR_EXPORTS_API bool ccActivateStageTwo(cc_handle_t handle, const char *szLandMark);//�������׶�ģ��
FACEDETECTOR_EXPORTS_API void ccDeactivateStageTwo(cc_handle_t handle);                    //�رն��׶�
// stage two eye
FACEDETECTOR_EXPORTS_API bool ccActivateStageTwoEye(cc_handle_t handle, const char *szLandMark);//�����۲����׶�ģ��
FACEDETECTOR_EXPORTS_API void ccDeactivateStageTwoEye(cc_handle_t handle);                    //�رն��׶�
//
FACEDETECTOR_EXPORTS_API void ccFDSetLandmarkFilter(cc_handle_t handle, float fValue);//�����˲�ϵ����0~1.0��Խ���ȶ���Խ���ͺ�Խ����
FACEDETECTOR_EXPORTS_API float ccFDGetLandmarkFilter(cc_handle_t handle);
FACEDETECTOR_EXPORTS_API void ccFDSetImgFilter(cc_handle_t handle, bool bValue);//��Ƶ�˲����ء�Ĭ�Ϲء�����˲����ú�ؼ���������΢�ͺ󵫸��ȣ���Ҫ��һ�����ٵ㶶�������ͺ����е�Ӧ�ÿ��Դ򿪴�ѡ�
FACEDETECTOR_EXPORTS_API void ccFDActivateEyeExpression(cc_handle_t handle);    //�򿪱�����
FACEDETECTOR_EXPORTS_API void ccFDDeactivateEyeExpression(cc_handle_t handle);  //�رձ�����
#if defined(_MSC_VER)
FACEDETECTOR_EXPORTS_API void ccFDDetect(cc_handle_t handle, unsigned char *pBuffer, int width, int height, int rowBytes, int ctType);//��Ƶ���
#else
FACEDETECTOR_EXPORTS_API void ccFDDetect(cc_handle_t handle, unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, bool bPoseEst);
#endif
FACEDETECTOR_EXPORTS_API int ccPicFD(cc_handle_t handle, unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, FaceRect_List faceRects);//ͼƬ�������ӿ�
FACEDETECTOR_EXPORTS_API int ccPicFA(cc_handle_t handle, unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, float* landmark);//����ͼƬ�����ؼ�����
FACEDETECTOR_EXPORTS_API void ccFDGetFaceRes(cc_handle_t handle, ccFDFaceRes *res);//��ȡ��Ƶ�����
FACEDETECTOR_EXPORTS_API void ccFDGetFaceResFlipHorizonal(cc_handle_t handle, ccFDFaceRes *res, int width, int height);//��ȡ������������

FACEDETECTOR_EXPORTS_API int ccFDNumFacedetected(cc_handle_t handle);//��ȡ��������
    
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
