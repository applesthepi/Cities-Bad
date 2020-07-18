#include "Camera.h"

#include <numbers>

Camera::Camera(float fovRadians, float aspect, float nearZ, float farZ)
	:m_FOV(fovRadians), m_Aspect(aspect), m_Near(nearZ), m_Far(farZ), m_NeedsVPUpdate(true), m_VP(glm::mat4(1.0f)),
	m_Position(glm::vec3(0.0f)), m_Rotation(glm::vec3(0.0f, 0.0f, 1.0f)), m_CameraFront(glm::vec3(0.0f, 0.0f, 1.0f)) {}


void Camera::SetRotation(glm::vec2 rotation)
{
	using std::numbers::pi;

	static float borderTop = pi / 2.0f - 0.05f;
	static float borderBottom = pi / -2.0f + 0.05f;

	if (rotation.x < borderBottom)
		rotation.x = borderBottom;
	else if (rotation.x > borderTop)
		rotation.x = borderTop;

	//if (rotation.y < 0.1f)
	//	rotation.y = 0.1f;
	//else if (rotation.y > pi - 0.1f)
	//	rotation.y = pi - 0.1f;

	rotation.y -= pi / 2.0f;
	m_Rotation = glm::vec3(rotation.x, rotation.y, 0.0f);

	m_CameraFront.x = cos(rotation.y) * cos(rotation.x);
	m_CameraFront.z = sin(rotation.y) * cos(rotation.x);
	m_CameraFront.y = sin(rotation.x);

	m_NeedsVPUpdate = true;




	//using std::numbers::pi;
	//
	//m_Rotation = rotation;
	//m_CameraFront = glm::normalize(rotation);
	//m_NeedsVPUpdate = true;
}

void Camera::SetPosition(glm::vec3 position)
{
	m_Position = position;
	m_NeedsVPUpdate = true;
}

void Camera::Translate(glm::vec3 position)
{
	m_Position += glm::vec3(-position.x, position.y, -position.z);
	m_NeedsVPUpdate = true;
}

void Camera::TranslateForward(glm::vec3 position)
{
	m_Position += position/* * m_Rotation*/;
	m_NeedsVPUpdate = true;
}

void Camera::Rotate(glm::vec3 rotation)
{
	//m_Rotation = glm::normalize(rotation + m_Rotation);
	//printf("%f, %f, %f\n", m_Rotation.x, m_Rotation.y, m_Rotation.z);
	//m_NeedsVPUpdate = true;
}

void Camera::SetFOV(float radians)
{
	m_FOV = radians;
	m_NeedsVPUpdate = true;
}

void Camera::SetAspect(float aspect)
{
	m_Aspect = aspect;
	m_NeedsVPUpdate = true;
}

void Camera::SetNearFar(float nearZ, float farZ)
{
	m_Near = nearZ;
	m_Far = farZ;
	m_NeedsVPUpdate = true;
}

glm::vec3 Camera::GetPosition()
{
	return m_Position;
}

glm::vec3 Camera::GetRotation()
{
	return m_Rotation;
}

glm::vec3 Camera::GetForward()
{
	return m_CameraFront;
}

glm::mat4 Camera::ConstructMVP(glm::vec3 objectPosition, glm::vec3 objectRotation, glm::vec3 objectScale)
{
	if (m_NeedsVPUpdate)
	{
		m_NeedsVPUpdate = false;
		ConstructVP();
	}

	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(1.0f, 1.0f, -1.0f) * objectPosition);
	model = glm::rotate(model, objectRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, objectRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, objectRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

	return m_VP * model;
}

void Camera::ConstructVP()
{
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, 0.1f, 10000.f);
	glm::mat4 view = glm::lookAt(
		glm::vec3(-m_Position.x, m_Position.y, -m_Position.z),
		glm::vec3(-m_Position.x, m_Position.y, -m_Position.z) + m_CameraFront, cameraUp);
	
	//view = glm::rotate(view, m_Rotation.x, glm::vec3(-1.0f, 0.0f, 0.0f));
	//view = glm::rotate(view, m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	//view = glm::rotate(view, m_Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	//view = glm::translate(view, glm::vec3(-1.0f, -1.0f, 1.0f) * m_Position);

	m_VP = projection * view;
}
