#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <cassert>

class GameLayer : public GLCore::Layer
{
public:
	GameLayer();
	virtual ~GameLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
private:
	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::OrthographicCameraController m_CameraController;
	
	GLuint m_QuadVA, m_QuadVB, m_QuadIB;
	GLuint m_TexPixel, m_TexTest;

	float* m_GrandBuffer;
	uint64_t m_GradeBufferCount;
};