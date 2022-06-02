#ifndef HUMAN_PARSING_DEF_H
#define HUMAN_PARSING_DEF_H


#	if defined(_MSC_VER)
#		ifdef  HUMAN_PARSING_EXPORTS
#			define HUMAN_PARSING_EXPORTS_API extern "C" __declspec(dllexport)
#		else
#			define HUMAN_PARSING_EXPORTS_API extern "C" __declspec(dllimport)
#		endif
#	else
#		ifdef __cplusplus
#			define HUMAN_PARSING_EXPORTS_API extern "C"
#		else
#			define HUMAN_PARSING_EXPORTS_API
#		endif
#	endif


#define HUMAN_PARSING_SDK_VERSION 1.0.0

enum HP_CT_TYPE
{
	HP_CT_BGRA,
	HP_CT_RGBA,
	HP_CT_RGB,
	HP_CT_BGR
};

#endif // HUMAN_PARSING_DEF_H