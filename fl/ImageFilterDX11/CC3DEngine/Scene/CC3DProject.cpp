
#include "CC3DProject.h"
#include <algorithm>
using namespace std;
//******************************CC3DProject*******************************************
CC3DProject::CC3DProject(void)
{
	m_ProjectMatrix = glm::mat4();
}

CC3DProject::~CC3DProject(void)
{

}

float* CC3DProject::Get()
{
	return &m_ProjectMatrix[0][0];
}

glm::mat4& CC3DProject::GetProjectMatrix()
{
	return m_ProjectMatrix;
}


//******************************CC3DOrthographicProject*******************************************
CC3DOrthographicProject::CC3DOrthographicProject(void)
{
	m_fLeft = 0.0f;
	m_fRight = 10.0f;
	m_fTop = 0.0f;
	m_fBottom =10.0f;
	m_fNear = 0.1f;
	m_fFar = 1000.0f;

	UpdateProjectMatrix();
}

CC3DOrthographicProject::CC3DOrthographicProject(float fLeft, float fRight, float fTop, float fBottom, float fNear, float fFar)
{
	Rest(fLeft, fRight, fTop, fBottom, fNear, fFar);
}

CC3DOrthographicProject::~CC3DOrthographicProject(void)
{

}

void CC3DOrthographicProject::Rest(float fLeft, float fRight, float fTop, float fBottom, float fNear, float fFar)
{
	SetLeft(fLeft);
	SetRight(fRight);
	SetTop(fTop);
	SetBottom(fBottom);
	SetNear(fNear);
	SetFar(fFar);
	UpdateProjectMatrix();
}

void CC3DOrthographicProject::UpdateProjectMatrix()
{
	m_ProjectMatrix = glm::ortho(m_fLeft, m_fRight, m_fTop, m_fBottom, m_fNear, m_fFar);
}

void CC3DOrthographicProject::SetLeft(float fLeft)
{
	m_fLeft = fLeft;
}

void CC3DOrthographicProject::SetRight(float fRight)
{
	m_fRight = fRight;
}

void CC3DOrthographicProject::SetTop(float fTop)
{
	m_fTop = fTop;
}

void CC3DOrthographicProject::SetBottom(float fBottom)
{
	m_fBottom = fBottom;
}

void CC3DOrthographicProject::SetNear(float fNear)
{
	m_fNear = fNear;
}

void CC3DOrthographicProject::SetFar(float fFar)
{
	m_fFar = ((std::max)(m_fNear, fFar));
}
//******************************CC3DPerspectiveProject*******************************************
CC3DPerspectiveProject::CC3DPerspectiveProject(void)
{
	m_fFovy = 45.0f;
	m_fAspect = 4.0f / 3.0f;
	m_fNear = 0.1f;
    m_fFar = 1000.0f;
	UpdateProjectMatrix();
}

CC3DPerspectiveProject::CC3DPerspectiveProject(float fFovy, float fAspect, float fNear, float fFar)
{
	Rest(fFovy, fAspect, fNear, fFar);
}

CC3DPerspectiveProject::~CC3DPerspectiveProject()
{

}

void CC3DPerspectiveProject::Rest(float fFovy, float fAspect, float fNear, float fFar)
{
	SetFovy(fFovy);
	SetAspect(fAspect);
	SetNear(fNear);
	SetFar(fFar);
	UpdateProjectMatrix();
}

void CC3DPerspectiveProject::UpdateProjectMatrix()
{
	m_ProjectMatrix = glm::perspective(glm::radians(m_fFovy), m_fAspect, m_fNear, m_fFar);
}

void CC3DPerspectiveProject::SetFovy(float fFovy)
{
	fFovy = ((std::max)(fFovy, 0.0f));
	fFovy = ((std::min)(fFovy, 180.0f));
	m_fFovy = fFovy;
	UpdateProjectMatrix();
}

void CC3DPerspectiveProject::SetAspect(float fAspect)
{
	m_fAspect = ((std::max)(fAspect, 0.000001f));
	UpdateProjectMatrix();
}

void CC3DPerspectiveProject::SetNear(float fNear)
{
	m_fNear = ((std::max)(fNear, 0.0f));
	UpdateProjectMatrix();
}

void CC3DPerspectiveProject::SetFar(float fFar)
{
	m_fFar = ((std::max)(m_fNear, fFar));
	UpdateProjectMatrix();
}