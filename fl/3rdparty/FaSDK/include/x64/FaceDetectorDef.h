#ifndef FaceDetectorDef_h
#define FaceDetectorDef_h

#if defined(_MSC_VER)
#ifdef  FACEDETECTOR_EXPORTS
#define FACEDETECTOR_EXPORTS_API extern "C" __declspec(dllexport)
#define FACEDETECTOR_EXPORTS_CLASS __declspec(dllexport)
#else
#define FACEDETECTOR_EXPORTS_API extern "C" __declspec(dllimport)
#define FACEDETECTOR_EXPORTS_CLASS __declspec(dllimport)
#endif

#else
#ifdef __cplusplus
#define FACEDETECTOR_EXPORTS_API extern "C"
#define FACEDETECTOR_EXPORTS_CLASS
#else
#define FACEDETECTOR_EXPORTS_API
#define FACEDETECTOR_EXPORTS_CLASS
#endif

#endif
#define FACE_LIB_VERSION 2.0.4
#define USE_MNN
#define NUMKEYPOINTS 130
enum CT_TYPE
{
	CT_ALPHA,
	CT_ALPHA4,
	CT_BGRA,
	CT_RGBA,
	CT_RGB,
	CT_RGB_GREY,
	CT_BGR
};
#endif
