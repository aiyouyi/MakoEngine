#ifndef HAND_DETECT_INTERFACE_H
#define HAND_DETECT_INTERFACE_H

#include "HandDetectDef.h"

//======================================SDK�ӿ�=========================================//
//---------------------------------------------------------------------------------------
/**ccHDInit ��ʼ��
 * @param szModelPath ģ���ļ�Ŀ¼���ּ�������ؼ��㡢���Ʒ����ģ���ļ�Ӧ�����ڴ�Ŀ¼��
 * @param deviceMode Ӳ��ģʽ��0 CPU, 1 GPU
*/
HAND_DETECT_EXPORTS_API cc_handle_t ccHDInit(const char* szModelPath, int deviceMode); 

//---------------------------------------------------------------------------------------
/**ccHDUnInit �ͷ�*/
HAND_DETECT_EXPORTS_API void ccHDUnInit(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDReset ����*/
HAND_DETECT_EXPORTS_API void ccHDReset(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDDetect ���
 * @param pBuffer image data
 * @param width image's height
 * @param height image's width
 * @param stride image's row pixels bytes
 * @param ctType image's format, BGRA/RGBA/BGR/RGB
*/
HAND_DETECT_EXPORTS_API bool ccHDDetect(cc_handle_t handle, const unsigned char* pBuffer, 
										int width, int height, int stride, int ctType);

//---------------------------------------------------------------------------------------
/**ccHDGetResult ��ȡ���*/
HAND_DETECT_EXPORTS_API ccHandRes ccHDGetResult(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDGetGestureResult ��ȡ���*/
HAND_DETECT_EXPORTS_API ccGestureRes ccHDGetGestureResult(cc_handle_t handle);

//====================================ROI���ӿ�========================================//
//---------------------------------------------------------------------------------------
/**ccHDSetMaxHandNum �����ּ����������*/
HAND_DETECT_EXPORTS_API void ccHDSetMaxHandNum(cc_handle_t handle, int numHand);

//---------------------------------------------------------------------------------------
/**ccHDSetRoiDetectSkipFrames ������ROI�����֡��*/
HAND_DETECT_EXPORTS_API void ccHDSetRoiDetectSkipFrames(cc_handle_t handle, int numSkipFrames);


//====================================Pose�ӿ�========================================//
//---------------------------------------------------------------------------------------
/**ccHDActivateHandPose ����Pose�ؼ������*/
HAND_DETECT_EXPORTS_API bool ccHDActivateHandPose(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDDeactivateHandPose �ر�Pose�ؼ������*/
HAND_DETECT_EXPORTS_API void ccHDDeactivateHandPose(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDSetPoseSmoothWeight ���ùؼ���ƽ���˲�ǿ�ȣ�0.0~1.0֮�䣬ȡֵԽ��Խ�ȶ������ӳ�Խ��*/
HAND_DETECT_EXPORTS_API void ccHDSetPoseSmoothWeight(cc_handle_t handle, float value);


//====================================Gesture�ӿ�========================================//
//---------------------------------------------------------------------------------------
// ��������
//---------------------------------------------------------------------------------------
/**ccHDActivateSingleGesture ������������ʶ��*/
HAND_DETECT_EXPORTS_API bool ccHDActivateSingleGesture(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDDeactivateSingleGesture �رյ�������ʶ��*/
HAND_DETECT_EXPORTS_API void ccHDDeactivateSingleGesture(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDSetSingleGestureRelaxation ���Ʒ����ɳ�ϵ����Խ��Խ���׷�Ϊ���ƣ�0.0~10.0֮��*/
HAND_DETECT_EXPORTS_API void ccHDSetSingleGestureRelaxation(cc_handle_t handle, float value);

// ˫������
//---------------------------------------------------------------------------------------
/**ccHDActivateDoubleGesture ����˫������ʶ��*/
HAND_DETECT_EXPORTS_API bool ccHDActivateDoubleGesture(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDDeactivateSingleGesture �رյ�������ʶ��*/
HAND_DETECT_EXPORTS_API void ccHDDeactivateDoubleGesture(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDSetDoubleGestureRelaxation ���Ʒ����ɳ�ϵ����Խ��Խ���׷�Ϊ���ƣ�0.0~10.0֮��*/
HAND_DETECT_EXPORTS_API void ccHDSetDoubleGestureRelaxation(cc_handle_t handle, float value);

#endif // HAND_DETECT_INTERFACE_H