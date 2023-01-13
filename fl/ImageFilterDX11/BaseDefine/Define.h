
#ifndef CC_BASE_BASIC_DEFINE_H_
#define CC_BASE_BASIC_DEFINE_H_


// 安全释放函数
#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) delete (x); (x) = nullptr; }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) { if (x) delete [] (x); (x) = nullptr; }
#endif
#ifndef SAFE_RELEASE_BUFFER
#define SAFE_RELEASE_BUFFER(x) { if (x) x->Release(); (x) = nullptr; }
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef CC_PI
#define CC_PI 3.1415926f
#define CC_ANGLE_PI 57.296
#endif

static constexpr float LIGHT_DISTANCE = 4.0f;

typedef long long int64;
typedef unsigned long long uint64;

typedef int int32;
typedef unsigned int uint32;

typedef short int16;
typedef unsigned short uint16;

//typedef char int8;
typedef unsigned char uint8;

#define LOG_TAG "CC3DEngine"

#define  LOGE(...)  do { printf("<[" LOG_TAG "] ERROR:> ");printf(__VA_ARGS__);printf("\n");}  while(0)


//#ifndef BYTE_DEF
//#define BYTE_DEF
//typedef unsigned char BYTE;
//#endif
//#ifndef byte_DEF
//#define byte_DEF
//typedef unsigned char byte;
//#endif
//！！！所有平台通用RGBA字节序！！！
#define CC_RED 0
#define CC_GREEN 1
#define CC_BLUE 2
#define CC_ALPHA 3
// math 定义
#include <math.h>
#include <string.h>
#include <stdint.h>

// 取消之前的宏定义
#undef PLATFORM_WINDOWS
#undef PLATFORM_LINUX
#undef PLATFORM_OSX
#undef PLATFORM_IOS
#undef PLATFORM_ANDROID
#undef PLATFORM_UNKNOWN
// 优先使用CMAKE工程预定义平台
#if defined(CMAKE_PLATFORM_WINDOWS)
#define PLATFORM_WINDOWS 1
#elif defined(CMAKE_PLATFORM_OSX)
#define PLATFORM_OSX 1
#elif defined(CMAKE_PLATFORM_LINUX)
#define PLATFORM_LINUX 1
#elif defined(CMAKE_PLATFORM_IOS)
#define PLATFORM_IOS 1
#elif defined(CMAKE_PLATFORM_ANDROID)
#define PLATFORM_ANDROID 1
#elif defined(_WIN32) || defined(_WIN32_) || defined(WIN32) || defined(_WIN64_) || defined(WIN64) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || TARGET_OS_IOS
#define PLATFORM_IOS 1
#elif TARGET_OS_OSX
#define PLATFORM_OSX 1
#endif
#elif defined(ANDROID) || defined(_ANDROID_)
#define PLATFORM_ANDROID 1
#elif defined(__linux__)
#define PLATFORM_LINUX 1
#else
#define PLATFORM_UNKNOWN 1
#endif

#ifndef _WIN64
#ifdef PLATFORM_WINDOWS
#include"gl/glew.h"
#elif defined(PLATFORM_OSX)
#include <OpenGL/gl.h>
#include <OpenGL/gl3.h>
#elif defined(PLATFORM_IOS)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif defined(PLATFORM_ANDROID)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <string>
#else
#error "NO_GL_SUPPORT"
#endif
#endif

#include <ctime>
#if !defined(__ANDROID__) && !defined(__APPLE__)
#include <windows.h>
#endif
#include <stdio.h>
#ifdef __ANDROID__
#include <android/log.h>
#define ALOG(level, TAG, ...) \
    do { \
        __android_log_print(level, TAG, __VA_ARGS__); \
    } while (0);

#define LOGI(...) ALOG(ANDROID_LOG_INFO, "EFFECT", __VA_ARGS__)

#define LOGD(...) ALOG(ANDROID_LOG_DEBUG, "EFFECT", __VA_ARGS__)

#define LOGE(...) ALOG(ANDROID_LOG_ERROR, "EFFECT", __VA_ARGS__)
#else
#endif



#endif /* ARKERNEL_BASE_BASIC_DEFINE_H_ */
