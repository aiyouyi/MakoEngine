#pragma once
#include <atomic>
#include <string>
#include <vector>

class CEffectPart;

struct FaceOriImageRect
{
	float  x = 0;
	float  y = 0;
	float  width = 0;
	float  height = 0;
	float Scalexy = 1;
};

enum FacePointType
{
	FACE_POINT_130,
	FACE_POINT_106,
};

enum CCEffect_status
{
	CES_IDLE,
	CES_LOADING,
	CES_READY,
	CES_RUN
};

struct CCResourceAsyn
{
	void reset()
	{
		//input
	//	Release();
		szPath = "";
		szXMLFile = "";
		status = CES_IDLE;
		m_UseDepth = false;
	}
	void Release()
	{
		//		for (int i = 0; i < m_AllEffectPart.size(); ++i)
		//		{
		//			SAFE_DELETE(m_AllEffectPart[i]);
		//		}
		m_AllEffectPart.clear();
	}
	//input
	std::string szPath;
	std::string szXMLFile;
	std::string m_resourcePath;

	//CCEffect_status status;
	std::atomic<CCEffect_status>status;
	std::vector<CEffectPart*>m_AllEffectPart;
	long m_nEffectDuring;
	bool m_UseDepth;

};

typedef void(*EffectCallback)(int event, const void* lpParam);