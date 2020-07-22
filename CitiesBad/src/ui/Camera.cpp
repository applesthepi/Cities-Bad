#include "Camera.h"

#include <numbers>

Camera::Camera(float fovRadians, float aspect, float nearZ, float farZ, float distance)
	:m_FOV(fovRadians), m_Aspect(aspect), m_Near(nearZ), m_Far(farZ), m_CameraDistance(distance), m_Lerp(0.0f), m_Lerping(false), m_NeedsVPUpdate(true),
	m_View(glm::mat4(1.0f)), m_Projection(glm::mat4(1.0f)),
	m_LerpBegin(0.0f), m_LerpEnd(0.0f),
	m_Position(glm::vec3(0.0f)), m_RealPosition(glm::vec3(0.0f)), m_Rotation(glm::vec3(0.0f, 0.0f, 1.0f)), m_CameraForward(glm::vec3(0.0f, 0.0f, 1.0f)), m_LocationForward(glm::vec3(0.0f, 0.0f, 1.0f)) {}


void Camera::FrameUpdate(Timestep ts)
{
	if (m_Lerping)
	{
		m_Lerp += ts * 10;

		if (m_Lerp >= 1.0f)
		{
			m_Lerp = 1.0f;
			m_Lerping = false;
		}

		m_CameraDistance = glm::mix(m_LerpEnd, m_LerpBegin, 1.0f - m_Lerp);
		m_NeedsVPUpdate = true;
	}
}

void Camera::SetRotation(glm::vec2 rotation)
{
	using std::numbers::pi;

	static float borderTop = pi / 2.0f - 0.05f;
	static float borderBottom = pi / -2.0f + 0.05f;

	if (rotation.x < borderBottom)
		rotation.x = borderBottom;
	else if (rotation.x > borderTop)
		rotation.x = borderTop;

	m_Rotation = glm::vec3(rotation.x, rotation.y, 0.0f);
	m_NeedsVPUpdate = true;
}

void Camera::SetPosition(glm::vec3 position)
{
	m_Position = { position.x, position.y, position.z };
	m_NeedsVPUpdate = true;

	m_Lerping = true;
	m_NeedsVPUpdate = true;
}

void Camera::Translate(glm::vec3 position)
{
	m_Position += glm::vec3(position.x, position.y, position.z);
	m_NeedsVPUpdate = true;
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

void Camera::SetDistance(float distance)
{
	m_LerpBegin = m_CameraDistance;
	m_LerpEnd = distance;
	
	m_Lerp = 0.0f;
	m_Lerping = true;
	m_NeedsVPUpdate = true;
}

glm::vec3 Camera::GetPosition()
{
	return m_Position;
}

glm::vec3 Camera::GetRealPosition()
{
	if (m_NeedsVPUpdate)
	{
		m_NeedsVPUpdate = false;
		ConstructVP();
	}

	return m_RealPosition;
}

glm::vec2 Camera::GetRotation()
{
	return m_Rotation;
}

glm::vec3 Camera::GetCameraForward()
{
	return m_CameraForward;
}

glm::vec3 Camera::GetLocationForward()
{
	return m_LocationForward;
}

glm::mat4 Camera::GetView()
{
	if (m_NeedsVPUpdate)
	{
		m_NeedsVPUpdate = false;
		ConstructVP();
	}

	return m_View;
}

glm::mat4 Camera::GetProjection()
{
	if (m_NeedsVPUpdate)
	{
		m_NeedsVPUpdate = false;
		ConstructVP();
	}

	return m_Projection;
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

	return m_Projection * m_View * model;
}

glm::mat4 Camera::ConstructModel(glm::vec3 objectPosition, glm::vec3 objectRotation, glm::vec3 objectScale)
{
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(1.0f, 1.0f, -1.0f) * objectPosition);
	model = glm::rotate(model, objectRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, objectRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, objectRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

	return model;
}

void Camera::ConstructVP()
{
	using std::numbers::pi;
	
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec2 cameraRotation = m_Rotation;

	m_CameraForward.x = cos(cameraRotation.y) * cos(cameraRotation.x);
	m_CameraForward.z = sin(cameraRotation.y) * cos(cameraRotation.x);
	m_CameraForward.y = sin(cameraRotation.x);

	m_LocationForward.x = cos(cameraRotation.y);
	m_LocationForward.z = sin(cameraRotation.y);

	// pitch
	float theta = m_Rotation.x + (pi / 2.0f);
	// yaw
	float phi = m_Rotation.y + (pi);

	// camera position based on marker (target; imaginary) position
	m_RealPosition = {
		m_CameraDistance * sin(theta) * cos(phi),
		m_CameraDistance * cos(theta),
		m_CameraDistance * sin(theta) * sin(phi)
	};

	m_RealPosition.x += m_Position.x;
	m_RealPosition.z += m_Position.z;

	m_Projection = glm::perspective(m_FOV, m_Aspect, m_Near, m_Far);
	m_View = glm::lookAt(
		m_RealPosition,
		m_RealPosition + m_CameraForward,
		cameraUp
	);
}
