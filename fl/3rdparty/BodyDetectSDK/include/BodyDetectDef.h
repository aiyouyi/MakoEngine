#ifndef BODY_DETECT_DEF_H
#define BODY_DETECT_DEF_H


#	if defined(_MSC_VER)
#		ifdef  BODY_DETECT_EXPORTS
#			define BODY_DETECT_EXPORTS_API extern "C" __declspec(dllexport)
#		else
#			define BODY_DETECT_EXPORTS_API extern "C" __declspec(dllimport)
#		endif
#	else
#		ifdef __cplusplus
#			define BODY_DETECT_EXPORTS_API extern "C"
#		else
#			define BODY_DETECT_EXPORTS_API
#		endif
#	endif


#define BODY_DETECT_SDK_VERSION 1.1.5
#define BODY_DETECTOR_MODEL_FILE "body_detector_v1.0.0"
#define BODY_POSE_2D_MODEL_FILE "body_pose_2d_v1.0.0"

#define BODY_KEYPOINTS_NUM 16
#define BODY_MAX_NUM 5

typedef struct ccBodyRect_t {
	float left;
	float top;
	float right;
	float bottom;
	float score;
}ccBodyRect;

typedef struct ccBodyPoint_t {
	float x;
	float y;
	float z;
	float score;
}ccBodyPoint;

typedef struct ccBodyInfo_t {
	ccBodyRect bodyRect;
	ccBodyPoint keyPointArr[BODY_KEYPOINTS_NUM];
}ccBodyInfo;

typedef struct ccBodyRes_t {
	ccBodyInfo arrBody[BODY_MAX_NUM];
	int numBody;
}ccBodyRes;

enum BD_CT_TYPE {
	BD_CT_BGRA,
	BD_CT_RGBA,
	BD_CT_BGR,
	BD_CT_RGB,
};

typedef void * cc_handle_t;

#endif // BODY_DETECT_DEF_H