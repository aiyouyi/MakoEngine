
#include "CC3DModelControl.h"

CC3DModelControl::CC3DModelControl(void)
{
	m_TransMatrix = glm::mat4(1.0f);
	m_ScaleMatrix = glm::mat4(1.0f);
	m_RotateMatrix = glm::mat4(1.0f);
	m_InitMatrix = glm::mat4(1.0f);
	UpdateModelMatrix();
}

CC3DModelControl::~CC3DModelControl(void)
{
	
}

float * CC3DModelControl::Get()
{
	return &m_ModelMatrix[0][0];
}

float * CC3DModelControl::GetNormal()
{
	auto normalMat = m_RotateMatrix * m_InitMatrix;
	return &m_RotateMatrix[0][0];
}

glm::mat4 & CC3DModelControl::GetModelMatrix()
{
	return m_ModelMatrix;
}

void CC3DModelControl::SetTranslate(float x, float y, float z)
{
	m_TransMatrix = glm::mat4(1.0f);
	this->AddTranslate(x, y, z);
}

void CC3DModelControl::AddTranslate(float x, float y, float z)
{
	m_TransMatrix = glm::translate(m_TransMatrix, glm::vec3(x, y, z));
	UpdateModelMatrix();
}

void CC3DModelControl::SetScale(float x, float y, float z)
{
	m_ScaleMatrix = glm::mat4(1.0f);
	this->AddScale(x, y, z);
}

void CC3DModelControl::AddScale(float x, float y, float z)
{
	m_ScaleMatrix = glm::scale(m_ScaleMatrix, glm::vec3(x, y, z));
	UpdateModelMatrix();
}

void CC3DModelControl::SetRotate(float xAngle, float yAngle, float zAngle)
{
	m_RotateMatrix = glm::mat4(1.0f);
	this->AddRotateX(xAngle);
	this->AddRotateY(yAngle);
	this->AddRotateZ(zAngle);
}

void CC3DModelControl::AddRotateX(float xAngle)
{
	m_RotateMatrix = glm::rotate(m_RotateMatrix, xAngle*CC_PI/180.f, glm::vec3(1.0f, 0.0f, 0.0f));
	UpdateModelMatrix();
}

void CC3DModelControl::AddRotateY(float yAngle)
{
	m_RotateMatrix = glm::rotate(m_RotateMatrix, yAngle*CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
	UpdateModelMatrix();
}

void CC3DModelControl::AddRotateZ(float zAngle)
{
	m_RotateMatrix = glm::rotate(m_RotateMatrix, zAngle*CC_PI / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));
	UpdateModelMatrix();
}

void CC3DModelControl::UpdateModelMatrix()
{
	m_ModelMatrix = m_TransMatrix*m_ScaleMatrix*m_RotateMatrix*m_InitMatrix;
}