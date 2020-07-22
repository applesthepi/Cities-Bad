#pragma once
#include "ui/Camera.h"

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
	Camera m_Camera;
	
	GLuint m_QuadVA, m_QuadVB, m_QuadIB;
	GLuint m_TexPixel, m_TexTest;

	float* m_GrandBuffer;
	uint64_t m_GradeBufferCount;
};