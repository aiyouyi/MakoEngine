
#include "CC3DLight.h"
#include <algorithm>
CC3DLight::CC3DLight(void)
{
	m_LightColor = glm::vec3(1.0f);
	m_LightAmbientIntensity = 0.2f;
	m_LightIntensity = 0.8f;
}

CC3DLight::~CC3DLight(void)
{

}

void CC3DLight::SetLightAmbientIntensity(float intensity)
{
	m_LightAmbientIntensity = ((std::max)(0.0f, intensity));
}

void CC3DLight::SetLightDiffuseIntensity(float intensity)
{
	m_LightIntensity = (std::max)(0.0f,  intensity);
}

void CC3DLight::SetLightColor(float r, float g, float b)
{
	m_LightColor.r = ((std::max)(0.0f, ((std::min)(1.0f, r))));
	m_LightColor.g = ((std::max)(0.0f, ((std::min)(1.0f, g))));
	m_LightColor.b = ((std::max)(0.0f, ((std::min)(1.0f, b))));
}


CC3DDirectionalLight::CC3DDirectionalLight(void)
{
	m_LightDirection = glm::vec3(0.0f, 0.0, -1.0f);
}

CC3DDirectionalLight::~CC3DDirectionalLight(void)
{

}

void CC3DDirectionalLight::SetLightDirection(float x, float y, float z)
{
	SetLightDirection(glm::vec3(x, y, z));
}

void CC3DDirectionalLight::SetLightDirection(glm::vec3 dir)
{
	m_LightDirection = glm::normalize(dir);
}



