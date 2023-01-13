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

// �ְ�Χ��ṹ
typedef struct ccHandRect_t {
	float left;
	float top;
	float right;
	float bottom;
	float score;
}ccHandRect;

// ÿ���ؼ��������ṹ
typedef struct ccHandPoint_t {
	float x;
	float y;
	float z;
	float score;
}ccHandPoint;

// ÿֻ�ֵ���Ϣ�ṹ
typedef struct ccHandInfo_t {
	ccHandRect handRect;	// �ְ�Χ��
	ccHandPoint keyPointArr[HAND_KEYPOINT_NUM]; // �ֲ�21���ؼ���λ��
	int handType; // 0���֣�1����
	int gestureType; // ����ʶ������
}ccHandInfo;

// ����ʶ�����ṹ
typedef struct ccGesture_t {
	ccHandRect handRect;	// �ְ�Χ��
	int gestureType; // ����ʶ������
}ccGesture;

typedef struct ccHandRes_t {
	ccHandInfo arrHand[HAND_MAX_NUM]; // ÿֻ�ֵ���Ϣ
	int numHand; // ��ǰ������ֵ�����
}ccHandRes;

typedef struct ccGestureRes_t {
	ccGesture arrGesture[HAND_MAX_NUM]; // ÿ�����Ƶ���Ϣ
	int numGesture; // ��ǰ��⵽����������
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
