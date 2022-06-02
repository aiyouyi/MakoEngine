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
	ccFDPoint extraEye[38];							 //������ͫ��
	bool own_extraEye_left, own_extraEye_right;      //��ǰ�Ƿ���ڳ�����ͫ��
	//ccFDPoint arrShapeProj[68];
	//������̬��λ��
	float yaw;
	float pitch;
	float roll;
	float x, y, z;
	float arrWMatrix[16];
	// �Ƿ񸲸��첿
	bool cover_mouth;
	// ������̬���ƿ�λ��
	int gesture_box_left, gesture_box_right, gesture_box_top, gesture_box_bottom;
	// ��������
	ccFDFaceExpression faceExpression;
}ccFDShape68;


typedef struct ccFDFaceRes_t
{
	ccFDShape68 arrFace[CC_MAX_FACE_COUNT];//���������ľ�����Ϣ
	int numFace;//��������������
	float matPerspective[16];
}ccFDFaceRes;

FACEDETECTOR_EXPORTS_API bool ccFDInit(const char *szLandMark);//ģ�ͳ�ʼ���ӿ�
FACEDETECTOR_EXPORTS_API void ccFDUnInit();
FACEDETECTOR_EXPORTS_API void ccFDReset();                    //���ã��������������Ϣ��
FACEDETECTOR_EXPORTS_API void ccFDSetMaxDetectedFace(int num);//�������ɼ����������
FACEDETECTOR_EXPORTS_API int  ccFDGetMaxDetectedFace();
FACEDETECTOR_EXPORTS_API void ccFDSetMinFaceDetected(float fMinFace);//������С�ɼ����������
FACEDETECTOR_EXPORTS_API float ccFDGetMinFaceDetected();
// stage two mouse
FACEDETECTOR_EXPORTS_API bool ccActivateStageTwo(const char *szLandMark);//�������׶�ģ��
FACEDETECTOR_EXPORTS_API void ccDeactivateStageTwo();                    //�رն��׶�
// stage two eye
FACEDETECTOR_EXPORTS_API bool ccActivateStageTwoEye(const char *szLandMark);//�����۲����׶�ģ��
FACEDETECTOR_EXPORTS_API void ccDeactivateStageTwoEye();                    //�رն��׶�
//
FACEDETECTOR_EXPORTS_API void ccFDSetLandmarkFilter(float fValue);//�����˲�ϵ����0~1.0��Խ���ȶ���Խ���ͺ�Խ����
FACEDETECTOR_EXPORTS_API float ccFDGetLandmarkFilter();
FACEDETECTOR_EXPORTS_API void ccFDSetImgFilter(bool bValue);//��Ƶ�˲����ء�Ĭ�Ϲء�����˲����ú�ؼ���������΢�ͺ󵫸��ȣ���Ҫ��һ�����ٵ㶶�������ͺ����е�Ӧ�ÿ��Դ򿪴�ѡ�
FACEDETECTOR_EXPORTS_API void ccFDActivateEyeExpression();    //�򿪱�����
FACEDETECTOR_EXPORTS_API void ccFDDeactivateEyeExpression();  //�رձ�����
#if defined(_MSC_VER)
FACEDETECTOR_EXPORTS_API void ccFDDetect(unsigned char *pBuffer, int width, int height, int rowBytes, int ctType);//��Ƶ���
#else
FACEDETECTOR_EXPORTS_API void ccFDDetect(unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, bool bPoseEst);
#endif
FACEDETECTOR_EXPORTS_API int ccPicFD(unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, FaceRect_List faceRects);//ͼƬ�������ӿ�
FACEDETECTOR_EXPORTS_API int ccPicFA(unsigned char *pBuffer, int width, int height, int rowBytes, int ctType, float* landmark);//����ͼƬ�����ؼ�����
FACEDETECTOR_EXPORTS_API void ccFDGetFaceRes(ccFDFaceRes *res);//��ȡ��Ƶ�����
FACEDETECTOR_EXPORTS_API void ccFDGetFaceResFlipHorizonal(ccFDFaceRes *res, int width, int height);//��ȡ������������

FACEDETECTOR_EXPORTS_API int ccFDNumFacedetected();//��ȡ��������
    
FACEDETECTOR_EXPORTS_API bool ccFDGetFaceDetected(float *arrFaceRect, int index);
FACEDETECTOR_EXPORTS_API bool ccFDGetFaceDetectedLast(float *arrFaceRect, int index);

FACEDETECTOR_EXPORTS_API int ccFDGetFaceShape2D(float *arrShape, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetHeaderRotate(float *arrRotate, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetHeaderTrans(float *arrTrans, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetHeaderMatrix(float *arrMatrix, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetFilterHeaderRotate(float *arrRotate, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetFilterHeaderTrans(float *arrTrans, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetFilterHeaderMatrix(float *arrMatrix, int index);
    
FACEDETECTOR_EXPORTS_API int ccFDGetPerspectiveMatrix(float *arrMatrix);
#endif
