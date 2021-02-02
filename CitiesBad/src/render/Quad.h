#pragma once
#include "ui/Camera.h"

#include <GLCore.h>
#include <GLCoreUtils.h>

using namespace GLCore;
using namespace GLCore::Utils;

class Quad
{
public:
	Quad(glm::vec4 color);
	Quad(const std::string& path);
	~Quad();

	void Render(Camera& camera);
	void Render(Camera& camera, uint32_t texture);
private:
	bool m_HasTexture;
	uint32_t m_VA, m_VB, m_IB;
	uint32_t m_Texture;

	Shader* m_Shader;
};