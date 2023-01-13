#ifndef HAND_DETECT_DEF_H
#define HAND_DETECT_DEF_H

#	if defined(_MSC_VER)
#		ifdef  HAND_DETECT_EXPORTS
#			define HAND_DETECT_EXPORTS_API extern "C" __declspec(dllexport)
#		else
#			define HAND_DETECT_EXPORTS_API extern "C" __declspec(dllimport)
#		endif
#	else
#		ifdef __cplusplus
#			define HAND_DETECT_EXPORTS_API extern "C"
#		else
#			define HAND_DETECT_EXPORTS_API
#		endif
#	endif


#define HAND_DETECT_SDK_VERSION 0.0.6
#define HAND_POSE_MODEL_FILE_CPU "hand_pose_cpu_v1.0.0"
#define HAND_POSE_MODEL_FILE_GPU "hand_pose_gpu_v1.0.0"
#define HAND_DETECTOR_MODEL_FILE "hand_detect_v1.0.0"
#define SINGLE_GESTURE_MODEL_FILE "hand_gesture_single_v1.0.0"
#define DOUBLE_GESTURE_MODEL_FILE "hand_gesture_double_v1.0.0"

#define HAND_KEYPOINT_NUM 21
#define HAND_MAX_NUM 6

// 手包围框结构
typedef struct ccHandRect_t {
	float left;
	float top;
	float right;
	float bottom;
	float score;
}ccHandRect;

// 每个关键点的坐标结构
typedef struct ccHandPoint_t {
	float x;
	float y;
	float z;
	float score;
}ccHandPoint;

// 每只手的信息结构
typedef struct ccHandInfo_t {
	ccHandRect handRect;	// 手包围框
	ccHandPoint keyPointArr[HAND_KEYPOINT_NUM]; // 手部21个关键点位置
	int handType; // 0右手，1左手
	int gestureType; // 手势识别类型
}ccHandInfo;

// 手势识别结果结构
typedef struct ccGesture_t {
	ccHandRect handRect;	// 手包围框
	int gestureType; // 手势识别类型
}ccGesture;

typedef struct ccHandRes_t {
	ccHandInfo arrHand[HAND_MAX_NUM]; // 每只手的信息
	int numHand; // 当前结果中手的数量
}ccHandRes;

typedef struct ccGestureRes_t {
	ccGesture arrGesture[HAND_MAX_NUM]; // 每个手势的信息
	int numGesture; // 当前检测到的手势数量
}ccGestureRes;

enum HD_CT_TYPE
{
	HD_CT_BGRA,
	HD_CT_RGBA,
	HD_CT_RGB,
	HD_CT_BGR
};

typedef void * cc_handle_t;

#endif // HAND_DETECT_DEF_H
