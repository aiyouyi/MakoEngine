#pragma once
#ifndef _H_CC3D_LIGHT_H_
#define _H_CC3D_LIGHT_H_
#include "Common/CC3DUtils.h"
class CC3DLight 
{
public:
	CC3DLight(void);
	virtual ~CC3DLight(void);

	//设置光源颜色归一化
	virtual void SetLightColor(float r, float g, float b);
	//设置环境光强度
	virtual void SetLightAmbientIntensity(float intensity);
	//设置漫反射光强度 
	virtual void SetLightDiffuseIntensity(float intensity);
public:
	//颜色0.0-1.0
	glm::vec3 m_LightColor;
	//环境光强度0.0-1.0
	float m_LightAmbientIntensity;
	//漫反射光强度0.0-
	float m_LightIntensity;
};

class CC3DDirectionalLight : public CC3DLight
{
public:
	CC3DDirectionalLight(void);
	virtual ~CC3DDirectionalLight(void);
	//设置光照方向
	virtual void SetLightDirection(float x, float y, float z);
	virtual void SetLightDirection(glm::vec3 dir);
public:
	//平行光方向-normalize
	glm::vec3 m_LightDirection;
};

#endif // _H_CC3D_LIGHT_H_
