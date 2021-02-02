#pragma once
#include "ui/Camera.h"

#include <GLCore.h>
#include <GLCoreUtils.h>

using namespace GLCore;
using namespace GLCore::Utils;

class Terrain
{
public:
	Terrain(glm::vec<2, uint32_t> size, float cellSize);
	~Terrain();

	void Render(Camera& camera, Timestep ts);
	void Render(Camera& camera, Timestep ts, Shader* shader);
	void SetLightProjection(glm::mat4 projection);
	glm::vec<2, uint32_t> GetSize();
	float GetCellSize();

	static glm::vec3 LIGHT_POS;
	static glm::mat4 LIGHT;
private:
	void RenderSelf(Camera& camera, Timestep ts);

	void BufferDefault();
	void BufferPosition(glm::vec<2, uint32_t> vertex, glm::vec3 position);
	void BufferColor(glm::vec<2, uint32_t> vertex, glm::vec4 color);

	uint32_t m_VA, m_VB, m_IB, m_TextureDiffuse, m_TextureNormal;
	bool m_Setup;
	float m_CellSize;
	glm::vec<2, uint32_t> m_Size;
	
	Shader* m_Shader;
	float* m_Buffer;
	glm::mat4 m_LightProjection;
};