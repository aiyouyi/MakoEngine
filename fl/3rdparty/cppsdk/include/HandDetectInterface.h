#ifndef HAND_DETECT_INTERFACE_H
#define HAND_DETECT_INTERFACE_H

#include "HandDetectDef.h"

//======================================SDK接口=========================================//
//---------------------------------------------------------------------------------------
/**ccHDInit 初始化
 * @param szModelPath 模型文件目录，手检测器、关键点、手势分类等模型文件应放置于此目录下
 * @param deviceMode 硬件模式，0 CPU, 1 GPU
*/
HAND_DETECT_EXPORTS_API cc_handle_t ccHDInit(const char* szModelPath, int deviceMode); 

//---------------------------------------------------------------------------------------
/**ccHDUnInit 释放*/
HAND_DETECT_EXPORTS_API void ccHDUnInit(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDReset 重置*/
HAND_DETECT_EXPORTS_API void ccHDReset(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDDetect 检测
 * @param pBuffer image data
 * @param width image's height
 * @param height image's width
 * @param stride image's row pixels bytes
 * @param ctType image's format, BGRA/RGBA/BGR/RGB
*/
HAND_DETECT_EXPORTS_API bool ccHDDetect(cc_handle_t handle, const unsigned char* pBuffer, 
										int width, int height, int stride, int ctType);

//---------------------------------------------------------------------------------------
/**ccHDGetResult 获取结果*/
HAND_DETECT_EXPORTS_API ccHandRes ccHDGetResult(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDGetGestureResult 获取结果*/
HAND_DETECT_EXPORTS_API ccGestureRes ccHDGetGestureResult(cc_handle_t handle);

//====================================ROI检测接口========================================//
//---------------------------------------------------------------------------------------
/**ccHDSetMaxHandNum 设置手检测数量上限*/
HAND_DETECT_EXPORTS_API void ccHDSetMaxHandNum(cc_handle_t handle, int numHand);

//---------------------------------------------------------------------------------------
/**ccHDSetRoiDetectSkipFrames 设置手ROI检测跳帧数*/
HAND_DETECT_EXPORTS_API void ccHDSetRoiDetectSkipFrames(cc_handle_t handle, int numSkipFrames);


//====================================Pose接口========================================//
//---------------------------------------------------------------------------------------
/**ccHDActivateHandPose 开启Pose关键点估计*/
HAND_DETECT_EXPORTS_API bool ccHDActivateHandPose(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDDeactivateHandPose 关闭Pose关键点估计*/
HAND_DETECT_EXPORTS_API void ccHDDeactivateHandPose(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDSetPoseSmoothWeight 设置关键点平滑滤波强度，0.0~1.0之间，取值越大越稳定，但延迟越高*/
HAND_DETECT_EXPORTS_API void ccHDSetPoseSmoothWeight(cc_handle_t handle, float value);


//====================================Gesture接口========================================//
//---------------------------------------------------------------------------------------
// 单手手势
//---------------------------------------------------------------------------------------
/**ccHDActivateSingleGesture 开启单手手势识别*/
HAND_DETECT_EXPORTS_API bool ccHDActivateSingleGesture(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDDeactivateSingleGesture 关闭单手手势识别*/
HAND_DETECT_EXPORTS_API void ccHDDeactivateSingleGesture(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDSetSingleGestureRelaxation 手势分类松弛系数，越大越容易分为手势，0.0~10.0之间*/
HAND_DETECT_EXPORTS_API void ccHDSetSingleGestureRelaxation(cc_handle_t handle, float value);

// 双手手势
//---------------------------------------------------------------------------------------
/**ccHDActivateDoubleGesture 开启双手手势识别*/
HAND_DETECT_EXPORTS_API bool ccHDActivateDoubleGesture(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDDeactivateSingleGesture 关闭单手手势识别*/
HAND_DETECT_EXPORTS_API void ccHDDeactivateDoubleGesture(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccHDSetDoubleGestureRelaxation 手势分类松弛系数，越大越容易分为手势，0.0~10.0之间*/
HAND_DETECT_EXPORTS_API void ccHDSetDoubleGestureRelaxation(cc_handle_t handle, float value);

#endif // HAND_DETECT_INTERFACE_H