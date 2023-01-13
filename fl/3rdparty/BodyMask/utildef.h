#ifndef __FURNACE_UTIL_DEF_H__
#define __FURNACE_UTIL_DEF_H__
#if defined(_MSC_VER)  //_MSC_VER是MS VC++编译器版本的值
#pragma warning( disable: 4251 )
#pragma warning( disable: 4819 )
#ifdef FURNACE_EXPORTS
#define FURNACE_API __declspec(dllexport)
#else
#define FURNACE_API __declspec(dllimport)
#endif
#else
#define FURNACE_API
#endif
#endif
