
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


#ifndef BYTE_DEF
#define BYTE_DEF
typedef unsigned char BYTE;
#endif
#ifndef byte_DEF
#define byte_DEF
typedef unsigned char byte;
#endif
//！！！所有平台通用RGBA字节序！！！
#define CC_RED 0
#define CC_GREEN 1
#define CC_BLUE 2
#define CC_ALPHA 3
// math 定义
#include <math.h>
#include <string.h>
#endif /* ARKERNEL_BASE_BASIC_DEFINE_H_ */
