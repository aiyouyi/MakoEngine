#ifndef common_h
#define common_h

#include "DX11ImageFilterDef.h"
#include <d3d11.h>

//==========Version Contral===================
//==========2021.04.01========================
#define LIBVERSION std::string("1.50")
#define LIBVERSION_DOUBLE(V) atof(V.c_str())
//============================================

typedef void * cc_handle_t;

enum EffectEvent
{
	EE_START,
	EE_FINISHED,
	EE_LOOP
};
typedef void(*EffectCallback)(int event, const void *lpParam);

enum kActionEvent
{
	AE_START = 0,
	AE_REPEAT = 1,
	AE_END = 2,
};


enum CCEffectType
{
	//美颜
	SMOOTH_EFFECT = 0,             //磨皮(0,1)
	LUT_FILTER_EFFECT ,            //美白(0,1)
	SHARP_EFFECT,                  //锐化清晰度(0,1)
	DENOISE_EFFECT,               //去噪(0,1)
	SHADOW_HIGHLIGHT_EFFECT,      //立体感(0,1)
	FACE_BRIGHT_EYE,               //亮眼（0,1）;
	FACE_REMOVE_POUCH,            //去黑眼圈（0,1）
	FACE_REMOVE_FALIN,             //去法令纹（0,1）
	LUT_CLEAR_EFFECT,              //lut清晰
	FACE_WHITE_TEETH,              //白牙

	//美型
	FACE_LIFT_EFFECT = 100,         //瘦脸(-1,1)
	FACE_SMALL_EFFECT,             //小脸(-1,1)
	FACE_NARROW_EFFECT,             //窄脸(-1,1)/削脸
	FACE_EYE_BIG_EFFECT,           //大眼(-1,1)
	FACE_EYE_OFFEST_EFFECT,         //眼距(-1,1)
	FACE_EYE_ROTATE_EFFECT,         //眼角(-1,1)
	FACE_CHEEK_BONES,              //颧骨(-1,1)
	FACE_YAMANE_EFFECT,            //山根(-1,1)
	FACE_LOWER_JAW,               //下颌骨（-1,1）
	FACE_FOREHEAD_EFFECT,          //额头(-1,1)/额高
	FACE_CHIN_EFFECT,               //下巴(-1,1)
	FACE_NOSE_EFFECT,               //鼻子(-1,1)/瘦鼻
	FACE_NOSEPOS_EFFECT,             //鼻长(-1,1)/鼻位
	FACE_MOUTHPOS_EFFECT,            //人中(-1,1)/嘴位
	FACE_MOUTH_EFFECT,            //嘴巴(-1,1)
	FACE_FOREHEADW_EFFECT,          //额宽(-1,1)


	//美妆
	FACE_LIPSTRICK = 200,            //口红上色（0,1）;
	FACE_EYESTRICK,                  //眼妆（0,1）;
	FACE_EYELID,                     //双眼皮（0,1）；
	FACE_MOUTH_STRICK,               //嘴巴上妆(0,1)
	FACE_EYEBROWSTRICK,              //眉毛（0,1）;
	FACE_BLUSH        ,              //腮红(0,1)

	FACE_MASK,                       //面具
	FACE_UV_MASK,                     //UV面具
	FACE_BLEND_BLUSH,                //脸部柔光
	FACE_LEVELS,                     //色阶
	DYE_HAIR_EFFECT,               //dye hair effect

	//其他
	BODY_LIFT_EFFECT = 300,         //瘦身(-1,1)
	TYPE_FILTER_EFFECT,             //风格滤镜(0,1);
	FACE_EFFECT_STICKER,            //2D贴纸特效
	FORE_EFFECT_STICKER,            //前景贴纸特效
	SEG_EFFECT_BACKGROUND,          //背景分割特效
	FACE_EFFECT_OFFSETSHAPE,       //人脸形变

	//镜头特效
	SOUL_BODY_EFFECT =350,              //灵魂出窍镜头特效
	GLITCH_EFFECT,                 //毛刺镜头特效
	LIGHTWAVE_EFFECT,              //光波特效
	RHYTHM_EFFECT,                  //律动特效
	FlipSplit,                  //律动特效


	PBR_3D_MODEL = 400,
	FACE_PBR_3D_MODEL,
	FACE_BODY_APART_3D_MODEL,
	FACE_3D_SWITCH_PALATA,          //3D特效替换板
	UNKNOW_EFFECT  =1000,
};


typedef void(*ActionCallback)(int event, const void *lpParam);

typedef void(*ActionUserCallback)(int event, const char *szID, const void *lpParam);

DX11IMAGEFILTER_EXPORTS_API unsigned long ccCurrentTime();

DX11IMAGEFILTER_EXPORTS_API void ccBeginFilter();

DX11IMAGEFILTER_EXPORTS_API void ccEndFilter();

DX11IMAGEFILTER_EXPORTS_API void ccInitFilter(ID3D11Device *pd3dDevice, ID3D11DeviceContext *pContext);
DX11IMAGEFILTER_EXPORTS_API void ccUnInitFilter();
#endif