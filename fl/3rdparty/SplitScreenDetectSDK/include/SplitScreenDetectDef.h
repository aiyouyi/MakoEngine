#ifndef SPLIT_SCREEN_DETECT_DEF_H
#define SPLIT_SCREEN_DETECT_DEF_H


#	if defined(_MSC_VER)
#		ifdef  SPLIT_SCREEN_DETECT_EXPORT
#			define SPLIT_SCREEN_DETECT_EXPORTS_API extern "C" __declspec(dllexport)
#		else
#			define SPLIT_SCREEN_DETECT_EXPORTS_API extern "C" __declspec(dllimport)
#		endif
#	else
#		ifdef __cplusplus
#			define SPLIT_SCREEN_DETECT_EXPORTS_API extern "C"
#		else
#			define SPLIT_SCREEN_DETECT_EXPORTS_API
#		endif
#	endif


#define SPLIT_SCREEN_DETECT_SDK_VERSION 1.0.3

enum SSD_SCREEN_TYPE {
	SSD_SCREEN_TYPE_FULL,   		 //全屏
	SSD_SCREEN_TYPE_TWO_REPLICATE, 	 //二分屏，复制
	SSD_SCREEN_TYPE_TWO_MIRROR,   	 //二分屏，镜像
	SSD_SCREEN_TYPE_THREE_REPLICATE, //三分屏，复制
};

enum SSD_CT_TYPE {
	SSD_CT_BGRA,
	SSD_CT_RGBA,
	SSD_CT_BGR,
	SSD_CT_RGB,
};

#endif // SPLIT_SCREEN_DETECT_DEF_H