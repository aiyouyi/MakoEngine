#ifndef HANDGESTUREDEF_H
#define HANDGESTUREDEF_H
#if defined(_MSC_VER)
#ifdef  CC_HANDGESTURE_EXPORTS
#define HANDGESTURE_EXPORTS_API extern "C" __declspec(dllexport)
#define HANDGESTURE_EXPORTS_CLASS __declspec(dllexport)
#else
#define HANDGESTURE_EXPORTS_API extern "C" __declspec(dllimport)
#define HANDGESTURE_EXPORTS_CLASS __declspec(dllimport)
#endif

#else
#ifdef __cplusplus
#define HANDGESTURE_EXPORTS_API extern "C"
#define HANDGESTURE_EXPORTS_CLASS
#else
#define HANDPOSE_EXPORTS_API
#define HANDPOSE_EXPORTS_CLASS
#endif

#endif

#define HANDKEYPOINTNUM 21

enum IMAGE_TYPE
{
	IM_ALPHA,
	IM_ALPHA4,
	IM_BGRA,
	IM_RGBA,
	IM_RGB,
	IM_RGB_GREY,
	IM_BGR
};

#endif // #ifndef HANDGESTUREDEF_H
