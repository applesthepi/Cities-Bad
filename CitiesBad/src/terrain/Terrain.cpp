#include "Terrain.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <random>

using namespace noise;

struct TerrainVertex
{
	TerrainVertex()
		:Position({ 0.0f, 0.0f, 0.0f }), Color({ 1.0f, 1.0f, 1.0f, 1.0f }), Normal({ 0.0f, 1.0f, 0.0f }), ShadowCoords({ 0.0f, 0.0f }) {}

	TerrainVertex(glm::vec3 position, glm::vec4 color)
		:Position(position), Color(color), Normal({ 0.0f, 1.0f, 0.0f }), ShadowCoords({ 0.0f, 0.0f }) {}

	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec3 Normal;
	glm::vec2 ShadowCoords;
};

static size_t terrainCellSize = sizeof(TerrainVertex) * 4;
static size_t terrainCellFloatCount = terrainCellSize / sizeof(float);

Terrain::Terrain(glm::vec<2, uint32_t> size, float cellSize)
	:m_VA(0), m_VB(0), m_IB(0), m_Setup(false), m_CellSize(cellSize), m_Size(size)
{
	m_Shader = Shader::FromGLSLTextFiles(
		"res/shaders/terrain.vert.glsl",
		"res/shaders/terrain.frag.glsl"
	);

	m_Setup = m_Shader->GetRendererID() > 0;

	if (!m_Setup)
	{
		LOG_ERROR("failed to setup terrain");
		return;
	}
	
	m_Buffer = (float*)malloc(sizeof(TerrainVertex) * size.x * size.y);
	
	if (m_Buffer == nullptr)
	{
		LOG_ERROR("out of memory");
		m_Setup = false;
		return;
	}

	BufferDefault();

	uint32_t* indices = (uint32_t*)malloc(sizeof(uint32_t) * (size.x - 1) * (size.y - 1) * 6);

	if (indices == nullptr)
	{
		LOG_ERROR("out of memory");
		m_Setup = false;
		return;
	}

	for (uint64_t y = 0; y < (size.y - 1); y++)
	{
		for (uint64_t x = 0; x < (size.x - 1); x++)
		{
			uint64_t itile = ((y * (size.x - 1)) + x);
			uint64_t vtile = ((y * size.x) + x);

			indices[itile * 6 + 0] = vtile;
			indices[itile * 6 + 1] = vtile + 1;
			indices[itile * 6 + 2] = vtile + m_Size.x + 1;
			indices[itile * 6 + 3] = vtile + m_Size.x + 1;
			indices[itile * 6 + 4] = vtile + m_Size.x;
			indices[itile * 6 + 5] = vtile;
		}
	}

	glUseProgram(m_Shader->GetRendererID());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform4f(
		glGetUniformLocation(m_Shader->GetRendererID(), "u_Color"),
		1.0f, 1.0f, 1.0f, 1.0f
	);

	glCreateVertexArrays(1, &m_VA);
	glBindVertexArray(m_VA);

	glCreateBuffers(1, &m_VB);
	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TerrainVertex) * size.x * size.y, m_Buffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, Normal));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, ShadowCoords));

	glCreateBuffers(1, &m_IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6 * (m_Size.x - 1) * (m_Size.y - 1), indices, GL_STATIC_DRAW);

	free(indices);
}

Terrain::~Terrain()
{
	if (!m_Setup)
		return;

	glDeleteVertexArrays(1, &m_VA);
	glDeleteBuffers(1, &m_VB);
	glDeleteBuffers(1, &m_IB);

	free(m_Buffer);
	delete m_Shader;
}

void Terrain::Render(Camera& camera, Timestep ts)
{
	if (!m_Setup)
		return;

	glUseProgram(m_Shader->GetRendererID());

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_Projection"), 1, GL_FALSE, glm::value_ptr(camera.GetProjection()));
	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_View"), 1, GL_FALSE, glm::value_ptr(camera.GetView()));
	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_Model"), 1, GL_FALSE, glm::value_ptr(camera.ConstructModel(
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f }
	)));
	glUniform1i(glGetUniformLocation(m_Shader->GetRendererID(), "u_ShadowTexture"), 0);
	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_Light"), 1, GL_FALSE, glm::value_ptr(LIGHT));
	glUniform3f(glGetUniformLocation(m_Shader->GetRendererID(), "u_LightPos"), LIGHT_POS.x, LIGHT_POS.y, LIGHT_POS.z);
	glUniform3f(glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewPos"), camera.GetRealPosition().x, camera.GetRealPosition().y, camera.GetRealPosition().z);

	//glUniform3f(glGetUniformLocation(m_Shader->GetRendererID(), "u_LightDirection"), 0.5f, 0.3f, 0.5f);
	//glUniform3f(glGetUniformLocation(m_Shader->GetRendererID(), "u_LightColor"), 1.0f, 1.0f, 0.8f);
	//glUniform4f(glGetUniformLocation(m_Shader->GetRendererID(), "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);

	RenderSelf(camera, ts);
}

