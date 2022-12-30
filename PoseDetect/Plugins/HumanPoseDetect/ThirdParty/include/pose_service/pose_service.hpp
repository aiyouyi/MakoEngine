#ifndef POSE_SERVICE_HPP
#define POSE_SERVICE_HPP

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define POSE_SERVICE_API __declspec(dllexport)
	// use SDK prefix to avoid name confliction in iOS
	typedef enum {
		SDK_BGR,
		SDK_RGB,
		SDK_I420,
		SDK_NV12,
		SDK_NV21
	} SDKFormat;

	// use SDK prefix to avoid name confliction
	POSE_SERVICE_API void POSEInit();
	POSE_SERVICE_API void POSEProcess(unsigned char* ptr, int height, int width, SDKFormat format, float* mesh_ptr);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // POSE_SERVICE_HPP
