#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

using namespace GLCore;
using namespace GLCore::Utils;

class Camera
{
public:
	Camera(float fovRadians, float aspect, float nearZ, float farZ, float distance);

	void FrameUpdate(Timestep ts);
	void SetRotation(glm::vec2 rotation);
	void SetPosition(glm::vec3 position);
	void Translate(glm::vec3 position);
	void SetFOV(float radians);
	void SetAspect(float aspect);
	void SetNearFar(float nearZ, float farZ);
	void SetDistance(float distance);

	// virtual position
	glm::vec3 GetPosition();
	// real camera position
	glm::vec3 GetRealPosition();

	glm::vec2 GetRotation();
	// forward normal from the camera
	glm::vec3 GetCameraForward();
	// forward normal for the virtual location (on map)
	glm::vec3 GetLocationForward();
	glm::mat4 GetView();
	glm::mat4 GetProjection();

	glm::mat4 ConstructMVP(glm::vec3 objectPosition, glm::vec3 objectRotation, glm::vec3 objectScale);
	glm::mat4 ConstructModel(glm::vec3 objectPosition, glm::vec3 objectRotation, glm::vec3 objectScale);
private:
	void ConstructVP();

	float m_FOV;
	float m_Aspect;
	float m_Near, m_Far;
	float m_CameraDistance;
	float m_Lerp;

	bool m_Lerping;
	bool m_NeedsVPUpdate;
	glm::mat4 m_View, m_Projection;

	float m_LerpBegin, m_LerpEnd;
	glm::vec3 m_Position, m_RealPosition;
	glm::vec3 m_Rotation;
	glm::vec3 m_CameraForward;
	glm::vec3 m_LocationForward;
};