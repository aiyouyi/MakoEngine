#ifndef __CCBASELIGHT_H__
#define __CCBASELIGHT_H__

#include "math/CCMath.h"
#include "math/Vec4.h"

NS_CC_BEGIN
class BaseLight
{
public:
	BaseLight() {}
	~BaseLight() {}

	cocos2d::Vec4 m_vDiffuse;
	cocos2d::Vec4 m_vAmbient;
	cocos2d::Vec4 m_vSpecular;
	cocos2d::Vec4 m_vOrientation;
};

struct NPRInfo
{
	bool m_bUseNPR;
	float m_fLineWidth;
	cocos2d::Vec4 m_vLineColor;
	cocos2d::Vec3 m_vZOffest;
	
	void initDefault() {
		m_bUseNPR = false;
		m_fLineWidth = 0;
		m_vLineColor = cocos2d::Vec4(0.5f, 0.5f, 0.5f, 1);
		m_vZOffest = cocos2d::Vec3(0.9f, 10.0f, 0.0002f);
	}
};
#endif
NS_CC_END