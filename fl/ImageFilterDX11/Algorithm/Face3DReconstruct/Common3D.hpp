#ifndef MTLAB_COMMON_COMMON_HPP_lym247943
#define MTLAB_COMMON_COMMON_HPP_lym247943

/// include all basics headers 
#include <algorithm>
#include <stdio.h>
#include <string.h>

// #include "MT3DFace/Common/MTDefine.h"
// #include "MT3DFace/Common/MTLog.h"
// #include "MT3DFace/Common/MTModels.h"
// #include "MT3DFace/Common/MTTypes.h"
// #include "MT3DFace/Common/MT3DImage.h"
// #include "MT3DFace/Common/MTPoint.hpp"
// #include "MT3DFace/Common/MTVector.hpp"

//plaform 平台区分
#if defined(_WIN32) || defined(_WIN32_) || defined(WIN32) || defined(_WIN64_) || defined(WIN64) || defined(_WIN64)
#include <io.h>
#define access _access
#define PLATFORM_WINDOWS 1
#elif defined(ANDROID) || defined(_ANDROID_)
#define PLATFORM_ANDROID 1
#elif defined(__linux__)
#define PLATFORM_LINUX      1
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || TARGET_OS_IOS
#define PLATFORM_IOS 1
#elif TARGET_OS_OSX
#define PLATFORM_MAC_OS 1
#endif
#else
#define PLATFORM_UNKNOWN 1
#endif

// EXPORT
#if defined(_WIN32_) || defined(WIN32) || defined(_WIN64_) || defined(WIN64)||defined(_WIN32) || defined(_WIN32_) || defined(WIN32) || defined(_WIN64_) || defined(WIN64) || defined(_WIN64)
	#ifdef MT3DFace_LIB
	#define MT3DFace_EXPORT __declspec(dllexport)
	#define MEITU_TEMPLATE
	// #else
	// #ifndef MT3DFace_SOURCE
	// #define MT3DFace_EXPORT __declspec(dllimport)
	// #define MEITU_TEMPLATE extern
	#else
	#define MT3DFace_EXPORT
	#define MT3DFace_TEMPLATE 
	#endif
	//#endif
#elif defined(_ADNROID_) || defined(ANDROID) || defined(__APPLE__) || defined(__linux__)
	#ifdef MT3DFace_LIB
	#define MT3DFace_EXPORT __attribute__((visibility("default")))
	#define MEITU_TEMPLATE
	#else
	#define MT3DFace_EXPORT
	#define MT3DFace_TEMPLATE extern
	#endif
#else
	#define MT3DFace_EXPORT
	#define MT3DFace_TEMPLATE 
#endif
#endif
