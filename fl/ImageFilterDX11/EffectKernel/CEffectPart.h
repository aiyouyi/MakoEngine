#pragma once
#include "Toolbox/xmlParser.h"
#include "BaseRenderParam.h"
#include "Toolbox/zip/unzip.h"
#include "common.h"
#include "AnimInfo.h"

//=======������Ч����ʱ��=======
struct EffectPlayControl  
{
	long delaytime = 0;
	long playtime = -1;
	long alltime = -1;
};
//============================

/******0:正常 1:正片叠底 2:柔光 3:滤色 4:变暗 5:颜色加深 6:线性加深 7:深色 8:变亮 9:颜色减淡 10:线性减淡 11:浅色 12:叠加******/
enum BlendType
{
	NORMAL_TYPE = 0,//正常
	MULTIPLY_TYPE,  //正片叠底
	SOFTLIGHT_TYPE, //柔光
	SCREEN_TYPE,    //��ɫ
	DARKEN_TYPE,    //�䰵
	COLOR_BURN_TYPE,//��ɫ����
	LINEAR_BURN_TYPE,//���Լ���
	DARKER_COLOR_TYPE,//��ɫ
	LIGHTEN_TYPE,    //����
	COLOR_DODGE_TYPE,//��ɫ����
	LINEAR_DODGE_TYPE,//���Լ���
	LIGHTER_COLOR_TYPE,//ǳɫ
	OVERLAY_TYPE,     //����
};
//============================

class CEffectPart
{
public:
	CEffectPart();
	virtual ~CEffectPart();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath = NULL);

	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);

	virtual bool WriteConfig(XMLNode& root, HZIP dst = 0, HZIP src = 0);
	virtual bool WriteConfig(std::string &tempPath, XMLNode& root, HZIP dst=0, HZIP src = 0);

	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	virtual void Release();
	virtual long GetRunTime();
	virtual BlendType GetBlendType(const char *szBlendType);
	virtual float GetBlendParm(BlendType &BlendType);
	const char* getShowName();
	void setShowName(const char* showname);
	static void createEffect(char* path, const char* type, CEffectPart* part);

	virtual void setAnim(AnimInfo& info);
	virtual void getAnim(AnimInfo &info);
	virtual void setAnimFps(float fps);
    void SetResourcePath(std::string &path);

	EffectPlayControl m_play;
	bool m_bEnableReder = true;
	bool m_bEnableWrite = true;
	long m_runTime = 0;
	float m_alpha = 1.0;
	CCEffectType m_EffectPart = UNKNOW_EFFECT;
	bool m_MSAA = false;
	BlendType m_BlendType = NORMAL_TYPE;
	std::string m_showname;
	std::string m_BlendName = "Normal";
	std::map<std::string, BlendType> m_BlendMap = { 
		{"Normal", NORMAL_TYPE}, {"Multiply", MULTIPLY_TYPE}, {"Softlight", SOFTLIGHT_TYPE}, {"Screen", SCREEN_TYPE}, {"Darken", DARKEN_TYPE}, {"Colorburn", COLOR_BURN_TYPE}, {"Linearburn", LINEAR_BURN_TYPE},
	    {"Darkercolor", DARKER_COLOR_TYPE}, {"Lighten", LIGHTEN_TYPE}, {"Colordodge", COLOR_DODGE_TYPE}, {"Lineardodge", LINEAR_DODGE_TYPE}, {"Lightercolor", LIGHTER_COLOR_TYPE}, {"Overlay", OVERLAY_TYPE},
	};

	long long m_anim_id;
	int m_index;

protected:
	ID3D11BlendState *m_pBlendStateNormal;
	ID3D11SamplerState* m_pSamplerLinear;
	DX11Shader *m_pShader = nullptr; //ֻ����ָ�룬�����������������ڹ���
	std::string m_resourcePath;


};