void Terrain::Render(Camera& camera, Timestep ts, Shader* shader)
{
	if (!m_Setup)
		return;

	glUseProgram(shader->GetRendererID());

	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, Position));

	RenderSelf(camera, ts);
}

void Terrain::RenderSelf(Camera& camera, Timestep ts)
{
	glBindVertexArray(m_VA);
	glDrawElements(GL_TRIANGLES, 6 * (m_Size.x - 1) * (m_Size.y - 1), GL_UNSIGNED_INT, nullptr);
}

void Terrain::BufferDefault()
{
	if (!m_Setup)
		return;

	std::mt19937 gen;
	std::uniform_real_distribution dis = std::uniform_real_distribution(0.5, 0.6);

	module::Perlin perlinGenerator;

	glm::vec3 colorTip = { 0.5f, 0.5f, 0.5f };
	glm::vec3 colorThick = { -0.05f, -0.03f, -0.05f };
	glm::vec3 colorBase = { 0.1f, 0.2f, 0.1f };

	for (uint64_t i = 0; i < m_Size.x * m_Size.y; i++)
	{
		double perlinTerrain = perlinGenerator.GetValue((float)(i % m_Size.x) / 500.0f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 500.0f) / 0.5f;
		double perlinTip = perlinGenerator.GetValue((float)(i % m_Size.x) / 500.0f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 500.0f) / 5.0f;
		double perlinThick = (1.0f - perlinGenerator.GetValue((float)(i % m_Size.x) / 500.0f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 500.0f) / 1.0f) - 1.0f;
		double perlinGrass = (perlinGenerator.GetValue((float)(i % m_Size.x) / 5.123f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 5.123f) / 100.0f + 0.15f) / 2.0f;
		double perlinGrassLow = (perlinGenerator.GetValue((float)(i % m_Size.x) / 100.123f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 100.123f) / 50.0f + 0.0f) / 1.0f;

		perlinTip = glm::clamp(static_cast<float>(perlinTip), 0.0f, 1.0f);
		perlinThick = glm::clamp(static_cast<float>(perlinThick), 0.0f, 1.0f);
		perlinGrass = glm::clamp(static_cast<float>(perlinGrass), 0.0f, 1.0f);
		perlinGrass = glm::clamp(static_cast<float>(perlinGrassLow), 0.0f, 1.0f);

		glm::vec3 offset = glm::vec3(
			m_CellSize * (i % m_Size.x),
			static_cast<float>(perlinTerrain),
			m_CellSize * floor(i / m_Size.x)
		);

		glm::vec3 thisColor = colorBase + (static_cast<float>(perlinTip) * colorTip) + (static_cast<float>(perlinThick) * colorThick) - static_cast<float>(perlinGrass + perlinGrassLow);
		TerrainVertex vertex = TerrainVertex(offset, glm::vec4(colorBase, 1.0f));

		memcpy(m_Buffer + (i * (sizeof(TerrainVertex) / sizeof(float))), &vertex, sizeof(TerrainVertex));
	}

	for (uint64_t y = 0; y < (m_Size.y - 1); y++)
	{
		for (uint64_t x = 0; x < (m_Size.x - 1); x++)
		{
			uint64_t vtile = ((y * m_Size.x) + x);

			glm::vec3 side1 = {
				m_Buffer[(vtile + m_Size.x) * (sizeof(TerrainVertex) / sizeof(float))],
				m_Buffer[(vtile + m_Size.x) * (sizeof(TerrainVertex) / sizeof(float)) + 1],
				m_Buffer[(vtile + m_Size.x) * (sizeof(TerrainVertex) / sizeof(float)) + 2]
			};

			glm::vec3 side2 = {
				m_Buffer[(vtile + m_Size.x + 1) * (sizeof(TerrainVertex) / sizeof(float))],
				m_Buffer[(vtile + m_Size.x + 1) * (sizeof(TerrainVertex) / sizeof(float)) + 1],
				m_Buffer[(vtile + m_Size.x + 1) * (sizeof(TerrainVertex) / sizeof(float)) + 2]
			};

			side1 -= glm::vec3(
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float))],
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float)) + 1],
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float)) + 2]
			);

			side2 -= glm::vec3(
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float))],
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float)) + 1],
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float)) + 2]
			);

			glm::vec3 normal = glm::cross(side1, side2);

			glm::vec2 shadowCoords = {
				static_cast<float>(x) / (static_cast<float>(m_Size.x) - 1.0f),
				static_cast<float>(y) / (static_cast<float>(m_Size.y) - 1.0f)
			};

			memcpy(m_Buffer + (vtile * (sizeof(TerrainVertex) / sizeof(float))) + 7, &normal, sizeof(glm::vec3));
			memcpy(m_Buffer + (vtile * (sizeof(TerrainVertex) / sizeof(float))) + 10, &shadowCoords, sizeof(glm::vec2));
		}
	}
}

void Terrain::BufferPosition(glm::vec<2, uint32_t> vertex, glm::vec3 position)
{
	if (!m_Setup)
		return;
}

void Terrain::BufferColor(glm::vec<2, uint32_t> vertex, glm::vec4 color)
{
	if (!m_Setup)
		return;
}
