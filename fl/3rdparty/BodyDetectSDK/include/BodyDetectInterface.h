#ifndef BODY_DETECT_INTERFACE_H
#define BODY_DETECT_INTERFACE_H

#include "BodyDetectDef.h"


//======================================SDK接口=========================================//
//---------------------------------------------------------------------------------------
/**Init 初始化
 * @param szModelPath 模型文件目录，人体检测器、2D关键点、3D关键点等模型文件应放置于此目录下
 * @param deviceMode 硬件模式，0 CPU, 1 GPU
*/
BODY_DETECT_EXPORTS_API cc_handle_t ccBDInit(const char* szModelPath, int deviceMode);

//---------------------------------------------------------------------------------------
/**Uninit 释放*/
BODY_DETECT_EXPORTS_API void ccBDUninit(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**Reset 重置*/
BODY_DETECT_EXPORTS_API void ccBDReset(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**Detect 检测
 * @param pBuffer image data
 * @param width image's height
 * @param height image's width
 * @param stride image's row pixels bytes
 * @param ctType image's format, BGRA/RGBA/BGR/RGB
 * @param screenType 画面分屏类型，支持标准横向二分屏和三分屏，1：全屏，2：二分屏，3：三分屏，
 此参数非零时，ccBDGetResult只保留左起第一子分屏区域内的识别结果，其他分屏结果忽略。
*/
BODY_DETECT_EXPORTS_API bool ccBDDetect(cc_handle_t handle, const unsigned char* pBuffer, 
										int width, int height, int stride, int ctType,
										int screenType);

//---------------------------------------------------------------------------------------
/**GetResults 获取结果*/
BODY_DETECT_EXPORTS_API ccBodyRes ccBDGetResult(cc_handle_t handle);


//====================================ROI检测接口========================================//
//---------------------------------------------------------------------------------------
/**ccBDSetRoiNumMax 设置人体ROI检测的最大Roi数量
 * @param numRoi Max ROI number
*/
BODY_DETECT_EXPORTS_API void ccBDSetRoiNumMax(cc_handle_t handle, int numRoi);

//---------------------------------------------------------------------------------------
/**ccBDSetRoiDetectSkipFrames 设置人体ROI检测跳帧数，ROI检测器跳帧执行以降低计算量
 * @param numSkipFrames Roi detector skip frames
*/
BODY_DETECT_EXPORTS_API void ccBDSetRoiDetectSkipFrames(cc_handle_t handle, int numSkipFrames);


//====================================Pose2D接口========================================//
//---------------------------------------------------------------------------------------
/**ccBDActivatePose2D 开启Pose2D估计*/
BODY_DETECT_EXPORTS_API bool ccBDActivatePose2D(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccBDDeactivatePose2D 关闭Pose2D估计*/
BODY_DETECT_EXPORTS_API void ccBDDeactivatePose2D(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccBDActivate2DPoseTracking 开启Pose2D跟踪*/
BODY_DETECT_EXPORTS_API bool ccBDActivate2DPoseTracking(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccBDDeactivate2DPoseTracking 关闭2DPose跟踪*/
BODY_DETECT_EXPORTS_API void ccBDDeactivate2DPoseTracking(cc_handle_t handle);

//---------------------------------------------------------------------------------------
/**ccBDSet2DPoseTrackFrames 设置2DPose跟踪帧数
 * @param numTrackFrames 2DPose Keypoint tracking frames
*/
BODY_DETECT_EXPORTS_API void ccBDSet2DPoseTrackFrames(cc_handle_t handle, int numTrackFrames);


//====================================Pose3D接口========================================//

//---------------------------------------------------------------------------------------

#endif // BODY_DETECT_INTERFACE_H