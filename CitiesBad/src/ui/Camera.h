#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

class Camera
{
public:
	Camera(float fovRadians, float aspect, float nearZ, float farZ);

	void SetRotation(glm::vec2 rotation);
	void SetPosition(glm::vec3 position);
	void Translate(glm::vec3 position);
	void TranslateForward(glm::vec3 position);
	void Rotate(glm::vec3 rotation);
	void SetFOV(float radians);
	void SetAspect(float aspect);
	void SetNearFar(float nearZ, float farZ);

	glm::vec3 GetPosition();
	glm::vec3 GetRotation();
	glm::vec3 GetForward();

	glm::mat4 ConstructMVP(glm::vec3 objectPosition, glm::vec3 objectRotation, glm::vec3 objectScale);
private:
	void ConstructVP();

	float m_FOV;
	float m_Aspect;
	float m_Near, m_Far;

	bool m_NeedsVPUpdate;
	glm::mat4 m_VP;

	glm::vec3 m_Position;
	glm::vec3 m_Rotation;
	glm::vec3 m_CameraFront;
};