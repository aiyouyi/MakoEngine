
#include "CC3DCamera.h"

CC3DCamera::CC3DCamera(void)
{
	m_CameraPos = glm::vec3(0,0,4);
	m_LookAtPos = glm::vec3(0,0,0);
	m_UpDir = glm::vec3(0,1,0);

	UpdateViewMatrix();
}

CC3DCamera::CC3DCamera(glm::vec3 cameraPos, glm::vec3 lookAtPos, glm::vec3 upDir)
{
	m_CameraPos = cameraPos;
	m_LookAtPos = lookAtPos;
	m_UpDir = upDir;

	UpdateViewMatrix();
}

CC3DCamera::~CC3DCamera(void)
{

}

float* CC3DCamera::Get()
{
	return &m_ViewMatrix[0][0];
}

void CC3DCamera::SetCameraPos(glm::vec3 pos)
{
	m_CameraPos = pos;

	UpdateViewMatrix();
}

void CC3DCamera::SetLookAtPos(glm::vec3 pos)
{
	m_LookAtPos = pos;

	UpdateViewMatrix();
}

void CC3DCamera::SetUpDir(glm::vec3 dir)
{
	m_UpDir = dir;

	UpdateViewMatrix();
}

glm::mat4& CC3DCamera::GetViewMatrix()
{
	return m_ViewMatrix;
}

glm::vec3& CC3DCamera::GetCameraPos()
{
	return m_CameraPos;
}

glm::vec3& CC3DCamera::GetLookAtPos()
{
	return m_LookAtPos;
}

glm::vec3& CC3DCamera::GetUpDir()
{
	return m_UpDir;
}

void CC3DCamera::SetDistanceToRO(float z)
{
	distance_to_ro = z;
	moveDistanceEachStep = distance_to_ro / 10.0f;
}

glm::mat4 CC3DCamera::GetWorldToOrign()
{
	glm::mat4 translation = glm::mat4(1.0f);
	return glm::translate(translation, -m_CameraPos);
}

void CC3DCamera::SetScale(float x, float y, float z)
{
	//缩放相当于相机进行平移
	glm::mat4 translation = glm::mat4(1.0f);
	translation = glm::translate(translation, glm::vec3(0.0f, 0.0f, z * 1.5f * moveDistanceEachStep));

	//glm::vec3 move_dir = glm::vec3(0.0f) - m_CameraPos;
	//move_dir = move_dir * glm::vec3(0.0f, 0.0f, z);

	//translation = glm::translate(translation, move_dir);

	m_CameraPos = translation * glm::vec4(m_CameraPos, 1.0);
	UpdateViewMatrix();
}

void CC3DCamera::setRoatate(float xAngle, float yAngle, float zAngle)
{
	//左键拖动相当于旋转相机
	glm::mat4 rotation = glm::mat4(1.0f);
	//rotation = glm::rotate(rotation, -xAngle * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f)) *
	//	glm::rotate(rotation, -yAngle * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f)) *
	//	glm::rotate(rotation, zAngle * CC_PI / 180.f, glm::vec3(0.0f, 0.0f, -1.0f));

	glm::tquat<float> qtn{ 1.f, 0.f, 0.f, 0.0f };
	qtn = glm::rotate(qtn, xAngle * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(qtn, yAngle * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::rotate(qtn, zAngle * CC_PI / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));

	//glm::tquat<float> qtn1 = glm::rotate(qtn, xAngle * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::tquat<float> qtn2 = glm::rotate(qtn, yAngle * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));
	//glm::tquat<float> qtn3 = glm::rotate(qtn, zAngle * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 1.0f));

	//qtn = qtn1 * qtn2 * qtn3;

	rotation = glm::mat4_cast(qtn) * rotation;

	m_CameraPos = glm::inverse(rotation) * glm::vec4(m_CameraPos, 1.0f);
	UpdateViewMatrix();
}

void CC3DCamera::UpdateViewMatrix()
{
	m_ViewMatrix = glm::lookAt(m_CameraPos, m_LookAtPos, m_UpDir);
}

void CC3DCamera::UpdateViewMatrix(const Vector3& eye, const Vector3& center, const Vector3& up)
{
	glm::vec3 CameraPos = glm::vec3(eye.x, eye.y, eye.z);
	glm::vec3 LookAtPos = glm::vec3(center.x, center.y, center.z);
	glm::vec3 UpDir = glm::vec3(up.x, up.y, up.z);
	m_ViewMatrix = glm::lookAt(CameraPos, LookAtPos, UpDir);

}
