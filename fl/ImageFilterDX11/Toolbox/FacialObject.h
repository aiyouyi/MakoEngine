#pragma once
#include "FSObject.h"
#include "BaseDefine/Vectors.h"
class FacialObject;
struct FacialCallbackInfo:public Ref
{
	FacialCallbackInfo()
	{
		pTarget = NULL;
		event = 0;
		pCallBack = NULL;
		pUserInfo = NULL;
	}

	void setInfo(FacialObject *pTarget, string szAnimationName, int event, ActionUserCallback pCallBack, void *pUserInfo)
	{
		this->pTarget = pTarget;
		this->szAnimationName = szAnimationName;
		this->event = event;
		this->pCallBack = pCallBack;
		this->pUserInfo = pUserInfo;
	}

	void *pUserInfo;

	FacialObject *pTarget;
	string szAnimationName;
	int event;

	ActionUserCallback pCallBack;
};

class FacialObject :public FSObject
{
public:
	static FacialObject *createFromZip(const std::string &file, HZIP hZip, const std::string &zip);

	FacialObject() 
	{ 
		m_autoPlay = false; 
		m_fNeckContrlWeight = 0.0f; 
		m_fWeight = 0.01f;
		m_szNeckBone = "neck";
		m_szNeckBone2 = "neck01";
	}
	virtual ~FacialObject();

	void setNeckBone(const char *szNeckBone, const char *szNeckBone2);

	//‰÷»æÀ≥–Ú....
	virtual void updateFacialInfo(float *arrExpAlpha);
	void updateNeckRotate(float *arrQ);

	void updateEyeRotate(Vector2 *pFacePoint,float *eyeBlinkParam);

	void playExt(const char *szID, ActionUserCallback pCallBack = NULL, void *pUserInfo = NULL);

	virtual void updateBlendWeights(int meshIndex, const map<int, float> &mapWeights);

	virtual void update(float ftime);

	void postEvent(FacialCallbackInfo *event);
private:
	bool m_autoPlay;
	float m_fNeckContrlWeight;
	float m_fWeight;

	string m_szNeckBone;
	string m_szNeckBone2;
	vector<FacialCallbackInfo *> m_listCallBack;
};